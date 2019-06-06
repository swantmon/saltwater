
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

namespace MR
{
    class CSLAMControl
    {
    private:

        enum EMessageType
        {
            COMMAND,
            TRANSFORM,
            DEPTHFRAME,
            COLORFRAME,
            LIGHTESTIMATE,
            PLANE
        };

        enum EDATASOURCE
        {
            NETWORK,
            KINECT
        };

        enum EPlaneAction
        {
            ADDPLANE,
            UPDATEPLANE,
            REMOVEPLANE
        };

        struct SIntrinsics
        {
            glm::vec2 m_FocalLength;
            glm::vec2 m_FocalPoint;
        };

        EDATASOURCE m_DataSource;
        
        Gfx::CTexturePtr m_DepthTexture;
        Gfx::CTexturePtr m_RGBATexture;
        std::vector<uint16_t> m_DepthBuffer;
        std::vector<char> m_ColorBuffer;
        glm::mat4 m_PoseMatrix;

        glm::ivec2 m_DepthSize;
        glm::ivec2 m_ColorSize;

        SIntrinsics m_ColorIntrinsics;
        SIntrinsics m_DepthIntrinsics;

        glm::ivec2 m_DeviceResolution;
        glm::mat4 m_DeviceProjectionMatrix;
        glm::mat4 m_RelativeCameraTransform;

        struct SRegisteringBuffer
        {
            SIntrinsics m_ColorIntrinsics;
            SIntrinsics m_DepthIntrinsics;
            glm::mat4 m_RelativeCameraTransform;
        };

        Gfx::CBufferPtr m_RegisteringBufferPtr;

        bool m_UseTrackingCamera = true;

        bool m_IsReconstructorInitialized = false;

        const int m_TileSize2D = 16;
        
        // -----------------------------------------------------------------------------
        // Stuff for selection box
        // -----------------------------------------------------------------------------
        enum class ESelection
        {
            NOSELECTION,
            FIRSTPRESS,
            FIRSTRELEASE,
        };

        glm::vec2 m_LatestCursorPosition;
        glm::vec3 m_SelectionBoxAnchor0;
        glm::vec3 m_SelectionBoxAnchor1;
        float m_SelectionBoxHeight;
        bool m_SelectionFlag;

        ESelection m_SelectionState;

        bool m_LeftAnchorSelected;

        Gfx::CSelectionTicket* m_pSelectionTicket;

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

        Gfx::CShaderPtr m_ShiftDepthCSPtr;
        Gfx::CShaderPtr m_RegisterDepthCSPtr;
        
        Gfx::CTexturePtr m_ShiftTexture;
        Gfx::CTexturePtr m_UnregisteredDepthTexture;
        Gfx::CTexturePtr m_ShiftLUTPtr;

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
        // Stuff for Kinect data source
        // -----------------------------------------------------------------------------
        using GetDepthBufferFunc = bool(*)(uint16_t*);
        using GetColorBufferFunc = bool(*)(char*);
        GetDepthBufferFunc GetDepthBuffer;
        GetColorBufferFunc GetColorBuffer;

        // -----------------------------------------------------------------------------
        // Recording
        // -----------------------------------------------------------------------------
        enum ERecordMode
        {
            NONE,
            PLAY,
            RECORD,
        };
        
        ERecordMode m_RecordMode;

        std::fstream m_RecordFile;
        std::unique_ptr<Base::CRecordWriter> m_pRecordWriter;
        std::unique_ptr<Base::CRecordReader> m_pRecordReader;

        // -----------------------------------------------------------------------------
        // Stuff for inpainting
        // -----------------------------------------------------------------------------
        Gfx::CTexturePtr m_PlaneTexture;
        glm::vec3 m_PlaneAnchor0;
        glm::vec3 m_PlaneAnchor1;

        Net::SocketHandle m_NeuralNetworkSocket;
        Net::CNetworkManager::CMessageDelegate::HandleType m_NeualNetworkDelegate;

        enum EInpaintintingMode
        {
            INPAINTING_DISABLED,
            INPAINTING_NN,
            INPAINTING_PIXMIX,
            INPAINTING_PIXMIX_OCEAN,
        };

        EInpaintintingMode m_InpaintingMode;

        int m_PlaneResolution;
        float m_PlaneScale;

        bool m_SendInpaintedResult;

        GLuint m_PixelDataBuffer; // OpenGL pixel buffer object has 3 times the size of the image (triple buffering)
        int m_PixelBufferSize;

        void* m_pGPUPixelData; // Pointer to raw gpu memory of persistent mapped buffer

        enum EStreamState
        {
            STREAM_SLAM,
            STREAM_DIMINSIHED
        };

        EStreamState m_StreamState;

        glm::mat4 m_PreliminaryPoseMatrix;

        using InpaintWithPixMixFunc = void(*)(const glm::ivec2&, const std::vector<glm::u8vec4>&, std::vector<glm::u8vec4>&);
        InpaintWithPixMixFunc InpaintWithPixMix;


        // -----------------------------------------------------------------------------
        // Plane extraction
        // -----------------------------------------------------------------------------
        std::unique_ptr<MR::CPlaneColorizer> m_pPlaneColorizer;

    public:

        void Start()
        {
            m_StreamState = STREAM_SLAM;

            m_SelectionBoxAnchor0 = glm::vec3(0.0f);
            m_SelectionBoxAnchor1 = glm::vec3(0.0f);
            m_SelectionBoxHeight = 0.0f;
            m_SelectionState = ESelection::NOSELECTION;
            m_LeftAnchorSelected = false;
            m_SelectionFlag = false;

            m_pSelectionTicket = &Gfx::SelectionRenderer::AcquireTicket(-1, -1, 1, 1, Gfx::SPickFlag::Voxel);

            Gfx::ReconstructionRenderer::SetReconstructor(m_Reconstructor);

            // -----------------------------------------------------------------------------
            // Determine where we get our data from
            // -----------------------------------------------------------------------------
            std::string DataSource = Core::CProgramParameters::GetInstance().Get("mr:slam:data_source", "network");

            if (DataSource == "network")
            {
                // -----------------------------------------------------------------------------
                // Create network connection for SLAM client
                // -----------------------------------------------------------------------------
                auto SLAMDelegate = std::bind(&CSLAMControl::OnNewSLAMMessage, this, std::placeholders::_1, std::placeholders::_2);

                int Port = Core::CProgramParameters::GetInstance().Get("mr:slam:network_port", 12345);
                m_SLAMSocket = Net::CNetworkManager::GetInstance().CreateServerSocket(Port);
                m_SLAMNetHandle = Net::CNetworkManager::GetInstance().RegisterMessageHandler(m_SLAMSocket, SLAMDelegate);
                
                m_PlaneResolution = Core::CProgramParameters::GetInstance().Get("mr:diminished_reality:inpainted_plane:resolution", 128);
                m_PlaneScale = Core::CProgramParameters::GetInstance().Get("mr:diminished_reality:inpainted_plane:scale", 2.0f);

                auto ModeParameter = Core::CProgramParameters::GetInstance().Get("mr:diminished_reality:mode", "pixmix");
                
                if (ModeParameter == "nn")
                {
                    ENGINE_CONSOLE_INFO("Inpainting with neural networks");

                    m_InpaintingMode = INPAINTING_NN;

                    // -----------------------------------------------------------------------------
                    // Create network connection for Neural Network Server
                    // -----------------------------------------------------------------------------
                    Port = Core::CProgramParameters::GetInstance().Get("mr:diminished_reality:net:port", 12346);
                    std::string IP = Core::CProgramParameters::GetInstance().Get("mr:diminished_reality:net:ip", "127.0.0.1");
                    m_NeuralNetworkSocket = Net::CNetworkManager::GetInstance().CreateClientSocket(IP, Port);

                    auto NNDelegate = std::bind(&CSLAMControl::OnNewNeuralNetMessage, this, std::placeholders::_1, std::placeholders::_2);
                    m_NeualNetworkDelegate = Net::CNetworkManager::GetInstance().RegisterMessageHandler(m_NeuralNetworkSocket, NNDelegate);
                }
                else if (ModeParameter == "pixmix")
                {
                    ENGINE_CONSOLE_INFO("Inpainting with PixMix (Open version)");

                    if (!Core::PluginManager::LoadPlugin("PixMix"))
                    {
                        BASE_THROWM("PixMix plugin was not loaded");
                    }

                    InpaintWithPixMix = (InpaintWithPixMixFunc)(Core::PluginManager::GetPluginFunction("PixMix", "Inpaint"));

                    m_InpaintingMode = INPAINTING_PIXMIX;
                }
                else if (ModeParameter == "pixmix_ocean")
                {
                    ENGINE_CONSOLE_INFO("Inpainting with PixMix (Original version)");

                    if (!Core::PluginManager::LoadPlugin("PixMixOcean"))
                    {
                        BASE_THROWM("PixMix plugin was not loaded");
                    }

                    InpaintWithPixMix = (InpaintWithPixMixFunc)(Core::PluginManager::GetPluginFunction("PixMix_Ocean", "Inpaint"));

                    m_InpaintingMode = INPAINTING_PIXMIX_OCEAN;
                }
                else
                {
                    m_InpaintingMode = INPAINTING_DISABLED;

                    ENGINE_CONSOLE_INFO("Inpainting is disabled");
                }
                
                m_DataSource = NETWORK;

                CreateShiftLUTTexture();
                CreateRegisteringBuffer();
            }
            else if (DataSource == "kinect")
            {
                // -----------------------------------------------------------------------------
                // Load Kinect plugin
                // -----------------------------------------------------------------------------
                
                if (!Core::PluginManager::LoadPlugin("Kinect"))
                {
                    BASE_THROWM("Kinect plugin was not loaded")
                }

                m_DataSource = KINECT;

                using GetIntrinsicsFunc = void(*)(glm::vec2&, glm::vec2&, glm::ivec2&);

                GetIntrinsicsFunc GetIntrinsics = (GetIntrinsicsFunc)(Core::PluginManager::GetPluginFunction("Kinect", "GetIntrinsics"));

                glm::vec2 FocalLength;
                glm::vec2 FocalPoint;
                
                GetIntrinsics(FocalLength, FocalPoint, m_DepthSize);
                m_ColorSize = m_DepthSize;

                m_Reconstructor.SetImageSizes(glm::vec2(m_DepthSize), glm::vec2(m_ColorSize));
                m_Reconstructor.SetIntrinsics(glm::vec2(FocalLength), glm::vec2(FocalPoint));
                
                m_Reconstructor.Start();

                m_IsReconstructorInitialized = true;

                m_DepthBuffer.resize(m_DepthSize.x * m_DepthSize.y);
                m_ColorBuffer.resize(m_DepthSize.x * m_DepthSize.y * 4);

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
                TextureDescriptor.m_Format = Gfx::CTexture::R16_UINT;

                m_DepthTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);
                
                TextureDescriptor.m_Format = Gfx::CTexture::R8G8B8A8_UBYTE;

                m_RGBATexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

                GetDepthBuffer = (GetDepthBufferFunc)(Core::PluginManager::GetPluginFunction("Kinect", "GetDepthBuffer"));
                GetColorBuffer = (GetColorBufferFunc)(Core::PluginManager::GetPluginFunction("Kinect", "GetColorBuffer"));

                MR::SReconstructionSettings Settings;
                m_Reconstructor.GetReconstructionSettings(&Settings);
                m_CaptureColor = Settings.m_CaptureColor;
            }
            else
            {
                BASE_THROWM("Unknown data source for SLAM plugin");
            }

            std::string RecordParam = Core::CProgramParameters::GetInstance().Get("mr:slam:recording:mode", "none");
            
            if (RecordParam == "record")
            {
                auto FileName = Core::CProgramParameters::GetInstance().Get("mr:slam:recording:file", "");

                m_RecordMode = RECORD;
                m_RecordFile.open(FileName, std::fstream::out | std::fstream::binary);

                ENGINE_CONSOLE_INFOV("Recoding into file file \"%s\"", FileName.c_str());
            }
            else if (RecordParam == "none")
            {
                m_RecordMode = NONE;
            }
            else
            {
                BASE_THROWM("Invalid recording mode!");
            }

            m_SendInpaintedResult = Core::CProgramParameters::GetInstance().Get("mr:diminished_reality:send_result", true);

            m_pPlaneColorizer = std::make_unique<MR::CPlaneColorizer>(&m_Reconstructor);
        }

        // -----------------------------------------------------------------------------

        void Exit()
        {
            m_RecordFile.close();

            m_DepthBuffer.clear();
            m_ColorBuffer.clear();

            m_Reconstructor.Exit();

            m_DepthTexture = nullptr;
            m_RGBATexture = nullptr;

            m_SLAMNetHandle = nullptr;
            
            m_YUVtoRGBCSPtr = nullptr;
            m_YTexture = nullptr;
            m_UVTexture = nullptr;

            m_ShiftTexture = nullptr;
            m_ShiftDepthCSPtr = nullptr;
            m_ShiftLUTPtr = nullptr;

            m_PlaneTexture = nullptr;
            
            Gfx::SelectionRenderer::Clear(*m_pSelectionTicket);
        }

        // -----------------------------------------------------------------------------

        void Update()
        {
            // -----------------------------------------------------------------------------
            // Selection
            // -----------------------------------------------------------------------------
            if (!m_SelectionFlag) m_SelectionState = ESelection::NOSELECTION;

            Gfx::CSelectionTicket& rSelectionTicket = *m_pSelectionTicket;

            if (m_SelectionState == ESelection::FIRSTPRESS)
            {
                Gfx::SelectionRenderer::PushPick(rSelectionTicket, glm::ivec2(m_LatestCursorPosition));
            }

            if (Gfx::SelectionRenderer::PopPick(rSelectionTicket))
            {
                if (rSelectionTicket.m_HitFlag == Gfx::SHitFlag::Entity)
                {
                    Gfx::ReconstructionRenderer::AddPositionToSelection(rSelectionTicket.m_WSPosition);
                }
            }

            // -----------------------------------------------------------------------------
            // Playing
            // -----------------------------------------------------------------------------
            if (m_RecordMode == PLAY && m_pRecordReader != nullptr)
            {
                m_pRecordReader->Update();

                if (m_pRecordReader->IsEnd())
                {
                    m_RecordMode = NONE;
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

                    HandleMessage(Message);
                }
            }

            // -----------------------------------------------------------------------------
            // Devices
            // -----------------------------------------------------------------------------
            if (m_DataSource == KINECT)
            {
                if (m_CaptureColor && GetDepthBuffer(m_DepthBuffer.data()) && GetColorBuffer(m_ColorBuffer.data()))
                {
                    Base::AABB2UInt TargetRect;
                    TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_DepthSize.x, m_DepthSize.y));

                    Gfx::TextureManager::CopyToTexture2D(m_DepthTexture, TargetRect, m_DepthSize.x, m_DepthBuffer.data());
                    Gfx::TextureManager::CopyToTexture2D(m_RGBATexture, TargetRect, m_DepthSize.x, m_ColorBuffer.data());

                    m_Reconstructor.OnNewFrame(m_DepthTexture, m_RGBATexture, nullptr);
                }
                else if (GetDepthBuffer(m_DepthBuffer.data()))
                {
                    Base::AABB2UInt TargetRect;
                    TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_DepthSize.x, m_DepthSize.y));
                    Gfx::TextureManager::CopyToTexture2D(m_DepthTexture, TargetRect, m_DepthSize.x, m_DepthBuffer.data());
                    m_Reconstructor.OnNewFrame(m_DepthTexture, nullptr, nullptr);
                }
            }

            if (m_UseTrackingCamera)
            {
                auto& rControl = static_cast<Cam::CEditorControl&>(Cam::ControlManager::GetActiveControl());

                // -----------------------------------------------------------------------------
                // Projection
                // -----------------------------------------------------------------------------
                rControl.SetProjectionMatrix(glm::transpose(m_DeviceProjectionMatrix));

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

            if (m_SendInpaintedResult && Net::CNetworkManager::GetInstance().IsConnected(m_SLAMSocket))
            {
                SendInpaintedResult();
            }

            if (m_StreamState == STREAM_DIMINSIHED)
            {
                auto AABB = Gfx::ReconstructionRenderer::GetSelectionBox();

                AABB.SetMax(AABB.GetMax() + glm::vec3(0.0f, 0.0f, 5.0f));

                Gfx::CTexturePtr Texture = Gfx::ReconstructionRenderer::GetInpaintedRendering(m_PoseMatrix, AABB, m_RGBATexture);
            }
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent)
        {
            if (!m_IsReconstructorInitialized)
            {
                return;
            }

            if (_rEvent.GetAction() == Base::CInputEvent::MouseLeftPressed)
            {
                m_SelectionState = ESelection::FIRSTPRESS;

                m_LatestCursorPosition = _rEvent.GetLocalCursorPosition();
            }
            else if (_rEvent.GetAction() == Base::CInputEvent::MouseLeftReleased)
            {
                m_SelectionState = ESelection::FIRSTRELEASE;
            }
            else if (_rEvent.GetAction() == Base::CInputEvent::MouseMove && m_SelectionState == ESelection::FIRSTPRESS)
            {
                m_LatestCursorPosition = _rEvent.GetLocalCursorPosition();
            }
            else if (_rEvent.GetAction() == Base::CInputEvent::MouseRightReleased && m_SelectionState == ESelection::FIRSTPRESS)
            {
                Gfx::ReconstructionRenderer::ResetSelection();
            }
            else if (_rEvent.GetAction() == Base::CInputEvent::MouseWheel)
            {
                auto AABB = Gfx::ReconstructionRenderer::GetSelectionBox();

                Gfx::ReconstructionRenderer::AddPositionToSelection(AABB.GetMax() + glm::vec3(0.0f, 0.0f, 0.1f));
            }
        }

        // -----------------------------------------------------------------------------

        void SetRecordFile(const std::string& _rFileName, float _Speed = 1.0f)
        {
            std::string RecordParam = Core::CProgramParameters::GetInstance().Get("mr:slam:recording:mode", "none");

            m_RecordFile.open(Core::AssetManager::GetPathToAssets() + "/" + _rFileName, std::fstream::in | std::fstream::binary);

            if (!m_RecordFile.is_open())
            {
                ENGINE_CONSOLE_INFOV("File %s not found", _rFileName.c_str());
            }

            m_pRecordReader = std::make_unique<Base::CRecordReader>(m_RecordFile, 1);

            m_pRecordReader->SkipTime();

            m_pRecordReader->SetSpeed(_Speed);

            ENGINE_CONSOLE_INFOV("Playing recording from file \"%s\"", _rFileName.c_str());
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
            m_SelectionFlag = _Flag;
        }

        // -----------------------------------------------------------------------------

        void EnableMouseControl(bool _Flag)
        {
            m_UseTrackingCamera = !_Flag;
        }

        // -----------------------------------------------------------------------------

        void ColorizePlanes()
        {
            m_pPlaneColorizer->ColorizeAllPlanes();
        }

        // -----------------------------------------------------------------------------

        void SetIsPlaying(bool _Flag)
        {
            if (m_RecordMode != RECORD)
            {
                m_RecordMode = _Flag ? PLAY : NONE;
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

			if (Net::CNetworkManager::GetInstance().IsConnected(m_SLAMSocket))
			{
				for (auto& [rPlaneID, rPlane] : m_Reconstructor.GetPlanes())
				{
					int32_t TextureWidth = rPlane.m_TexturePtr->GetNumberOfPixelsU();
					int32_t TextureHeight = rPlane.m_TexturePtr->GetNumberOfPixelsV();

					auto Transform = glm::eulerAngleX(-glm::half_pi<float>()) * rPlane.m_Transform;
					Transform = glm::transpose(Transform);

					auto VertexCount = static_cast<uint32_t>(rPlane.m_Vertices.size());
					auto IndexCount = static_cast<uint32_t>(rPlane.m_Indices.size());

					std::vector<glm::vec3> Vertices;
					std::vector<glm::vec2> UV;
					std::vector<uint16_t> Indices;

					Vertices.reserve(VertexCount);
					UV.reserve(VertexCount);
					Indices.reserve(IndexCount);

					for (auto& Vertex : rPlane.m_Vertices)
					{
						Vertices.push_back(Vertex.m_Position);
						UV.push_back(Vertex.m_UV);
					}

					for (auto& Index : rPlane.m_Indices)
					{
						Indices.push_back(static_cast<uint16_t>(Index));
					}

					int32_t MessageID = PLANE;

					int VerticesMemSize = VertexCount * sizeof(Vertices[0]);
					int UVMemSize = VertexCount * sizeof(UV[0]);
					int IndicesMemSize = IndexCount * sizeof(Indices[0]);

					int MessageLength = sizeof(MessageID) + static_cast<int>(rPlaneID.size()); // Message ID + Plane ID
					MessageLength += sizeof(rPlane.m_Extent) + sizeof(rPlane.m_Transform);
					MessageLength += VerticesMemSize + UVMemSize + IndicesMemSize + 3 * sizeof(uint32_t); // Mesh + Counters
					MessageLength += TextureWidth * TextureHeight * 4 + 2 * sizeof(int32_t); // Texture size + RGBA data

					std::vector<char> Payload(MessageLength);

					int Offset = 0;

					std::memcpy(Payload.data() + Offset, &MessageID, sizeof(MessageID));
					Offset += sizeof(MessageID);

					std::memcpy(Payload.data() + Offset, rPlaneID.data(), rPlaneID.size());
					Offset += static_cast<int>(rPlaneID.size());

					std::memcpy(Payload.data() + Offset, &rPlane.m_Extent, sizeof(rPlane.m_Extent));
					Offset += sizeof(rPlane.m_Extent);

					std::memcpy(Payload.data() + Offset, &Transform, sizeof(Transform));
					Offset += sizeof(Transform);

					std::memcpy(Payload.data() + Offset, &VertexCount, sizeof(VertexCount));
					Offset += sizeof(VertexCount);

					std::memcpy(Payload.data() + Offset, Vertices.data(), VerticesMemSize);
					Offset += VerticesMemSize;

					std::memcpy(Payload.data() + Offset, &VertexCount, sizeof(VertexCount)); // UV count is the same as vertex count
					Offset += sizeof(VertexCount);

					std::memcpy(Payload.data() + Offset, UV.data(), UVMemSize);
					Offset += UVMemSize;

					std::memcpy(Payload.data() + Offset, &IndexCount, sizeof(IndexCount)); // UV count is the same as vertex count
					Offset += sizeof(IndexCount);

					std::memcpy(Payload.data() + Offset, Indices.data(), IndicesMemSize);
					Offset += IndicesMemSize;

					std::memcpy(Payload.data() + Offset, &TextureWidth, sizeof(TextureWidth));
					Offset += sizeof(TextureWidth);

					std::memcpy(Payload.data() + Offset, &TextureHeight, sizeof(TextureHeight));
					Offset += sizeof(TextureHeight);

					Gfx::TextureManager::CopyTextureToCPU(rPlane.m_TexturePtr, Payload.data() + Offset);
					Offset += 4 * TextureWidth * TextureHeight;

					Net::CMessage Message;
					Message.m_Category = 0;
					Message.m_CompressedSize = MessageLength;
					Message.m_DecompressedSize = MessageLength;
					Message.m_MessageType = 0;
					Message.m_Payload = std::move(Payload);

					Net::CNetworkManager::GetInstance().SendMessage(m_SLAMSocket, Message);
				}
			}
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

        void SendInpaintedResult()
        {
            const auto& AABB = Gfx::ReconstructionRenderer::GetSelectionBox();

            Gfx::CTexturePtr Texture = Gfx::ReconstructionRenderer::GetInpaintedRendering(m_PoseMatrix, AABB);

            if (Texture != nullptr)
            {
                glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PixelDataBuffer);

                Gfx::TextureManager::CopyTextureToCPU(Texture, nullptr);

                glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                
                GLsync Fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

                //auto Status = glClientWaitSync(Fence, GL_SYNC_FLUSH_COMMANDS_BIT, INT_MAX);
                glClientWaitSync(Fence, GL_SYNC_FLUSH_COMMANDS_BIT, INT_MAX);

                std::vector<char> Compressed;

                Base::Compress(static_cast<char*>(m_pGPUPixelData), m_PixelBufferSize, Compressed, 1);

                Net::CMessage Message;
                Message.m_Category = 0;
                Message.m_CompressedSize = static_cast<int>(Compressed.size());
                Message.m_DecompressedSize = m_PixelBufferSize;
                Message.m_MessageType = 0;
                Message.m_Payload = std::move(Compressed);

                //Net::CNetworkManager::GetInstance().SendMessage(m_SLAMSocket, Message);
            }
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
                else if (MessageID == 2)
                {
                    auto ColorSize = *reinterpret_cast<const glm::ivec2*>(Decompressed.data() + 2 * sizeof(int32_t));

                    EnableDiminishedReality(ColorSize);
                }
            }
            else if (MessageType == TRANSFORM)
            {
                if (m_StreamState == STREAM_SLAM)
                {
                    m_PoseMatrix = *reinterpret_cast<glm::mat4*>(Decompressed.data() + sizeof(int32_t)) * glm::eulerAngleX(glm::pi<float>());
                }
                else if (m_StreamState == STREAM_DIMINSIHED)
                {
                    m_PreliminaryPoseMatrix = *reinterpret_cast<glm::mat4*>(Decompressed.data() + sizeof(int32_t)) * glm::eulerAngleX(glm::pi<float>());
                }                
            }
            else if (MessageType == DEPTHFRAME)
            {
                //int32_t Width = *reinterpret_cast<int32_t*>(Decompressed.data() + sizeof(int32_t));
                //int32_t Height = *reinterpret_cast<int32_t*>(Decompressed.data() + 2 * sizeof(int32_t));

                m_DepthIntrinsics.m_FocalLength.x = *reinterpret_cast<float*>(Decompressed.data() + 3 * sizeof(int32_t));
                m_DepthIntrinsics.m_FocalLength.y = *reinterpret_cast<float*>(Decompressed.data() + 4 * sizeof(int32_t));
                m_DepthIntrinsics.m_FocalPoint.x = *reinterpret_cast<float*>(Decompressed.data() + 5 * sizeof(int32_t));
                m_DepthIntrinsics.m_FocalPoint.y = *reinterpret_cast<float*>(Decompressed.data() + 6 * sizeof(int32_t));

                const uint16_t* RawBuffer = reinterpret_cast<uint16_t*>(Decompressed.data() + 7 * sizeof(int32_t));

                Base::AABB2UInt TargetRect;
                TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_DepthSize));
                Gfx::TextureManager::CopyToTexture2D(m_ShiftTexture, TargetRect, m_DepthSize.x, const_cast<uint16_t*>(RawBuffer));

                Gfx::ContextManager::SetShaderCS(m_ShiftDepthCSPtr);
                Gfx::ContextManager::SetImageTexture(0, m_ShiftTexture);
                Gfx::ContextManager::SetImageTexture(1, m_UnregisteredDepthTexture);
                Gfx::ContextManager::SetImageTexture(2, m_ShiftLUTPtr);

                int WorkgroupsX = DivUp(m_CaptureColor ? m_ColorSize.x : m_DepthSize.x, m_TileSize2D);
                int WorkgroupsY = DivUp(m_CaptureColor ? m_ColorSize.y : m_DepthSize.y, m_TileSize2D);
                Gfx::ContextManager::Dispatch(WorkgroupsX, WorkgroupsY, 1);
                
                SRegisteringBuffer BufferData;
                BufferData.m_ColorIntrinsics = m_ColorIntrinsics;
                BufferData.m_DepthIntrinsics = m_DepthIntrinsics;
                BufferData.m_RelativeCameraTransform = m_RelativeCameraTransform;

                Gfx::BufferManager::UploadBufferData(m_RegisteringBufferPtr, &BufferData);

                Gfx::ContextManager::SetConstantBuffer(0, m_RegisteringBufferPtr);

                Gfx::ContextManager::SetImageTexture(0, m_DepthTexture);
                Gfx::ContextManager::SetShaderCS(m_RegisterDepthCSPtr);
                Gfx::ContextManager::Dispatch(WorkgroupsX, WorkgroupsY, 1);

                if (!m_CaptureColor && m_StreamState == STREAM_SLAM)
                {
                    m_Reconstructor.OnNewFrame(m_DepthTexture, nullptr, &m_PoseMatrix, m_DepthIntrinsics.m_FocalLength, m_DepthIntrinsics.m_FocalPoint);
                }
            }
            else if (MessageType == COLORFRAME && m_CaptureColor)
            {
                ExtractRGBAFrame(Decompressed);

                if (m_StreamState == STREAM_SLAM)
                {
                    m_Reconstructor.OnNewFrame(m_DepthTexture, m_RGBATexture, &m_PoseMatrix, m_ColorIntrinsics.m_FocalLength, m_ColorIntrinsics.m_FocalPoint);
                }
                else
                {
                    m_PoseMatrix = m_PreliminaryPoseMatrix;
                }
            }
            else if (MessageType == LIGHTESTIMATE)
            {
                const float AmbientIntensity = *reinterpret_cast<float*>(Decompressed.data() + sizeof(int32_t));
                const float LightTemperature = *reinterpret_cast<float*>(Decompressed.data() + sizeof(int32_t) + sizeof(float));
            }
            else if (MessageType == PLANE)
            {
                int Offset = sizeof(int32_t);

				std::string PlaneID(Decompressed.data() + Offset, Decompressed.data() + Offset + 16);

				Offset += static_cast<int>(PlaneID.size());
                int PlaneAction = *reinterpret_cast<int*>(Decompressed.data() + Offset);

                Offset += sizeof(PlaneAction);
                glm::mat4 PlaneTransform = *reinterpret_cast<glm::mat4*>(Decompressed.data() + Offset);

                Offset += sizeof(PlaneTransform);
                glm::vec4 PlaneExtent = *reinterpret_cast<glm::vec4*>(Decompressed.data() + Offset);

                Offset += sizeof(PlaneExtent);

                PlaneTransform = glm::eulerAngleX(glm::half_pi<float>()) * PlaneTransform;

                if (Offset < Decompressed.size()) // Is there additional data (a mesh)?
                {
                    int VertexCount = *reinterpret_cast<int*>(Decompressed.data() + Offset);

                    Offset += sizeof(VertexCount);
                    glm::vec4* pVertices = reinterpret_cast<glm::vec4*>(Decompressed.data() + Offset);

                    Offset += VertexCount * sizeof(pVertices[0]);
                    int UVCount = *reinterpret_cast<int*>(Decompressed.data() + Offset);

                    Offset += sizeof(UVCount);
                    glm::vec2* pUV = reinterpret_cast<glm::vec2*>(Decompressed.data() + Offset);

                    Offset += UVCount * sizeof(pUV[0]);
                    int IndexCount = *reinterpret_cast<int*>(Decompressed.data() + Offset);

                    Offset += sizeof(IndexCount);
                    uint16_t* pIndices = reinterpret_cast<uint16_t*>(Decompressed.data() + Offset);

                    Offset += IndexCount * sizeof(pIndices[0]);

                    assert(UVCount == VertexCount);
                    assert(IndexCount % 3 == 0);

                    std::vector<CSLAMReconstructor::SPlaneVertex> Vertices;
                    std::vector<uint32_t> Indices;

                    for (int i = 0; i < VertexCount; ++ i)
                    {
                        CSLAMReconstructor::SPlaneVertex Vertex;

                        Vertex.m_Position = glm::vec3(pVertices[i].x, pVertices[i].y, pVertices[i].z);
                        Vertex.m_UV = glm::vec2(pUV[i].x, pUV[i].y);

                        Vertices.push_back(Vertex);
                    }

                    for (int i = 0; i < IndexCount; ++i)
                    {
                        Indices.push_back(pIndices[i]);
                    }

                    switch (PlaneAction)
                    {
                    case ADDPLANE:
                        m_Reconstructor.AddPlaneWithMesh(PlaneTransform, PlaneExtent, Vertices, Indices, PlaneID);
                        break;
                    case UPDATEPLANE:
                        m_Reconstructor.UpdatePlaneWithMesh(PlaneTransform, PlaneExtent, Vertices, Indices, PlaneID);
                        break;
                    case REMOVEPLANE:
                        m_Reconstructor.RemovePlane(PlaneID);
                        break;
                    }
                }
                else
                {
                    switch (PlaneAction)
                    {
                    case ADDPLANE:
                        m_Reconstructor.AddPlane(PlaneTransform, PlaneExtent, PlaneID);
                        break;
                    case UPDATEPLANE:
                        m_Reconstructor.UpdatePlane(PlaneTransform, PlaneExtent, PlaneID);
                        break;
                    case REMOVEPLANE:
                        m_Reconstructor.RemovePlane(PlaneID);
                        break;
                    }
                }

                m_pPlaneColorizer->UpdatePlane(PlaneID);
            }
        }

        // -----------------------------------------------------------------------------

        void ExtractRGBAFrame(const std::vector<char>& _rData)
        {
            const int32_t Width = *reinterpret_cast<const int32_t*>(_rData.data() + sizeof(int32_t));
            const int32_t Height = *reinterpret_cast<const int32_t*>(_rData.data() + 2 * sizeof(int32_t));

            m_ColorIntrinsics.m_FocalLength.x = *reinterpret_cast<const float*>(_rData.data() + 3 * sizeof(int32_t));
            m_ColorIntrinsics.m_FocalLength.y = *reinterpret_cast<const float*>(_rData.data() + 4 * sizeof(int32_t));
            m_ColorIntrinsics.m_FocalPoint.x = *reinterpret_cast<const float*>(_rData.data() + 5 * sizeof(int32_t));
            m_ColorIntrinsics.m_FocalPoint.y = *reinterpret_cast<const float*>(_rData.data() + 6 * sizeof(int32_t));

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
            m_RelativeCameraTransform = _rMessage.m_RelativeCameraTransform;
            m_DeviceProjectionMatrix = _rMessage.m_DeviceProjectionMatrix;

            Gfx::ReconstructionRenderer::SetDeviceResolution(m_DeviceResolution);

            MR::SReconstructionSettings Settings;
            m_Reconstructor.GetReconstructionSettings(&Settings);

            m_CaptureColor = Settings.m_CaptureColor;

            if (m_CaptureColor)
            {
                FocalPoint.x = (FocalPoint.x / m_DepthSize.x) * m_ColorSize.x;
                FocalPoint.y = (FocalPoint.y / m_DepthSize.y) * m_ColorSize.y;

                m_Reconstructor.SetImageSizes(glm::vec2(m_ColorSize), glm::vec2(m_ColorSize));
                m_Reconstructor.SetIntrinsics(glm::vec2(FocalLength), glm::vec2(FocalPoint));
            }
            else
            {
                m_Reconstructor.SetImageSizes(glm::vec2(m_DepthSize), glm::vec2(m_DepthSize));
                m_Reconstructor.SetIntrinsics(glm::vec2(FocalLength), glm::vec2(FocalPoint));
            }

            m_Reconstructor.Start();

            m_IsReconstructorInitialized = true;

            m_DepthBuffer.resize(m_DepthSize.x * m_DepthSize.y);

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
            TextureDescriptor.m_Format = Gfx::CTexture::R16_UINT;
            m_ShiftTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

            TextureDescriptor.m_NumberOfPixelsU = m_CaptureColor ? m_ColorSize.x : m_DepthSize.x;
            TextureDescriptor.m_NumberOfPixelsV = m_CaptureColor ? m_ColorSize.y : m_DepthSize.y;
            m_DepthTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);
            m_UnregisteredDepthTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

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
            m_ShiftDepthCSPtr = Gfx::ShaderManager::CompileCS("../../plugins/slam/cs_shift_depth.glsl", "main", DefineString.c_str());

            m_RegisterDepthCSPtr = Gfx::ShaderManager::CompileCS("../../plugins/slam/cs_register_depth.glsl", "main", DefineString.c_str());

            if (m_SendInpaintedResult)
            {
                m_PixelBufferSize = m_DeviceResolution.x * m_DeviceResolution.y * 4;
                glCreateBuffers(1, &m_PixelDataBuffer);

                // The GPU buffer has three times the size of the image so we can use triple buffering and avoid stalls

                glNamedBufferStorage(m_PixelDataBuffer, m_PixelBufferSize * 3, nullptr, GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT);
                m_pGPUPixelData = glMapNamedBufferRange(m_PixelDataBuffer, 0, m_PixelBufferSize * 3, GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT);
            }

            ENGINE_CONSOLE_INFO("Initialization complete");
        }

        // -----------------------------------------------------------------------------

        void EnableDiminishedReality(const glm::ivec2& _ColorSize)
        {
            CreatePlane();
            m_StreamState = STREAM_DIMINSIHED;

            m_ColorSize = _ColorSize;

            Gfx::STextureDescriptor TextureDescriptor = {};

            TextureDescriptor.m_NumberOfPixelsU = m_ColorSize.x;
            TextureDescriptor.m_NumberOfPixelsV = m_ColorSize.y;
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_NumberOfMipMaps = 1;
            TextureDescriptor.m_NumberOfTextures = 1;
            TextureDescriptor.m_Binding = Gfx::CTexture::ShaderResource;
            TextureDescriptor.m_Access = Gfx::CTexture::CPUWrite;
            TextureDescriptor.m_Usage = Gfx::CTexture::GPUReadWrite;
            TextureDescriptor.m_Semantic = Gfx::CTexture::UndefinedSemantic;
            TextureDescriptor.m_pFileName = nullptr;
            TextureDescriptor.m_pPixels = nullptr;
            TextureDescriptor.m_Format = Gfx::CTexture::R8G8B8A8_UBYTE;
            m_RGBATexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

            TextureDescriptor.m_Format = Gfx::CTexture::R8_UBYTE;
            m_YTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

            TextureDescriptor.m_NumberOfPixelsU = m_ColorSize.x / 2;
            TextureDescriptor.m_NumberOfPixelsV = m_ColorSize.y / 2;
            TextureDescriptor.m_Format = Gfx::CTexture::R8G8_UBYTE;
            m_UVTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

            std::stringstream DefineStream;
            DefineStream
                << "#define TILE_SIZE_2D " << m_TileSize2D << " \n"
                << "#define DEPTH_WIDTH " << m_DepthSize.x << " \n"
                << "#define DEPTH_HEIGHT " << m_DepthSize.y << " \n"
                << "#define COLOR_WIDTH " << m_ColorSize.x << " \n"
                << "#define COLOR_HEIGHT " << m_ColorSize.y << " \n"
                << "#define CAPTURE_COLOR " << " \n";

            std::string DefineString = DefineStream.str();
            m_YUVtoRGBCSPtr = Gfx::ShaderManager::CompileCS("../../plugins/slam/cs_yuv_to_rgb.glsl", "main", DefineString.c_str());
        }

        // -----------------------------------------------------------------------------

        void OnNewSLAMMessage(const Net::CMessage& _rMessage, Net::SocketHandle _SocketHandle)
        {
            BASE_UNUSED(_SocketHandle);
            
            if (_rMessage.m_MessageType == 0)
            {
                if (m_RecordMode == RECORD)
                {
                    if (m_pRecordWriter == nullptr)
                    {
                        m_pRecordWriter = std::make_unique<Base::CRecordWriter>(m_RecordFile, 1);
                    }

                    *m_pRecordWriter << _rMessage.m_Category;
                    *m_pRecordWriter << _rMessage.m_MessageType;
                    *m_pRecordWriter << _rMessage.m_CompressedSize;
                    *m_pRecordWriter << _rMessage.m_DecompressedSize;
                    Base::Write(*m_pRecordWriter, _rMessage.m_Payload);
                }

                HandleMessage(_rMessage);
            }
            else if (_rMessage.m_MessageType == 2)
            {
                // Enable mouse control after disconnect
                m_UseTrackingCamera = false;
            }
        }

        // -----------------------------------------------------------------------------

        void OnNewNeuralNetMessage(const Net::CMessage& _rMessage, Net::SocketHandle _SocketHandle)
        {
            BASE_UNUSED(_SocketHandle);
            BASE_UNUSED(_rMessage);   

            if (_rMessage.m_MessageType == 0)
            {
                auto ScaledResolution = static_cast<int>(m_PlaneResolution / m_PlaneScale);
                int BorderSize = (m_PlaneResolution - ScaledResolution) / 2;
                int Min = BorderSize;
                int Max = m_PlaneResolution - BorderSize;

                ENGINE_CONSOLE_INFO("Received inpainted plane");
                auto TargetRect = Base::AABB2UInt(glm::uvec2(Min, Min), glm::uvec2(Max, Max));
                Gfx::TextureManager::CopyToTexture2D(m_PlaneTexture, TargetRect, ScaledResolution * 4, const_cast<char*>(_rMessage.m_Payload.data()), true);

                const auto& AABB = Gfx::ReconstructionRenderer::GetSelectionBox();
                Gfx::ReconstructionRenderer::SetInpaintedPlane(m_PlaneTexture, AABB);
            }
        }

        // -----------------------------------------------------------------------------

        void CreatePlane()
        {
            if (!m_IsReconstructorInitialized)
            {
                ENGINE_CONSOLE_INFO("Reconstruction is not initialized");
                return;
            }

            const auto& AABB = Gfx::ReconstructionRenderer::GetSelectionBox();
            m_PlaneTexture = m_Reconstructor.CreatePlaneTexture(AABB);

            if (m_InpaintingMode == INPAINTING_NN)
            {
                if (!Net::CNetworkManager::GetInstance().IsConnected(m_NeuralNetworkSocket))
                {
                    ENGINE_CONSOLE_INFO("Cannot send plane to neural net because the socket has no connection");
                    Gfx::ReconstructionRenderer::SetInpaintedPlane(m_PlaneTexture, AABB);
                    return;
                }

                Net::CMessage Message;

                Message.m_Category = 0;
                Message.m_Payload = std::vector<char>(m_PlaneResolution * m_PlaneResolution * 4);
                Message.m_CompressedSize = Message.m_DecompressedSize = static_cast<int>(Message.m_Payload.size());
                Message.m_MessageType = 0;

                Gfx::TextureManager::CopyTextureToCPU(m_PlaneTexture, Message.m_Payload.data());

                Net::CNetworkManager::GetInstance().SendMessage(m_NeuralNetworkSocket, Message);
            }
            else if (m_InpaintingMode == INPAINTING_PIXMIX || m_InpaintingMode == INPAINTING_PIXMIX_OCEAN)
            {
				std::vector<glm::u8vec4> RawData(m_PlaneResolution * m_PlaneResolution);

                Gfx::TextureManager::CopyTextureToCPU(m_PlaneTexture, reinterpret_cast<char*>(RawData.data()));

				std::vector<glm::u8vec4> InpaintedImage(m_PlaneResolution * m_PlaneResolution);

                InpaintWithPixMix(glm::ivec2(m_PlaneResolution, m_PlaneResolution), RawData, InpaintedImage);

                auto TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_PlaneResolution, m_PlaneResolution));
                Gfx::TextureManager::CopyToTexture2D(m_PlaneTexture, TargetRect, m_PlaneResolution, reinterpret_cast<char*>(InpaintedImage.data()), true);

                Gfx::ReconstructionRenderer::SetInpaintedPlane(m_PlaneTexture, AABB);
            }
            else
            {
                Gfx::ReconstructionRenderer::SetInpaintedPlane(m_PlaneTexture, AABB);
            }
        }

        // -----------------------------------------------------------------------------

        void CreateRegisteringBuffer()
        {
            Gfx::SBufferDescriptor BufferDesc = {};
            BufferDesc.m_Binding = Gfx::CBuffer::ConstantBuffer;
            BufferDesc.m_Access = Gfx::CBuffer::CPUWrite;
            BufferDesc.m_NumberOfBytes = sizeof(SRegisteringBuffer);
            BufferDesc.m_Usage = Gfx::CBuffer::GPURead;

            m_RegisteringBufferPtr = Gfx::BufferManager::CreateBuffer(BufferDesc);
        }

        // -----------------------------------------------------------------------------

        void CreateShiftLUTTexture()
        {
            static const uint16_t LUT[] = { 0,
                264, 264, 265, 265, 265, 265, 265, 266, 266, 266, 266, 267, 267, 267, 267, 268, 268, 268,
                268, 269, 269, 269, 269, 270, 270, 270, 270, 271, 271, 271, 271, 272, 272, 272, 272, 273,
                273, 273, 273, 274, 274, 274, 274, 275, 275, 275, 275, 276, 276, 276, 276, 277, 277, 277,
                277, 278, 278, 278, 278, 279, 279, 279, 279, 280, 280, 280, 280, 281, 281, 281, 281, 282,
                282, 282, 283, 283, 283, 283, 284, 284, 284, 284, 285, 285, 285, 286, 286, 286, 286, 287,
                287, 287, 287, 288, 288, 288, 289, 289, 289, 289, 290, 290, 290, 291, 291, 291, 291, 292,
                292, 292, 293, 293, 293, 293, 294, 294, 294, 295, 295, 295, 295, 296, 296, 296, 297, 297,
                297, 297, 298, 298, 298, 299, 299, 299, 300, 300, 300, 300, 301, 301, 301, 302, 302, 302,
                303, 303, 303, 304, 304, 304, 304, 305, 305, 305, 306, 306, 306, 307, 307, 307, 308, 308,
                308, 309, 309, 309, 309, 310, 310, 310, 311, 311, 311, 312, 312, 312, 313, 313, 313, 314,
                314, 314, 315, 315, 315, 316, 316, 316, 317, 317, 317, 318, 318, 318, 319, 319, 319, 320,
                320, 320, 321, 321, 321, 322, 322, 322, 323, 323, 324, 324, 324, 325, 325, 325, 326, 326,
                326, 327, 327, 327, 328, 328, 329, 329, 329, 330, 330, 330, 331, 331, 331, 332, 332, 333,
                333, 333, 334, 334, 334, 335, 335, 336, 336, 336, 337, 337, 337, 338, 338, 339, 339, 339,
                340, 340, 340, 341, 341, 342, 342, 342, 343, 343, 344, 344, 344, 345, 345, 346, 346, 346,
                347, 347, 348, 348, 348, 349, 349, 350, 350, 350, 351, 351, 352, 352, 353, 353, 353, 354,
                354, 355, 355, 355, 356, 356, 357, 357, 358, 358, 358, 359, 359, 360, 360, 361, 361, 361,
                362, 362, 363, 363, 364, 364, 365, 365, 365, 366, 366, 367, 367, 368, 368, 369, 369, 369,
                370, 370, 371, 371, 372, 372, 373, 373, 374, 374, 375, 375, 376, 376, 376, 377, 377, 378,
                378, 379, 379, 380, 380, 381, 381, 382, 382, 383, 383, 384, 384, 385, 385, 386, 386, 387,
                387, 388, 388, 389, 389, 390, 390, 391, 391, 392, 392, 393, 393, 394, 394, 395, 395, 396,
                396, 397, 397, 398, 399, 399, 400, 400, 401, 401, 402, 402, 403, 403, 404, 404, 405, 406,
                406, 407, 407, 408, 408, 409, 409, 410, 411, 411, 412, 412, 413, 413, 414, 415, 415, 416,
                416, 417, 417, 418, 419, 419, 420, 420, 421, 422, 422, 423, 423, 424, 425, 425, 426, 426,
                427, 428, 428, 429, 429, 430, 431, 431, 432, 433, 433, 434, 434, 435, 436, 436, 437, 438,
                438, 439, 439, 440, 441, 441, 442, 443, 443, 444, 445, 445, 446, 447, 447, 448, 449, 449,
                450, 451, 451, 452, 453, 453, 454, 455, 456, 456, 457, 458, 458, 459, 460, 460, 461, 462,
                463, 463, 464, 465, 465, 466, 467, 468, 468, 469, 470, 471, 471, 472, 473, 474, 474, 475,
                476, 477, 477, 478, 479, 480, 480, 481, 482, 483, 484, 484, 485, 486, 487, 487, 488, 489,
                490, 491, 491, 492, 493, 494, 495, 496, 496, 497, 498, 499, 500, 500, 501, 502, 503, 504,
                505, 506, 506, 507, 508, 509, 510, 511, 512, 512, 513, 514, 515, 516, 517, 518, 519, 520,
                520, 521, 522, 523, 524, 525, 526, 527, 528, 529, 530, 531, 532, 533, 533, 534, 535, 536,
                537, 538, 539, 540, 541, 542, 543, 544, 545, 546, 547, 548, 549, 550, 551, 552, 553, 554,
                555, 556, 557, 558, 559, 560, 561, 563, 564, 565, 566, 567, 568, 569, 570, 571, 572, 573,
                574, 575, 577, 578, 579, 580, 581, 582, 583, 584, 586, 587, 588, 589, 590, 591, 593, 594,
                595, 596, 597, 599, 600, 601, 602, 603, 605, 606, 607, 608, 609, 611, 612, 613, 614, 616,
                617, 618, 620, 621, 622, 623, 625, 626, 627, 629, 630, 631, 633, 634, 635, 637, 638, 639,
                641, 642, 644, 645, 646, 648, 649, 650, 652, 653, 655, 656, 658, 659, 661, 662, 663, 665,
                666, 668, 669, 671, 672, 674, 675, 677, 678, 680, 682, 683, 685, 686, 688, 689, 691, 693,
                694, 696, 697, 699, 701, 702, 704, 706, 707, 709, 711, 712, 714, 716, 717, 719, 721, 723,
                724, 726, 728, 730, 732, 733, 735, 737, 739, 741, 742, 744, 746, 748, 750, 752, 754, 755,
                757, 759, 761, 763, 765, 767, 769, 771, 773, 775, 777, 779, 781, 783, 785, 787, 789, 791,
                794, 796, 798, 800, 802, 804, 806, 808, 811, 813, 815, 817, 820, 822, 824, 826, 829, 831,
                833, 836, 838, 840, 843, 845, 847, 850, 852, 855, 857, 860, 862, 864, 867, 869, 872, 875,
                877, 880, 882, 885, 888, 890, 893, 895, 898, 901, 904, 906, 909, 912, 915, 917, 920, 923,
                926, 929, 932, 935, 937, 940, 943, 946, 949, 952, 955, 958, 962, 965, 968, 971, 974, 977,
                980, 984, 987, 990, 993, 997, 1000, 1003, 1007, 1010, 1014, 1017, 1020, 1024, 1027, 1031,
                1035, 1038, 1042, 1045, 1049, 1053, 1056, 1060, 1064, 1068, 1072, 1075, 1079, 1083, 1087,
                1091, 1095, 1099, 1103, 1107, 1111, 1115, 1120, 1124, 1128, 1132, 1137, 1141, 1145, 1150,
                1154, 1159, 1163, 1168, 1172, 1177, 1181, 1186, 1191, 1196, 1200, 1205, 1210, 1215, 1220,
                1225, 1230, 1235, 1240, 1245, 1250, 1256, 1261, 1266, 1272, 1277, 1282, 1288, 1294, 1299,
                1305, 1310, 1316, 1322, 1328, 1334, 1340, 1346, 1352, 1358, 1364, 1370, 1377, 1383, 1389,
                1396, 1402, 1409, 1416, 1422, 1429, 1436, 1443, 1450, 1457, 1464, 1471, 1479, 1486, 1493,
                1501, 1508, 1516, 1524, 1531, 1539, 1547, 1555, 1563, 1572, 1580, 1588, 1597, 1605, 1614,
                1623, 1631, 1640, 1649, 1658, 1668, 1677, 1686, 1696, 1706, 1715, 1725, 1735, 1745, 1756,
                1766, 1776, 1787, 1798, 1809, 1820, 1831, 1842, 1853, 1865, 1876, 1888, 1900, 1912, 1925,
                1937, 1950, 1962, 1975, 1988, 2002, 2015, 2029, 2043, 2057, 2071, 2085, 2100, 2115, 2130,
                2145, 2160, 2176, 2192, 2208, 2224, 2241, 2258, 2275, 2292, 2310, 2328, 2346, 2365, 2384,
                2403, 2422, 2442, 2462, 2482, 2503, 2524, 2545, 2567, 2589, 2612, 2635, 2658, 2682, 2706,
                2731, 2756, 2782, 2808, 2834, 2861, 2889, 2917, 2945, 2975, 3005, 3035, 3066, 3098, 3130,
                3163, 3197, 3231, 3266, 3302, 3339, 3377, 3415, 3454, 3495, 3536, 3578, 3621, 3665, 3711,
                3757, 3805, 3854, 3904, 3956, 4008, 4063, 4118, 4176, 4235, 4295, 4358, 4422, 4488, 4556,
                4627, 4699, 4774, 4851, 4931, 5013, 5099, 5187, 5278, 5373, 5471, 5572, 5678, 5787, 5901,
                6020, 6143, 6271, 6405, 6545, 6691, 6844, 7003, 7171, 7346, 7531, 7725, 7929, 8144, 8372,
                8612, 8866, 9137, 9424, 9729
            };

            const int Count = sizeof(LUT) / sizeof(LUT[0]);

            Gfx::STextureDescriptor TextureDescriptor = {};

            TextureDescriptor.m_NumberOfPixelsU = Count;
            TextureDescriptor.m_NumberOfPixelsV = 1;
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_NumberOfMipMaps = 1;
            TextureDescriptor.m_NumberOfTextures = 1;
            TextureDescriptor.m_Binding = Gfx::CTexture::ShaderResource;
            TextureDescriptor.m_Access = Gfx::CTexture::CPUWrite;
            TextureDescriptor.m_Usage = Gfx::CTexture::GPUReadWrite;
            TextureDescriptor.m_Semantic = Gfx::CTexture::UndefinedSemantic;
            TextureDescriptor.m_pFileName = nullptr;
            TextureDescriptor.m_pPixels = nullptr;
            TextureDescriptor.m_Format = Gfx::CTexture::R16_UINT;
            m_ShiftLUTPtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

            Base::AABB2UInt TargetRect;
            TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(Count, 1));
            Gfx::TextureManager::CopyToTexture2D(m_ShiftLUTPtr, TargetRect, Count, const_cast<uint16_t*>(LUT));
        }

        // -----------------------------------------------------------------------------

        glm::vec3 KelvinToRGB(float _Kelvin)
        {
            _Kelvin = _Kelvin / 100.f;
            glm::vec3 RGB;

            if (_Kelvin <= 66)
            {
                RGB.r = 255;

                RGB.g = _Kelvin;
                RGB.g = 99.4708025861f * std::log(RGB.g) - 161.1195681661f;

                if (_Kelvin <= 19)
                {
                    RGB.b = 0;
                }
                else
                {
                    RGB.b = _Kelvin - 10;
                    RGB.b = 138.5177312231f * std::log(RGB.b) - 305.0447927307f;
                }
            }
            else
            {
                RGB.r = _Kelvin - 60;
                RGB.r = 329.698727446f * std::pow(RGB.r, -0.1332047592f);

                RGB.g = _Kelvin - 60;
                RGB.g = 288.1221695283f * std::pow(RGB.g, -0.0755148492f);

                RGB.b = 255;
            }

            return glm::clamp(RGB, 0.0f, 255.0f) / 255.0f;
        }

        // -----------------------------------------------------------------------------

        int DivUp(int TotalShaderCount, int WorkGroupSize)
        {
            return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
        }
    };
} // namespace Scpt