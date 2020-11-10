
#pragma once

#include "base/base_compression.h"
#include "base/base_exception.h"
#include "base/base_include_glm.h"
#include "base/base_serialize_record_reader.h"
#include "base/base_serialize_record_writer.h"

#include "engine/camera/cam_control_manager.h"
#include "engine/camera/cam_editor_control.h"

#include "engine/core/core_program_parameters.h"

#include "engine/core/core_plugin_manager.h"

#include "engine/engine.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_selection.h"
#include "engine/graphic/gfx_selection_renderer.h"
#include "engine/graphic/gfx_view_manager.h"

#include "engine/core/core_asset_manager.h"

#include "plugin/slam/mr_plane_colorizer.h"

#include "engine/script/script_script.h"

#include "engine/network/core_network_manager.h"

#include "plugin/slam/gfx_reconstruction_renderer.h"

#include <filesystem>

namespace MR
{
    class CSLAMControlARKit
    {
    private:

        enum EMessageType
        {
            COMMAND,
            TRANSFORM,
            STRUCTUREDEPTHFRAME,
            COLORFRAME,
            LIGHTESTIMATE,
            PLANE,
            COLORINTRINSICS,
            ARKITDEPTHFRAME
        };

        struct SIntrinsics
        {
            glm::vec2 m_FocalLength;
            glm::vec2 m_FocalPoint;
        };
        
        Gfx::CTexturePtr m_DepthTexture;
        Gfx::CTexturePtr m_DepthTextureResized;
        Gfx::CTexturePtr m_RGBATexture;
        glm::mat4 m_PoseMatrix;

        glm::ivec2 m_DepthSize;
        glm::ivec2 m_ColorSize;

        SIntrinsics m_ColorIntrinsics;
        SIntrinsics m_DepthIntrinsics;

        glm::ivec2 m_DeviceResolution;
        glm::mat4 m_DeviceProjectionMatrix;

        bool m_UseTrackingCamera = true;

        bool m_IsReconstructorInitialized = false;

        const int m_TileSize2D = 16;
        
        // -----------------------------------------------------------------------------
        // Reconstructor
        // -----------------------------------------------------------------------------

        bool m_CaptureColor;

        MR::CSLAMReconstructor m_Reconstructor;

        // -----------------------------------------------------------------------------
        // Stuff for network data source
        // -----------------------------------------------------------------------------
        Net::CNetworkManager::CMessageDelegate::HandleType m_SLAMNetHandle;

        Gfx::CShaderPtr m_YUVtoRGBCSPtr;
        Gfx::CTexturePtr m_YTexture;
        Gfx::CTexturePtr m_UVTexture;

        Gfx::CShaderPtr m_ConvertARKitDepthCSPtr;
        
        struct SIntrinsicsMessage
        {
            glm::vec2  m_FocalLength;
            glm::vec2  m_FocalPoint;
            glm::ivec2 m_DepthSize;
            glm::ivec2 m_ColorSize;
            glm::ivec2 m_DeviceResolution;
            glm::mat4  m_DeviceProjectionMatrix;
            glm::mat4  m_RelativeCameraTransform;
        };

        Net::SocketHandle m_SLAMSocket;

        // -----------------------------------------------------------------------------
        // Recording
        // -----------------------------------------------------------------------------
        enum EPlayMode
        {
            NONE,
            PLAY,
            LOAD_SCENE,
        };
        
        EPlayMode m_PlayMode = NONE;

        std::fstream m_RecordFile;
        std::unique_ptr<Base::CRecordWriter> m_pRecordWriter;
		std::unique_ptr<Base::CRecordReader> m_pRecordReader;

        std::fstream m_TempRecordFile;
        std::unique_ptr<Base::CRecordWriter> m_pTempRecordWriter;
        std::string m_TempRecordPath;

		int m_NumberOfExtractedFrames;

		bool m_ExtractStream;

    public:

        void Start()
        {
            Gfx::ReconstructionRenderer::SetReconstructor(m_Reconstructor);

			// -----------------------------------------------------------------------------
			// Settings
			// -----------------------------------------------------------------------------
			m_ExtractStream = Core::CProgramParameters::GetInstance().Get("mr:file_storing:extract_stream", false);
			m_NumberOfExtractedFrames = 0;

            // -----------------------------------------------------------------------------
                // Create network connection for SLAM client
                // -----------------------------------------------------------------------------
            auto SLAMDelegate = std::bind(&CSLAMControlARKit::OnNewSLAMMessage, this, std::placeholders::_1, std::placeholders::_2);

            int Port = Core::CProgramParameters::GetInstance().Get("mr:slam:network_port", 12345);
            m_SLAMSocket = Net::CNetworkManager::GetInstance().CreateServerSocket(Port);
            m_SLAMNetHandle = Net::CNetworkManager::GetInstance().RegisterMessageHandler(m_SLAMSocket, SLAMDelegate);

            auto ModeParameter = Core::CProgramParameters::GetInstance().Get("mr:diminished_reality:mode", "pixmix");

            m_TempRecordPath = Core::AssetManager::GetPathToAssets() + "/recordings/" + "_temp_recording.swr";
            m_TempRecordFile.open(m_TempRecordPath , std::fstream::out | std::fstream::binary | std::fstream::trunc);
        }

        // -----------------------------------------------------------------------------

        void Exit()
        {
            m_RecordFile.close();
            m_TempRecordFile.close();

            m_Reconstructor.Exit();

            m_DepthTexture = nullptr;
            m_RGBATexture = nullptr;

            m_SLAMNetHandle = nullptr;
            
            m_YUVtoRGBCSPtr = nullptr;
            m_YTexture = nullptr;
            m_UVTexture = nullptr;
        }

        // -----------------------------------------------------------------------------

        void Update()
        {
            // -----------------------------------------------------------------------------
            // Playing
            // -----------------------------------------------------------------------------
            if ((m_PlayMode == PLAY || m_PlayMode == LOAD_SCENE) && m_pRecordReader != nullptr)
            {
                m_pRecordReader->Update();

                if (m_pRecordReader->IsEnd())
                {
                    m_PlayMode = NONE;
                    m_UseTrackingCamera = false;
                }

                while (!m_pRecordReader->IsEnd() && m_pRecordReader->PeekTimecode() < m_pRecordReader->GetTime())
                {
                    Net::CMessage Message;

                    *m_pRecordReader >> Message.m_Category;
                    *m_pRecordReader >> Message.m_MessageType;
                    *m_pRecordReader >> Message.m_CompressedSize;
                    *m_pRecordReader >> Message.m_DecompressedSize;
                    Base::Read(*m_pRecordReader, Message.m_Payload);

                    // TODO: find better solution
                    // We just create a temporary recording everytime so we can always save a slam scene.
                    // However, we could also save the loaded recording when saving a scene again.

                    if (m_pTempRecordWriter == nullptr)
                    {
                        m_pTempRecordWriter = std::make_unique<Base::CRecordWriter>(m_TempRecordFile, 1);
                    }

                    WriteMessage(*m_pTempRecordWriter, Message);

                    HandleMessage(Message);
                }
            }

            // -----------------------------------------------------------------------------
            // Devices
            // -----------------------------------------------------------------------------
            
            if (m_UseTrackingCamera)
            {
                auto& rControl = static_cast<Cam::CEditorControl&>(Cam::ControlManager::GetActiveControl());
                
                // -----------------------------------------------------------------------------
                // View
                // -----------------------------------------------------------------------------
                glm::mat4 PoseMatrix = glm::eulerAngleX(glm::radians(90.0f)) * m_PoseMatrix;

                glm::mat4 View = PoseToView(PoseMatrix);

                auto Test = -glm::vec3(PoseMatrix[3].x, PoseMatrix[3].y, PoseMatrix[3].z);

                rControl.SetPosition(glm::vec4(PoseMatrix[3].x, PoseMatrix[3].y, PoseMatrix[3].z, 1.0f));
                rControl.SetRotation(glm::mat4(glm::inverse(glm::mat3(View))));
                rControl.Update();
            }
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent)
        {
            BASE_UNUSED(_rEvent);

            if (!m_IsReconstructorInitialized)
            {
                return;
            }
        }

        // -----------------------------------------------------------------------------

        void SetRecordFile(const std::string& _rFileName, float _Speed = 1.0f)
        {
            m_TempRecordFile = std::fstream(m_TempRecordPath, std::fstream::out | std::fstream::binary);
            m_pTempRecordWriter = std::make_unique<Base::CRecordWriter>(m_TempRecordFile, 1);

            m_RecordFile = std::fstream(_rFileName, std::fstream::in | std::fstream::binary);

            if (!m_RecordFile.is_open())
            {
                BASE_THROWM(("File " + _rFileName + " was not found").c_str());
            }

            m_pRecordReader = std::make_unique<Base::CRecordReader>(m_RecordFile, 1);
            m_pRecordReader->SkipTime();
            m_pRecordReader->SetSpeed(_Speed);
            m_Reconstructor.ResetReconstruction();
        }

        // -----------------------------------------------------------------------------

        void SetPlaybackSpeed(float _Speed)
        {
            if (m_pRecordReader != nullptr)
            {
                m_pRecordReader->SetSpeed(_Speed);
            }
        }

        // -----------------------------------------------------------------------------

        void SetActivateSelection(bool _Flag)
        {
            BASE_UNUSED(_Flag);
        }

        // -----------------------------------------------------------------------------

        void EnableMouseControl(bool _Flag)
        {
            m_UseTrackingCamera = !_Flag;
        }

        // -----------------------------------------------------------------------------

        void ColorizePlanes()
        {
            
        }

        // -----------------------------------------------------------------------------

        void SetIsPlaying(bool _Flag)
        {
            if (m_PlayMode != LOAD_SCENE)
            {
                m_PlayMode = _Flag ? PLAY : NONE;
            }
        }

        // -----------------------------------------------------------------------------
        
        void ResetReconstruction()
        {
            m_Reconstructor.ResetReconstruction();
        }

		// -----------------------------------------------------------------------------
        
		void SendPlanes()
		{
			//m_pPlaneColorizer->ColorizeAllPlanes();

		}

        // -----------------------------------------------------------------------------

        void ReadScene(CSceneReader& _rCodec)
        {
            std::string RecordFileName;

            Base::Serialize(_rCodec, RecordFileName);

            try
            {
                SetRecordFile(RecordFileName, Core::CProgramParameters::GetInstance().Get("mr:file_storing:scene_load_speed", 10000.0f));
            }
            catch (...)
            {
                BASE_THROWM(("The slam data could be loaded because " + RecordFileName + " was not found").c_str());
            }

            m_PlayMode = LOAD_SCENE;
        }

        // -----------------------------------------------------------------------------

        void WriteScene(CSceneWriter& _rCodec)
        {
            std::string RecordFolder = Core::AssetManager::GetPathToAssets() + "/recordings/scene";

            std::string RecordFileName;
            int FileCount = 0;
            do
            {
                RecordFileName = RecordFolder + '/' + std::to_string(FileCount ++) + ".swr";
            } while (std::filesystem::exists(RecordFileName));
            
            try
            {
                if (!std::filesystem::exists(RecordFolder))
                {
                    if (!std::filesystem::create_directory(RecordFolder))
                    {
                        BASE_THROWM("Scene folder could not be created");
                    }
                }
                else if (!std::filesystem::is_directory(RecordFolder))
                {
                    BASE_THROWM(("Cannot create directory " + RecordFolder + ". Is there already a file with that name?").c_str());
                }

                if (!std::filesystem::copy_file(m_TempRecordPath, RecordFileName))
                {
                    BASE_THROWM("SLAM record file could not be saved as part of the scene");
                }
            }
            catch (std::exception& e)
            {
                BASE_THROWM(e.what());
            }

            Base::Serialize(_rCodec, RecordFileName);
        }

    private:

        glm::mat4 PoseToView(const glm::mat4& _rPoseMatrix)
        {
            glm::vec3 Eye = _rPoseMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            glm::vec3 At = _rPoseMatrix * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
            glm::vec3 Up = _rPoseMatrix * glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);

            return glm::lookAtRH(Eye, At, Up);
        }

        // -----------------------------------------------------------------------------

        void HandleMessage(const Net::CMessage& _rMessage)
        {
            std::vector<char> Decompressed(_rMessage.m_DecompressedSize);

            if (_rMessage.m_CompressedSize != _rMessage.m_DecompressedSize)
            {
                try
                {
					Base::Decompress(_rMessage.m_Payload, Decompressed);
                }
                catch (...)
                {
					ENGINE_CONSOLE_ERRORV("Failed to decompress! Ignoring network message!");
					return;
                }
            }
            else
            {
                std::memcpy(Decompressed.data(), _rMessage.m_Payload.data(), Decompressed.size());
            }

            int32_t MessageType = *reinterpret_cast<int32_t*>(Decompressed.data());

            if (MessageType == COMMAND)
            {
                const int MessageID = *reinterpret_cast<int32_t*>(Decompressed.data() + sizeof(int32_t));

                if (MessageID == 0 && m_IsReconstructorInitialized)
                {
                    m_Reconstructor.ResetReconstruction();
                }
                else if (MessageID == 1)
                {
                    InitializeSLAM(*reinterpret_cast<const SIntrinsicsMessage*>(Decompressed.data() + sizeof(int32_t) * 2));
                }
            }
            else if (MessageType == TRANSFORM)
            {
                m_PoseMatrix = *reinterpret_cast<glm::mat4*>(Decompressed.data() + sizeof(int32_t)) * glm::eulerAngleX(glm::pi<float>());
            }
            else if (MessageType == COLORFRAME && m_CaptureColor)
            {
                ExtractRGBAFrame(Decompressed);

                m_Reconstructor.OnNewFrame(m_DepthTextureResized, m_RGBATexture, &m_PoseMatrix, m_ColorIntrinsics.m_FocalLength, m_ColorIntrinsics.m_FocalPoint);

                if (m_ExtractStream)
                {
                    auto FrameString = std::to_string(m_NumberOfExtractedFrames++);

                    auto PathToDepthTexture = Core::AssetManager::GetPathToAssets() + "/" + FrameString + "_depth.raw";
                    auto PathToColorTexture = Core::AssetManager::GetPathToAssets() + "/" + FrameString + "_color.png";

                    Gfx::TextureManager::SaveTexture(m_DepthTexture, PathToDepthTexture);
                    Gfx::TextureManager::SaveTexture(m_RGBATexture, PathToColorTexture);

                    std::ofstream PoseMatrixStream(Core::AssetManager::GetPathToAssets() + "/" + FrameString + "_pose_intrinsics.byte", std::ofstream::binary);

                    PoseMatrixStream.write(reinterpret_cast<char*>(&m_PoseMatrix), sizeof(m_PoseMatrix));
                    PoseMatrixStream.write(reinterpret_cast<char*>(&m_ColorIntrinsics.m_FocalLength), sizeof(m_ColorIntrinsics.m_FocalLength));
                    PoseMatrixStream.write(reinterpret_cast<char*>(&m_ColorIntrinsics.m_FocalPoint), sizeof(m_ColorIntrinsics.m_FocalPoint));

                    PoseMatrixStream.close();
                }
            }
            else if (MessageType == ARKITDEPTHFRAME)
            {
                int32_t Width = *reinterpret_cast<int32_t*>(Decompressed.data() + sizeof(int32_t));
                int32_t Height = *reinterpret_cast<int32_t*>(Decompressed.data() + 2 * sizeof(int32_t));

                const float* RawBuffer = reinterpret_cast<float*>(Decompressed.data() + 7 * sizeof(int32_t));

                Base::AABB2UInt TargetRect;
                TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(Width, Height));
                Gfx::TextureManager::CopyToTexture2D(m_DepthTexture, TargetRect, m_DepthSize.x, const_cast<float*>(RawBuffer));

                Gfx::ContextManager::SetShaderCS(m_ConvertARKitDepthCSPtr);
                Gfx::ContextManager::SetImageTexture(0, m_DepthTexture);
                Gfx::ContextManager::SetImageTexture(1, m_DepthTextureResized);

                Gfx::ContextManager::Dispatch(DivUp(m_ColorSize.x, m_TileSize2D), DivUp(m_ColorSize.y, m_TileSize2D), 1);

                if (!m_CaptureColor)
                {
                    m_Reconstructor.OnNewFrame(m_DepthTextureResized, nullptr, &m_PoseMatrix, m_DepthIntrinsics.m_FocalLength, m_DepthIntrinsics.m_FocalPoint);
                }
            }
        }

        // -----------------------------------------------------------------------------

        void ExtractRGBAFrame(const std::vector<char>& _rData)
        {
            const int32_t Width = *reinterpret_cast<const int32_t*>(_rData.data() + sizeof(int32_t));
            const int32_t Height = *reinterpret_cast<const int32_t*>(_rData.data() + 2 * sizeof(int32_t));

            m_ColorIntrinsics.m_FocalLength = *reinterpret_cast<const glm::vec2*>(_rData.data() + 3 * sizeof(int32_t));
            m_ColorIntrinsics.m_FocalPoint = *reinterpret_cast<const glm::vec2*>(_rData.data() + 3 * sizeof(int32_t) + sizeof(glm::vec2));

            m_DeviceProjectionMatrix = *reinterpret_cast<const glm::mat4*>(_rData.data() + 7 * sizeof(int32_t));

            const char* YData = _rData.data() + 7 * sizeof(int32_t) + sizeof(glm::mat4);
            const char* UVData = YData + Width * Height;

            Base::AABB2UInt TargetRect;
            TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_ColorSize.x, m_ColorSize.y));
            Gfx::TextureManager::CopyToTexture2D(m_YTexture, TargetRect, m_ColorSize.x, const_cast<char*>(YData));

            TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_ColorSize.x / 2, m_ColorSize.y / 2));
            Gfx::TextureManager::CopyToTexture2D(m_UVTexture, TargetRect, m_ColorSize.x / 2, const_cast<char*>(UVData));
            
            Gfx::ContextManager::SetShaderCS(m_YUVtoRGBCSPtr);
            Gfx::ContextManager::SetImageTexture(0, m_YTexture);
            Gfx::ContextManager::SetImageTexture(1, m_UVTexture);
            Gfx::ContextManager::SetImageTexture(2, m_RGBATexture);

            Gfx::ContextManager::Dispatch(DivUp(m_ColorSize.x, m_TileSize2D), DivUp(m_ColorSize.y, m_TileSize2D), 1);

            if (m_UseTrackingCamera)
            {
                auto& rControl = static_cast<Cam::CEditorControl&>(Cam::ControlManager::GetActiveControl());

                rControl.SetProjectionMatrix(glm::transpose(m_DeviceProjectionMatrix));
            }
        }

        // -----------------------------------------------------------------------------

        void InitializeSLAM(const SIntrinsicsMessage& _rMessage)
        {
            ENGINE_CONSOLE_INFO("Initializing reconstructor");
            
            glm::vec2 FocalLength = _rMessage.m_FocalLength;
            glm::vec2 FocalPoint = _rMessage.m_FocalPoint;
            m_DepthSize = _rMessage.m_DepthSize;
            m_ColorSize = _rMessage.m_ColorSize;
            m_DeviceResolution = _rMessage.m_DeviceResolution;
            m_DeviceProjectionMatrix = _rMessage.m_DeviceProjectionMatrix;

            Gfx::ReconstructionRenderer::SetDeviceResolution(m_DeviceResolution);

            MR::SReconstructionSettings Settings;
            m_Reconstructor.GetReconstructionSettings(&Settings);

            m_CaptureColor = Settings.m_CaptureColor;

            m_Reconstructor.SetImageSizes(glm::vec2(m_ColorSize), glm::vec2(m_ColorSize));
            m_Reconstructor.SetIntrinsics(glm::vec2(FocalLength), glm::vec2(FocalPoint));

            m_Reconstructor.Start();

            m_IsReconstructorInitialized = true;

            Gfx::STextureDescriptor TextureDescriptor = {};

            TextureDescriptor.m_NumberOfPixelsU = m_DepthSize.x;
            TextureDescriptor.m_NumberOfPixelsV = m_DepthSize.y;
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_NumberOfMipMaps = 1;
            TextureDescriptor.m_NumberOfTextures = 1;
            TextureDescriptor.m_Binding = Gfx::CTexture::ShaderResource;
            TextureDescriptor.m_Access = Gfx::CTexture::CPUWrite;
            TextureDescriptor.m_Usage = Gfx::CTexture::GPUReadWrite;
            TextureDescriptor.m_Semantic = Gfx::CTexture::UndefinedSemantic;
            TextureDescriptor.m_pFileName = nullptr;
            TextureDescriptor.m_pPixels = nullptr;
            TextureDescriptor.m_Format = Gfx::CTexture::R32_FLOAT;

            m_DepthTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

            TextureDescriptor.m_NumberOfPixelsU = m_ColorSize.x;
            TextureDescriptor.m_NumberOfPixelsV = m_ColorSize.y;
            TextureDescriptor.m_Format = Gfx::CTexture::R16_UINT;

            m_DepthTextureResized = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

            std::stringstream DefineStream;
            DefineStream
                << "#define TILE_SIZE_2D " << m_TileSize2D << " \n"
                << "#define DEPTH_WIDTH " << m_DepthSize.x << " \n"
                << "#define DEPTH_HEIGHT " << m_DepthSize.y << " \n";

            if (m_CaptureColor)
            {
                DefineStream
                    << "#define COLOR_WIDTH " << m_ColorSize.x << " \n"
                    << "#define COLOR_HEIGHT " << m_ColorSize.y << " \n"
                    << "#define CAPTURE_COLOR " << " \n";

                TextureDescriptor.m_NumberOfPixelsU = m_ColorSize.x;
                TextureDescriptor.m_NumberOfPixelsV = m_ColorSize.y;
                TextureDescriptor.m_Format = Gfx::CTexture::R8G8B8A8_UBYTE;
                m_RGBATexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

                TextureDescriptor.m_Format = Gfx::CTexture::R8_UBYTE;
                m_YTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

                TextureDescriptor.m_NumberOfPixelsU = m_ColorSize.x / 2;
                TextureDescriptor.m_NumberOfPixelsV = m_ColorSize.y / 2;
                TextureDescriptor.m_Format = Gfx::CTexture::R8G8_UBYTE;
                m_UVTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

                std::string DefineString = DefineStream.str();
                m_YUVtoRGBCSPtr = Gfx::ShaderManager::CompileCS("../../plugins/slam/cs_yuv_to_rgb.glsl", "main", DefineString.c_str());
            }
            std::string DefineString = DefineStream.str();

            m_ConvertARKitDepthCSPtr = Gfx::ShaderManager::CompileCS("../../plugins/slam/cs_convert_arkit_depth.glsl", "main", DefineString.c_str());;
            
            ENGINE_CONSOLE_INFO("Initialization complete");
        }

        // -----------------------------------------------------------------------------

        void OnNewSLAMMessage(const Net::CMessage& _rMessage, Net::SocketHandle _SocketHandle)
        {
            BASE_UNUSED(_SocketHandle);
            
            if (_rMessage.m_MessageType == 0)
            {
                if (m_pTempRecordWriter == nullptr)
                {
                    m_pTempRecordWriter = std::make_unique<Base::CRecordWriter>(m_TempRecordFile, 1);
                }

                WriteMessage(*m_pTempRecordWriter, _rMessage);

                HandleMessage(_rMessage);
            }
            else if (_rMessage.m_MessageType == 2)
            {
                // Enable mouse control after disconnect
                m_UseTrackingCamera = false;
            }
        }

        // -----------------------------------------------------------------------------

        void WriteMessage(Base::CRecordWriter& _rWriter, const Net::CMessage& _rMessage)
        {
            _rWriter << _rMessage.m_Category;
            _rWriter << _rMessage.m_MessageType;
            _rWriter << _rMessage.m_CompressedSize;
            _rWriter << _rMessage.m_DecompressedSize;
            Base::Write(_rWriter, _rMessage.m_Payload);
        }

        // -----------------------------------------------------------------------------

        int DivUp(int TotalShaderCount, int WorkGroupSize)
        {
            return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
        }
    };
} // namespace Scpt