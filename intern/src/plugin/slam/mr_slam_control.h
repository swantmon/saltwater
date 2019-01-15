
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
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

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

    private:

        enum EDATASOURCE
        {
            NETWORK,
            KINECT
        };

        EDATASOURCE m_DataSource;
        
        Gfx::CTexturePtr m_DepthTexture;
        Gfx::CTexturePtr m_RGBTexture;
        std::vector<uint16_t> m_DepthBuffer;
        std::vector<char> m_ColorBuffer;
        glm::mat4 m_PoseMatrix;

        glm::ivec2 m_DepthSize;
        glm::ivec2 m_ColorSize;

        bool m_UseTrackingCamera = false;

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
            SECONDPRESS,
            SECONDRELEASE
        };

        glm::vec3 m_SelectionBoxAnchor0;
        glm::vec3 m_SelectionBoxAnchor1;
        float m_SelectionBoxHeight;

        ESelection m_SelectionState;

        bool m_MousePressed;

        // -----------------------------------------------------------------------------
        // Reconstructor
        // -----------------------------------------------------------------------------

        bool m_CaptureColor;

        std::unique_ptr<MR::CScalableSLAMReconstructor> m_pReconstructor;

        // -----------------------------------------------------------------------------
        // Stuff for network data source
        // -----------------------------------------------------------------------------
        std::shared_ptr<Net::CMessageDelegate> m_NetworkDelegate;

        Gfx::CShaderPtr m_YUVtoRGBCSPtr;
        Gfx::CTexturePtr m_YTexture;
        Gfx::CTexturePtr m_UVTexture;

        Gfx::CTexturePtr m_ShiftTexture;
        Gfx::CShaderPtr m_ShiftDepthCSPtr;
        Gfx::CTexturePtr m_ShiftLUTPtr;

        Gfx::CBufferPtr m_RGBConversionBuffer;

        struct SRGBConversion
        {
            glm::vec4 m_Ambient;
            glm::vec4 m_Temperature;
        };

        // -----------------------------------------------------------------------------
        // Stuff for Kinect data source
        // -----------------------------------------------------------------------------
        typedef bool(*GetDepthBufferFunc)(uint16_t*);
        typedef bool(*GetColorBufferFunc)(char*);
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
        std::string m_RecordFileName;

        std::fstream m_RecordFile;
        std::unique_ptr<Base::CRecordWriter> m_pRecordWriter;
        std::unique_ptr<Base::CRecordReader> m_pRecordReader;

    public:

        void Start()
        {
            m_SelectionBoxAnchor0 = glm::vec3(0.0f);
            m_SelectionBoxAnchor1 = glm::vec3(0.0f);
            m_SelectionBoxHeight = 0.0f;
            m_SelectionState = ESelection::NOSELECTION;
            m_MousePressed = false;

            m_pReconstructor.reset(new MR::CScalableSLAMReconstructor);
            Gfx::ReconstructionRenderer::SetReconstructor(*m_pReconstructor);

            // -----------------------------------------------------------------------------
            // Determine where we get our data from
            // -----------------------------------------------------------------------------
            std::string DataSource = Core::CProgramParameters::GetInstance().Get("mr:slam:data_source", "network");

            if (DataSource == "network")
            {
                // -----------------------------------------------------------------------------
                // Create network connection
                // -----------------------------------------------------------------------------
                m_NetworkDelegate = std::shared_ptr<Net::CMessageDelegate>(new Net::CMessageDelegate(std::bind(&CSLAMControl::OnNewMessage, this, std::placeholders::_1, std::placeholders::_2)));

                Net::CNetworkManager::GetInstance().RegisterMessageHandler(0, m_NetworkDelegate);

                m_DataSource = NETWORK;

                CreateShiftLUTTexture();

                Gfx::SBufferDescriptor ConstantBufferDesc = {};

                ConstantBufferDesc.m_Usage = Gfx::CBuffer::EUsage::GPURead;
                ConstantBufferDesc.m_Binding = Gfx::CBuffer::ConstantBuffer;
                ConstantBufferDesc.m_Access = Gfx::CBuffer::CPUWrite;
                ConstantBufferDesc.m_NumberOfBytes = sizeof(SRGBConversion);

                m_RGBConversionBuffer = Gfx::BufferManager::CreateBuffer(ConstantBufferDesc);
            }
            else if (DataSource == "kinect")
            {
                // -----------------------------------------------------------------------------
                // Load Kinect plugin
                // -----------------------------------------------------------------------------
                
                Engine::LoadPlugin("plugin_kinect");

                if (!Core::PluginManager::HasPlugin("Kinect"))
                {
                    throw Base::CException(__FILE__, __LINE__, "Kinect plugin was not loaded");
                }

                m_DataSource = KINECT;

                typedef void(*GetIntrinsicsFunc)(glm::vec2&, glm::vec2&, glm::ivec2&);

                GetIntrinsicsFunc GetIntrinsics = (GetIntrinsicsFunc)(Core::PluginManager::GetPluginFunction("Kinect", "GetIntrinsics"));

                glm::vec2 FocalLength;
                glm::vec2 FocalPoint;
                
                GetIntrinsics(FocalLength, FocalPoint, m_DepthSize);
                m_ColorSize = m_DepthSize;

                m_pReconstructor->SetImageSizes(glm::vec2(m_DepthSize), glm::vec2(m_ColorSize));
                m_pReconstructor->SetIntrinsics(glm::vec2(FocalLength), glm::vec2(FocalPoint));
                
                m_pReconstructor->Start();

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
                TextureDescriptor.m_pPixels = 0;
                TextureDescriptor.m_Format = Gfx::CTexture::R16_UINT;

                m_DepthTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);
                
                TextureDescriptor.m_Format = Gfx::CTexture::R8G8B8A8_UBYTE;

                m_RGBTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

                GetDepthBuffer = (GetDepthBufferFunc)(Core::PluginManager::GetPluginFunction("Kinect", "GetDepthBuffer"));
                GetColorBuffer = (GetColorBufferFunc)(Core::PluginManager::GetPluginFunction("Kinect", "GetColorBuffer"));

                MR::SReconstructionSettings Settings;
                m_pReconstructor->GetReconstructionSettings(&Settings);
                m_CaptureColor = Settings.m_CaptureColor;
            }
            else
            {
                throw Base::CException(__FILE__, __LINE__, "Unknown data source for SLAM plugin");
            }

            std::string RecordParam = Core::CProgramParameters::GetInstance().Get("mr:slam:recording:mode", "none");
            m_RecordFileName = Core::CProgramParameters::GetInstance().Get("mr:slam:recording:file", "");
            double SpeedOfPlayback = Core::CProgramParameters::GetInstance().Get("mr:slam:recording:speed", 1.0);

            if (RecordParam == "play")
            {
                m_RecordMode = PLAY;
                m_RecordFile.open(m_RecordFileName, std::fstream::in | std::fstream::binary);
                m_pRecordReader.reset(new Base::CRecordReader(m_RecordFile, 1));

                m_pRecordReader->SetSpeed(SpeedOfPlayback);
            }
            else if (RecordParam == "record")
            {
                m_RecordMode = RECORD;
                m_RecordFile.open(m_RecordFileName, std::fstream::out | std::fstream::binary);
                m_pRecordWriter.reset(new Base::CRecordWriter(m_RecordFile, 1));
            }
            else
            {
                m_RecordMode = NONE;
            }
        }

        // -----------------------------------------------------------------------------

        void Exit()
        {
            m_RecordFile.close();

            m_DepthBuffer.clear();
            m_ColorBuffer.clear();

            m_pReconstructor->Exit();

            m_RGBConversionBuffer = nullptr;

            m_DepthTexture = nullptr;
            m_RGBTexture = nullptr;

            m_NetworkDelegate = nullptr;
            
            m_YUVtoRGBCSPtr = nullptr;
            m_YTexture = nullptr;
            m_UVTexture = nullptr;

            m_ShiftTexture = nullptr;
            m_ShiftDepthCSPtr = nullptr;
            m_ShiftLUTPtr = nullptr;

            m_pReconstructor.release();
        }

        // -----------------------------------------------------------------------------

        void Update()
        {
            if (m_RecordMode == PLAY)
            {
                m_pRecordReader->Update();

                if (m_pRecordReader->IsEnd())
                {
                    m_RecordMode = NONE;
                    m_UseTrackingCamera = false;
                }

                while (!m_pRecordReader->IsEnd())
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

            if (m_DataSource == KINECT)
            {
                if (m_CaptureColor && GetDepthBuffer(m_DepthBuffer.data()) && GetColorBuffer(m_ColorBuffer.data()))
                {
                    Base::AABB2UInt TargetRect;
                    TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_DepthSize.x, m_DepthSize.y));

                    Gfx::TextureManager::CopyToTexture2D(m_DepthTexture, TargetRect, m_DepthSize.x, m_DepthBuffer.data());
                    Gfx::TextureManager::CopyToTexture2D(m_RGBTexture, TargetRect, m_DepthSize.x, m_ColorBuffer.data());

                    m_pReconstructor->OnNewFrame(m_DepthTexture, m_RGBTexture, nullptr);
                }
                else if (GetDepthBuffer(m_DepthBuffer.data()))
                {
                    Base::AABB2UInt TargetRect;
                    TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_DepthSize.x, m_DepthSize.y));
                    Gfx::TextureManager::CopyToTexture2D(m_DepthTexture, TargetRect, m_DepthSize.x, m_DepthBuffer.data());
                    m_pReconstructor->OnNewFrame(m_DepthTexture, nullptr, nullptr);
                }
            }

            if (m_SelectionState == ESelection::FIRSTRELEASE)
            {
                Gfx::CTexturePtr PlaneTexture = m_pReconstructor->CreatePlaneTexture(m_SelectionBoxAnchor0, m_SelectionBoxAnchor1);
            }

            if (m_UseTrackingCamera)
            {
                Cam::CControl& rControl = static_cast<Cam::CEditorControl&>(Cam::ControlManager::GetActiveControl());

                glm::mat4 PoseMatrix = m_PoseMatrix;
                PoseMatrix = glm::eulerAngleX(glm::radians(90.0f)) * PoseMatrix;

                glm::vec3 Eye = PoseMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
                glm::vec3 At = PoseMatrix * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
                glm::vec3 Up = PoseMatrix * glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);

                glm::mat4 View = glm::lookAtRH(Eye, At, Up);

                rControl.SetPosition(glm::vec4(Eye, 1.0f));
                rControl.SetRotation(glm::mat4(glm::inverse(glm::mat3(View))));
                rControl.Update();
            }
        }

        // -----------------------------------------------------------------------------

        glm::vec3 ComputeAnchor1(const Base::CInputEvent& _rEvent)
        {
            glm::ivec2 RawCursor = _rEvent.GetLocalCursorPosition();

            const glm::ivec2 WindowSize = Gfx::Main::GetActiveWindowSize();
            const glm::vec3 CameraPosition = Gfx::ViewManager::GetMainCamera()->GetView()->GetPosition();
            const glm::mat4 ViewProjectionMatrix = Gfx::ViewManager::GetMainCamera()->GetViewProjectionMatrix();

            glm::ivec2 Cursor;
            Cursor.x = RawCursor.y;
            Cursor.y = WindowSize.y - RawCursor.x;

            glm::vec4 CSCursorPosition = glm::vec4(glm::vec2(Cursor) / glm::vec2(WindowSize) * 2.0f - 1.0f, 0.0f, 1.0f);
            glm::mat4 InvViewProjectionMatrix = glm::inverse(ViewProjectionMatrix);

            glm::vec4 WSCursorPosition = InvViewProjectionMatrix * CSCursorPosition;
            WSCursorPosition /= WSCursorPosition.w;

            glm::vec3 RayDirection = glm::vec3(WSCursorPosition) - CameraPosition;

            float d = (m_SelectionBoxAnchor0.z - CameraPosition.z) / RayDirection.z;

            return d * RayDirection + CameraPosition;
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
                m_MousePressed = true;

                if (m_SelectionState == ESelection::NOSELECTION)
                {
                    m_SelectionBoxAnchor0 = Gfx::ReconstructionRenderer::Pick(_rEvent.GetLocalCursorPosition());
                    m_SelectionState = ESelection::FIRSTPRESS;
                }
            }
            else if (_rEvent.GetAction() == Base::CInputEvent::MouseLeftReleased)
            {
                m_MousePressed = false;

                m_SelectionState = m_SelectionState == ESelection::FIRSTPRESS ? ESelection::FIRSTRELEASE : ESelection::NOSELECTION;

                //Gfx::CTexturePtr PlaneTexture = m_pReconstructor->CreatePlaneTexture(m_SelectionBoxAnchor0, m_SelectionBoxAnchor1);
            }
            else if (_rEvent.GetAction() == Base::CInputEvent::MouseMove)
            {
                if (m_MousePressed && m_SelectionState == ESelection::FIRSTPRESS)
                {
                    m_SelectionBoxAnchor1 = ComputeAnchor1(_rEvent);
                }
            }
            Gfx::ReconstructionRenderer::SetSelectionBox(m_SelectionBoxAnchor0, m_SelectionBoxAnchor1, m_SelectionBoxHeight, static_cast<int>(m_SelectionState));
        }

    private:

        void HandleMessage(const Net::CMessage& _rMessage)
        {
            std::vector<char> Decompressed(_rMessage.m_DecompressedSize);

            if (_rMessage.m_CompressedSize != _rMessage.m_DecompressedSize)
            {
                Base::Decompress(_rMessage.m_Payload, Decompressed);
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
                    m_pReconstructor->ResetReconstruction();
                }
                else if (MessageID == 1)
                {
                    ENGINE_CONSOLE_INFO("Initializing reconstructor");

                    glm::vec2 FocalLength = *reinterpret_cast<glm::vec2*>(Decompressed.data() + sizeof(int32_t) * 2);
                    glm::vec2 FocalPoint = *reinterpret_cast<glm::vec2*>(Decompressed.data() + sizeof(int32_t) * 2 + sizeof(glm::vec2));
                    m_DepthSize = *reinterpret_cast<glm::ivec2*>(Decompressed.data() + sizeof(int32_t) * 2 + sizeof(glm::vec2) * 2);
                    m_ColorSize = *reinterpret_cast<glm::ivec2*>(Decompressed.data() + sizeof(int32_t) * 2 + sizeof(glm::vec2) * 2 + sizeof(glm::ivec2));

                    MR::SReconstructionSettings Settings;
                    m_pReconstructor->GetReconstructionSettings(&Settings);

                    m_CaptureColor = Settings.m_CaptureColor;

                    if (m_CaptureColor)
                    {
                        FocalPoint.x = (FocalPoint.x / m_DepthSize.x) * m_ColorSize.x;
                        FocalPoint.y = (FocalPoint.y / m_DepthSize.y) * m_ColorSize.y;

                        m_pReconstructor->SetImageSizes(glm::vec2(m_ColorSize), glm::vec2(m_ColorSize));
                        m_pReconstructor->SetIntrinsics(glm::vec2(FocalLength), glm::vec2(FocalPoint));
                    }
                    else
                    {
                        m_pReconstructor->SetImageSizes(glm::vec2(m_DepthSize), glm::vec2(m_DepthSize));
                        m_pReconstructor->SetIntrinsics(glm::vec2(FocalLength), glm::vec2(FocalPoint));
                    }

                    m_pReconstructor->Start();

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
                    TextureDescriptor.m_pPixels = 0;
                    TextureDescriptor.m_Format = Gfx::CTexture::R16_UINT;
                    m_ShiftTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

                    TextureDescriptor.m_NumberOfPixelsU = m_CaptureColor ? m_ColorSize.x : m_DepthSize.x;
                    TextureDescriptor.m_NumberOfPixelsV = m_CaptureColor ? m_ColorSize.y : m_DepthSize.y;
                    m_DepthTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

                    std::stringstream DefineStream;
                    DefineStream
                        << "#define TILE_SIZE_2D " << m_TileSize2D << " \n"
                        << "#define DEPTH_WIDTH "  << m_DepthSize.x << " \n"
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
                        m_RGBTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

                        TextureDescriptor.m_Format = Gfx::CTexture::R8_UBYTE;
                        m_YTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

                        TextureDescriptor.m_NumberOfPixelsU = m_ColorSize.x / 2;
                        TextureDescriptor.m_NumberOfPixelsV = m_ColorSize.y / 2;
                        TextureDescriptor.m_Format = Gfx::CTexture::R8G8_UBYTE;
                        m_UVTexture = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

                        std::string DefineString = DefineStream.str();
                        m_YUVtoRGBCSPtr = Gfx::ShaderManager::CompileCS("slam\\cs_yuv_to_rgb.glsl", "main", DefineString.c_str());
                    }
                    std::string DefineString = DefineStream.str();
                    m_ShiftDepthCSPtr = Gfx::ShaderManager::CompileCS("slam\\cs_shift_depth.glsl", "main", DefineString.c_str());

                    m_UseTrackingCamera = true;

                    ENGINE_CONSOLE_INFO("Initialization complete");
                }
            }
            else if (MessageType == TRANSFORM)
            {
                m_PoseMatrix = *reinterpret_cast<glm::mat4*>(Decompressed.data() + sizeof(int32_t)) * glm::eulerAngleX(glm::pi<float>());
            }
            else if (MessageType == DEPTHFRAME)
            {
                //int32_t Width = *reinterpret_cast<int32_t*>(Decompressed.data() + sizeof(int32_t));
                //int32_t Height = *reinterpret_cast<int32_t*>(Decompressed.data() + 2 * sizeof(int32_t));

                const uint16_t* RawBuffer = reinterpret_cast<uint16_t*>(Decompressed.data() + 3 * sizeof(int32_t));

                Base::AABB2UInt TargetRect;
                TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_DepthSize));
                Gfx::TextureManager::CopyToTexture2D(m_ShiftTexture, TargetRect, m_DepthSize.x, const_cast<uint16_t*>(RawBuffer));

                Gfx::ContextManager::SetShaderCS(m_ShiftDepthCSPtr);
                Gfx::ContextManager::SetImageTexture(0, m_ShiftTexture);
                Gfx::ContextManager::SetImageTexture(1, m_DepthTexture);
                Gfx::ContextManager::SetImageTexture(2, m_ShiftLUTPtr);

                int WorkgroupsX = DivUp(m_CaptureColor ? m_ColorSize.x : m_DepthSize.x, m_TileSize2D);
                int WorkgroupsY = DivUp(m_CaptureColor ? m_ColorSize.y : m_DepthSize.y, m_TileSize2D);
                Gfx::ContextManager::Dispatch(WorkgroupsX, WorkgroupsY, 1);

                m_UseTrackingCamera = true;

                if (!m_CaptureColor)
                {
                    m_pReconstructor->OnNewFrame(m_DepthTexture, nullptr, &m_PoseMatrix);
                }
            }
            else if (MessageType == COLORFRAME && m_CaptureColor)
            {
                const int32_t Width = *reinterpret_cast<int32_t*>(Decompressed.data() + sizeof(int32_t));
                const int32_t Height = *reinterpret_cast<int32_t*>(Decompressed.data() + 2 * sizeof(int32_t));

                const char* YData = Decompressed.data() + 3 * sizeof(int32_t);
                const char* UVData = YData + Width * Height;

                Base::AABB2UInt TargetRect;
                TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_ColorSize.x, m_ColorSize.y));
                Gfx::TextureManager::CopyToTexture2D(m_YTexture, TargetRect, m_ColorSize.x, const_cast<char*>(YData));

                TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_ColorSize.x / 2, m_ColorSize.y / 2));
                Gfx::TextureManager::CopyToTexture2D(m_UVTexture, TargetRect, m_ColorSize.x / 2, const_cast<char*>(UVData));

                Gfx::ContextManager::SetConstantBuffer(0, m_RGBConversionBuffer);

                Gfx::ContextManager::SetShaderCS(m_YUVtoRGBCSPtr);
                Gfx::ContextManager::SetImageTexture(0, m_YTexture);
                Gfx::ContextManager::SetImageTexture(1, m_UVTexture);
                Gfx::ContextManager::SetImageTexture(2, m_RGBTexture);

                Gfx::ContextManager::Dispatch(DivUp(m_ColorSize.x, m_TileSize2D), DivUp(m_ColorSize.y, m_TileSize2D), 1);

                m_pReconstructor->OnNewFrame(m_DepthTexture, m_RGBTexture, &m_PoseMatrix);
            }
            else if (MessageType == LIGHTESTIMATE)
            {
                const float AmbientIntensity = *reinterpret_cast<float*>(Decompressed.data() + sizeof(int32_t));
                const float LightTemperature = *reinterpret_cast<float*>(Decompressed.data() + sizeof(int32_t) + sizeof(float));

                glm::vec3 LightColor = KelvinToRGB(LightTemperature);

                SRGBConversion Data;
                Data.m_Ambient = glm::vec4(AmbientIntensity);
                Data.m_Temperature = glm::vec4(LightColor, LightTemperature);
                Gfx::BufferManager::UploadBufferData(m_RGBConversionBuffer, &Data);
            }
            else if (MessageType == PLANE)
            {
                int PlaneID = *reinterpret_cast<int*>(Decompressed.data() + sizeof(int32_t));
                int PlaneAction = *reinterpret_cast<int*>(Decompressed.data() + 2 * sizeof(int32_t));

                glm::mat4 PlaneTransform = *reinterpret_cast<glm::mat4*>(Decompressed.data() + 3 * sizeof(int32_t));
                glm::vec4 PlaneExtent = *reinterpret_cast<glm::vec4*>(Decompressed.data() + 3 * sizeof(int32_t) + sizeof(glm::mat4));

                PlaneTransform = glm::eulerAngleX(glm::half_pi<float>()) * PlaneTransform;

                switch (PlaneAction)
                {
                case 0:
                    m_pReconstructor->AddPlane(PlaneTransform, PlaneExtent, PlaneID);
                    break;
                case 1:
                    m_pReconstructor->UpdatePlane(PlaneTransform, PlaneExtent, PlaneID);
                    break;
                case 2:
                    m_pReconstructor->RemovePlane(PlaneID);
                    break;
                }
            }
        }

        void OnNewMessage(const Net::CMessage& _rMessage, int _Port)
        {
            BASE_UNUSED(_Port);
            
            if (_rMessage.m_MessageType == 0)
            {
                if (m_RecordMode == RECORD)
                {
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
            TextureDescriptor.m_pPixels = 0;
            TextureDescriptor.m_Format = Gfx::CTexture::R16_UINT;
            m_ShiftLUTPtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

            Base::AABB2UInt TargetRect;
            TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(Count, 1));
            Gfx::TextureManager::CopyToTexture2D(m_ShiftLUTPtr, TargetRect, Count, const_cast<uint16_t*>(LUT));
        }

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

        int DivUp(int TotalShaderCount, int WorkGroupSize)
        {
            return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
        }
    };
} // namespace Scpt