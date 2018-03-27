
#include "mr/mr_precompiled.h"

#include "base/base_coordinate_system.h"
#include "base/base_input_event.h"
#include "base/base_pool.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_jni_interface.h"

#include "data/data_ar_controller_component.h"
#include "data/data_camera_component.h"
#include "data/data_component_facet.h"
#include "data/data_component_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_camera_interface.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

#include "gui/gui_event_handler.h"

#include "mr/mr_control_manager.h"

#include <array>
#include <vector>

#include "arcore_c_api.h"

#if PLATFORM_ANDROID
#include "GLES3/gl32.h"

#ifndef GL_OES_EGL_image_external
#define GL_OES_EGL_image_external 1
#define GL_TEXTURE_EXTERNAL_OES           0x8D65
#define GL_TEXTURE_BINDING_EXTERNAL_OES   0x8D67
#define GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES 0x8D68
#endif /* GL_OES_EGL_image_external */

#endif

using namespace MR;
using namespace MR::ControlManager;

namespace
{
    glm::vec3 GetPlaneColor(int _Index)
    {
        static constexpr int s_NumberOfPlaneColors = 12;

        constexpr unsigned int s_PlaneColorRGB[s_NumberOfPlaneColors] = {
                0xFFFFFFFF, 0xF44336FF, 0xE91E63FF, 0x9C27B0FF, 0x673AB7FF, 0x3F51B5FF,
                0x2196F3FF, 0x03A9F4FF, 0x00BCD4FF, 0x009688FF, 0x4CAF50FF, 0x8BC34AFF};

        const int RGB = s_PlaneColorRGB[_Index % s_NumberOfPlaneColors];

        return glm::vec3(((RGB >> 24) & 0xff) / 255.0f, ((RGB >> 16) & 0xff) / 255.0f, ((RGB >> 8) & 0xff) / 255.0f);
    }

    const float c_Uvs[] = {
            0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    constexpr char c_VertexShaderWebcam[] = R"(
        layout(location = 0) in vec2 in_UV;

        layout(location = 0) out vec2 out_UV;

        void main()
        {
            vec2 Vertices[4];

            Vertices[0] = vec2(-1.0f, -1.0f);
            Vertices[1] = vec2( 1.0f, -1.0f);
            Vertices[2] = vec2(-1.0f,  1.0f);
            Vertices[3] = vec2( 1.0f,  1.0f);

            out_UV = in_UV;

            gl_Position = vec4(Vertices[gl_VertexID], 0.0f, 1.0f);
        }
    )";

    constexpr char c_FragmentShaderWebcam[] = R"(
        #extension GL_OES_EGL_image_external_essl3 : require

        precision mediump float;

        layout(location = 0) uniform samplerExternalOES in_ExtOESTexture;

        layout(location = 0) in vec2 in_UV;

        layout(location = 0) out vec4 out_Output;

        void main()
        {
            out_Output = texture(in_ExtOESTexture, in_UV);
        }
    )";

    constexpr char c_VertexShaderPlane[] = R"(
        precision highp float;

        layout(location = 0) uniform mat4 m_MVP;

        layout(location = 0) in vec3 in_Vertex;

        layout(location = 0) out float out_Alpha;

        void main()
        {
          gl_Position = m_MVP * vec4(in_Vertex.x, 0.0f, in_Vertex.y, 1.0f);

          out_Alpha = in_Vertex.z;
        }
    )";

    constexpr char c_FragmentShaderPlane[] = R"(
        precision highp float;

        layout(location = 1) uniform vec4 m_Color;

        layout(location = 0) in float in_Alpha;

        layout(location = 0) out vec4 out_Output;

        void main()
        {
            vec2 PixelPos = gl_FragCoord.xy + 0.5f;

            float PatternMask = mod((PixelPos.x / 2.0f + PixelPos.y / 2.0f), 2.0f);

            out_Output = mix(m_Color, vec4(0.0f, 0.0f ,0.0f, 0.0f), PatternMask) * in_Alpha;
        }
    )";

    constexpr char c_VertexShaderPoint[] = R"(
        layout(location = 0) uniform mat4 m_MVP;

        layout(location = 0) in vec4 in_Vertex;

        void main()
        {
            gl_PointSize = 10.0;

            gl_Position = m_MVP * vec4(in_Vertex.xyz, 1.0);
        }
    )";

    constexpr char c_FragmentShaderPoint[] = R"(
        precision lowp float;

        layout(location = 0) out vec4 out_Output;

        void main()
        {
            out_Output = vec4(0.1215f, 0.7372f, 0.8235f, 1.0f);
        }
    )";

    unsigned int g_TextureID;
    unsigned int g_AttributeUVs;
    unsigned int g_AttributePlaneVertices;
    unsigned int g_AttributePointVertices;
    unsigned int g_PlaneIndices;

    static constexpr int s_MaxNumberOfVerticesPerPlane = 1024;
    static constexpr int s_MaxNumberOfVerticesPerPoint = 512;

    static constexpr int s_NumberOfVertices = 4;
    bool g_IsUVsInitialized = false;
    float g_TransformedUVs[s_NumberOfVertices * 2];
} // namespace

namespace
{
    class CMRControlManager : private Base::CUncopyable
    {
    BASE_SINGLETON_FUNC(CMRControlManager);

    public:

        CMRControlManager();
        ~CMRControlManager();

    public:

        void OnStart();
        void OnExit();
        void Update();

        void OnPause();
        void OnResume();

        void OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height);

        void OnDraw();

        const CCamera& GetCamera();

        const CLightEstimation& GetLightEstimation();

        const CMarker* AcquireNewMarker(float _X, float _Y);
        void ReleaseMarker(const CMarker* _pMarker);

        Gfx::CTexturePtr GetBackgroundTexture();

    private:

        static std::string s_Permissions[];

    private:

        class CInternCamera : public CCamera
        {
        private:

            friend class CMRControlManager;
        };

        class CInternLightEstimation : public CLightEstimation
        {
        private:

            friend class CMRControlManager;
        };

        class CInternMarker : public CMarker
        {
        public:

            ArAnchor* m_pAnchor;

        private:

            friend class CMRControlManager;
        };

    private:

        typedef std::vector<CInternMarker> CTrackedObjects;

    private:

        bool m_InstallRequested;

        ArSession* m_pARSession;
        ArFrame* m_pARFrame;
        CTrackedObjects m_TrackedObjects;

        CInternCamera m_Camera;

        CInternLightEstimation m_LightEstimation;

        glm::mat3 m_ARCToEngineMatrix;

        Gfx::CTexturePtr m_BackgroundTexturePtr;

        Gfx::CTargetSetPtr m_BackgroundTargetSetPtr;

        Gfx::CShaderPtr m_WebcamVSPtr;

        Gfx::CShaderPtr m_WebcamPSPtr;

        Gfx::CShaderPtr m_PlaneVS;

        Gfx::CShaderPtr m_PlanePS;

        Gfx::CShaderPtr m_PointVS;

        Gfx::CShaderPtr m_PointPS;

        Gfx::CBufferPtr m_WebcamUVBufferPtr;

        Gfx::CBufferPtr m_PlaneVerticesBufferPtr;

        Gfx::CBufferPtr m_PlaneIndicesBufferPtr;

        Gfx::CBufferPtr m_PointVerticesBufferPtr;

        Gfx::CBufferPtr m_MatrixBufferPtr;

        Gfx::CBufferPtr m_ColorBufferPtr;

    private:

        void OnDirtyEntity(Dt::CEntity* _pEntity);
        void OnDirtyComponent(Dt::IComponent* _pComponent);
    };
} // namespace

namespace
{
    std::string CMRControlManager::s_Permissions[] = { "android.permission.CAMERA" };
}

namespace
{
    CMRControlManager::CMRControlManager()
        : m_InstallRequested(false)
        , m_pARSession       (0)
        , m_pARFrame         (0)
        , m_TrackedObjects   ( )
        , m_ARCToEngineMatrix(1.0f)
    {
        m_ARCToEngineMatrix = Base::CCoordinateSystem::GetBaseMatrix(glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,0,-1));
    }

    // -----------------------------------------------------------------------------

    CMRControlManager::~CMRControlManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Shader for background and debug
        // -----------------------------------------------------------------------------
        m_WebcamVSPtr = Gfx::ShaderManager::CompileVS(c_VertexShaderWebcam, "main", nullptr, nullptr, 0, false, false, true);

        m_WebcamPSPtr = Gfx::ShaderManager::CompilePS(c_FragmentShaderWebcam, "main", nullptr, nullptr, 0, false, false, true);

        m_PlaneVS = Gfx::ShaderManager::CompileVS(c_VertexShaderPlane, "main", nullptr, nullptr, 0, false, false, true);

        m_PlanePS = Gfx::ShaderManager::CompilePS(c_FragmentShaderPlane, "main", nullptr, nullptr, 0, false, false, true);

        m_PointVS = Gfx::ShaderManager::CompileVS(c_VertexShaderPoint, "main", nullptr, nullptr, 0, false, false, true);

        m_PointPS = Gfx::ShaderManager::CompilePS(c_FragmentShaderPoint, "main", nullptr, nullptr, 0, false, false, true);

        const Gfx::SInputElementDescriptor InputLayout[] =
        {
            { "UV", 0, Gfx::CInputLayout::Float2Format, 0, 0, 8, Gfx::CInputLayout::PerVertex, 0, },
        };

        Gfx::ShaderManager::CreateInputLayout(InputLayout, 1, m_WebcamVSPtr);

        // -----------------------------------------------------------------------------
        // Background texture
        // -----------------------------------------------------------------------------
        Gfx::STextureDescriptor TextureDesc;

        TextureDesc.m_NumberOfPixelsU  = Gfx::Main::GetActiveWindowSize()[0];
        TextureDesc.m_NumberOfPixelsV  = Gfx::Main::GetActiveWindowSize()[1];
        TextureDesc.m_NumberOfPixelsW  = 1;
        TextureDesc.m_NumberOfMipMaps  = 1;
        TextureDesc.m_NumberOfTextures = 1;
        TextureDesc.m_Binding          = Gfx::CTexture::RenderTarget | Gfx::CTexture::ShaderResource;
        TextureDesc.m_Access           = Gfx::CTexture::CPUWrite;
        TextureDesc.m_Format           = Gfx::CTexture::Unknown;
        TextureDesc.m_Usage            = Gfx::CTexture::GPURead;
        TextureDesc.m_Semantic         = Gfx::CTexture::Diffuse;
        TextureDesc.m_pFileName        = 0;
        TextureDesc.m_pPixels          = 0;
        TextureDesc.m_Format           = Gfx::CTexture::R8G8B8A8_UBYTE;

        m_BackgroundTexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc);

        // -----------------------------------------------------------------------------
        // Labels
        // -----------------------------------------------------------------------------
        Gfx::TextureManager::SetTextureLabel(m_BackgroundTexturePtr, "Webcam texture");

        // -----------------------------------------------------------------------------
        // Target sets & view ports
        // -----------------------------------------------------------------------------
        m_BackgroundTargetSetPtr = Gfx::TargetSetManager::CreateTargetSet(m_BackgroundTexturePtr);

        // -----------------------------------------------------------------------------
        // Buffer
        // -----------------------------------------------------------------------------
        Gfx::SBufferDescriptor ConstanteBufferDesc;
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::VertexBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = 8 * sizeof(float);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_WebcamUVBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::VertexBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = s_MaxNumberOfVerticesPerPlane * sizeof(glm::vec3);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_PlaneVerticesBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::IndexBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = s_MaxNumberOfVerticesPerPlane * 3 * sizeof(short);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_PlaneIndicesBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::VertexBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = s_MaxNumberOfVerticesPerPoint * sizeof(glm::vec3);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_PointVerticesBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::VertexBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(glm::mat4);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_MatrixBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::VertexBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(glm::vec4);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_ColorBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

#ifdef PLATFORM_ANDROID
        glGenTextures(1, &g_TextureID);

        glBindTexture(GL_TEXTURE_EXTERNAL_OES, g_TextureID);

        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif

        // -----------------------------------------------------------------------------
        // Handler
        // -----------------------------------------------------------------------------
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CMRControlManager::OnDirtyEntity));

        Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(DATA_DIRTY_COMPONENT_METHOD(&CMRControlManager::OnDirtyComponent));
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnExit()
    {
        m_TrackedObjects.clear();

#ifdef PLATFORM_ANDROID
        // -----------------------------------------------------------------------------
        // AR session and frame
        // -----------------------------------------------------------------------------
        ArSession_destroy(m_pARSession);

        ArFrame_destroy(m_pARFrame);

        // -----------------------------------------------------------------------------
        // OpenGLES
        // -----------------------------------------------------------------------------
        glDeleteTextures(1, &g_TextureID);
#endif
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::Update()
    {
#ifdef PLATFORM_ANDROID
        if (m_pARSession == nullptr) return;

        ArStatus Result;

        ArSession_setCameraTextureName(m_pARSession, g_TextureID);

        Result = ArSession_update(m_pARSession, m_pARFrame);

        if (Result != AR_SUCCESS) return;

        // -----------------------------------------------------------------------------
        // Update camera
        // -----------------------------------------------------------------------------
        m_Camera.m_Near = Base::CProgramParameters::GetInstance().Get<float>("mr:ar:camera:near", 0.1f);
        m_Camera.m_Far  = Base::CProgramParameters::GetInstance().Get<float>("mr:ar:camera:far", 100.0f);

        ArCamera* pARCamera;

        ArFrame_acquireCamera(m_pARSession, m_pARFrame, &pARCamera);

        ArCamera_getViewMatrix(m_pARSession, pARCamera, glm::value_ptr(m_Camera.m_ViewMatrix));

        ArCamera_getProjectionMatrix(m_pARSession, pARCamera, m_Camera.m_Near, m_Camera.m_Far, glm::value_ptr(m_Camera.m_ProjectionMatrix));

        ArTrackingState CameraTrackingState;

        ArCamera_getTrackingState(m_pARSession, pARCamera, &CameraTrackingState);

        ArCamera_release(pARCamera);

        switch(CameraTrackingState)
        {
            case AR_TRACKING_STATE_PAUSED:
                m_Camera.m_TrackingState = CCamera::Paused;
                break;
            case AR_TRACKING_STATE_STOPPED:
                m_Camera.m_TrackingState = CCamera::Stopped;
                break;
            case AR_TRACKING_STATE_TRACKING:
                m_Camera.m_TrackingState = CCamera::Tracking;
                break;
        }

        if (CameraTrackingState != AR_TRACKING_STATE_TRACKING) return;

        // -----------------------------------------------------------------------------
        // Light estimation
        // Intensity value ranges from 0.0f to 1.0f.
        // -----------------------------------------------------------------------------
        ArLightEstimate* ARLightEstimate;
        ArLightEstimateState ARLightEstimateState;

        ArLightEstimate_create(m_pARSession, &ARLightEstimate);

        ArFrame_getLightEstimate(m_pARSession, m_pARFrame, ARLightEstimate);

        ArLightEstimate_getState(m_pARSession, ARLightEstimate, &ARLightEstimateState);

        m_LightEstimation.m_EstimationState = CLightEstimation::NotValid;

        if (ARLightEstimateState == AR_LIGHT_ESTIMATE_STATE_VALID)
        {
            ArLightEstimate_getPixelIntensity(m_pARSession, ARLightEstimate, &m_LightEstimation.m_Intensity);

            m_LightEstimation.m_EstimationState = CLightEstimation::Valid;
        }

        ArLightEstimate_destroy(ARLightEstimate);

        ARLightEstimate = nullptr;

        // -----------------------------------------------------------------------------
        // Use tracked objects matrices
        // -----------------------------------------------------------------------------
        for (auto& rObject : m_TrackedObjects)
        {
            ArTrackingState TrackingState = AR_TRACKING_STATE_STOPPED;

            ArAnchor_getTrackingState(m_pARSession, rObject.m_pAnchor, &TrackingState);

            switch(TrackingState)
            {
                case AR_TRACKING_STATE_PAUSED:
                    rObject.m_TrackingState = CMarker::Paused;
                    break;
                case AR_TRACKING_STATE_STOPPED:
                    rObject.m_TrackingState = CMarker::Stopped;
                    break;
                case AR_TRACKING_STATE_TRACKING:
                    rObject.m_TrackingState = CMarker::Tracking;
                    break;
            }

            if (TrackingState != AR_TRACKING_STATE_TRACKING) continue;

            ArPose* pARPose = 0;

            ArPose_create(m_pARSession, 0, &pARPose);

            ArAnchor_getPose(m_pARSession, rObject.m_pAnchor, pARPose);

            ArPose_getMatrix(m_pARSession, pARPose, glm::value_ptr(rObject.m_ModelMatrix));

            ArPose_destroy(pARPose);
        }
#endif
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnPause()
    {
#ifdef PLATFORM_ANDROID
        if (m_pARSession == nullptr) return;

        ArSession_pause(m_pARSession);
#endif
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnResume()
    {
#ifdef PLATFORM_ANDROID
        void* pEnvironment = Core::JNI::GetJavaEnvironment();
        void* pActivity    = Core::JNI::GetActivity();
        void* pContext     = Core::JNI::GetContext();

        if(!Core::JNI::CheckPermission(s_Permissions[0]))
        {
            Core::JNI::AcquirePermissions(s_Permissions, 1);
        }

        if (m_pARSession == nullptr && pEnvironment != nullptr && pActivity != nullptr && pContext != nullptr && Core::JNI::CheckPermission(s_Permissions[0]))
        {
            ArStatus Status;

            // -----------------------------------------------------------------------------
            // AR installation
            // -----------------------------------------------------------------------------
            ArInstallStatus InstallStatus;

            bool UserRequestedInstallation = !m_InstallRequested;

            Status = ArCoreApk_requestInstall(pEnvironment, pActivity, UserRequestedInstallation, &InstallStatus);

            if (Status != AR_SUCCESS) return;

            switch (InstallStatus)
            {
                case AR_INSTALL_STATUS_INSTALLED:
                    break;
                case AR_INSTALL_STATUS_INSTALL_REQUESTED:
                {
                    BASE_CONSOLE_INFO("ArCore is not installed on this device.");

                    m_InstallRequested = true;

                    return;
                } break;
            }

            // -----------------------------------------------------------------------------
            // Create session
            // -----------------------------------------------------------------------------
            Status = ArSession_create(pEnvironment, pContext, &m_pARSession);

            if (Status != AR_SUCCESS) BASE_THROWM("Application has to be closed because of unsupported ArCore.");

            assert(m_pARSession != 0);

            ArConfig* ARConfig = 0;

            ArConfig_create(m_pARSession, &ARConfig);

            assert(ARConfig != 0);

            Status = ArSession_checkSupported(m_pARSession, ARConfig);

            assert(Status == AR_SUCCESS);

            Status = ArSession_configure(m_pARSession, ARConfig);

            assert(Status == AR_SUCCESS);

            ArConfig_destroy(ARConfig);

            ArFrame_create(m_pARSession, &m_pARFrame);

            assert(m_pARFrame != 0);

            // -----------------------------------------------------------------------------
            // Default geometry
            // -----------------------------------------------------------------------------
            int Rotation = Core::JNI::GetDeviceRotation();
            int Width    = Core::JNI::GetDeviceDimension()[0];
            int Height   = Core::JNI::GetDeviceDimension()[1];

            ArSession_setDisplayGeometry(m_pARSession, Rotation, Width, Height);
        }

        if (m_pARSession != nullptr)
        {
            ArSession_resume(m_pARSession);
        }
#endif
    }

    // -----------------------------------------------------------------------------


    void CMRControlManager::OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height)
    {
#ifdef PLATFORM_ANDROID
        ArSession_setDisplayGeometry(m_pARSession, _DisplayRotation, _Width, _Height);
#else
        BASE_UNUSED(_DisplayRotation);
        BASE_UNUSED(_Width);
        BASE_UNUSED(_Height);
#endif
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnDraw()
    {
        // -----------------------------------------------------------------------------
        // Prepare
        // -----------------------------------------------------------------------------
        Gfx::TargetSetManager::ClearTargetSet(m_BackgroundTargetSetPtr, glm::vec4(1.0f));

        Gfx::ContextManager::SetTargetSet(m_BackgroundTargetSetPtr);

        Gfx::ContextManager::SetBlendState(Gfx::StateManager::GetBlendState(Gfx::CBlendState::Default));

        Gfx::ContextManager::SetDepthStencilState(Gfx::StateManager::GetDepthStencilState(Gfx::CDepthStencilState::NoDepth));

        Gfx::ContextManager::SetRasterizerState(Gfx::StateManager::GetRasterizerState(Gfx::CRasterizerState::NoCull));

        Gfx::ContextManager::SetViewPortSet(Gfx::ViewManager::GetViewPortSet());

        // -----------------------------------------------------------------------------
        // Background image from webcam
        // -----------------------------------------------------------------------------
#ifdef PLATFORM_ANDROID
        int32_t HasGeometryChanged = 0;

        ArFrame_getDisplayGeometryChanged(m_pARSession, m_pARFrame, &HasGeometryChanged);

        if (HasGeometryChanged != 0 || g_IsUVsInitialized == false)
        {
            ArFrame_transformDisplayUvCoords(m_pARSession, m_pARFrame, s_NumberOfVertices * 2, c_Uvs, g_TransformedUVs);

            Gfx::BufferManager::UploadBufferData(m_WebcamUVBufferPtr, &g_TransformedUVs);

            g_IsUVsInitialized = true;
        }
#endif // PLATFORM_ANDROID

        Gfx::ContextManager::SetShaderVS(m_WebcamVSPtr);

        Gfx::ContextManager::SetShaderPS(m_WebcamPSPtr);

        Gfx::ContextManager::SetVertexBuffer(m_WebcamUVBufferPtr);

        Gfx::ContextManager::SetInputLayout(m_WebcamVSPtr->GetInputLayout());

#ifdef PLATFORM_ANDROID
        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_EXTERNAL_OES, g_TextureID);
#endif // PLATFORM_ANDROID

        Gfx::ContextManager::Draw(4, 0);

#ifdef PLATFORM_ANDROID
        // -----------------------------------------------------------------------------
        // Render planes
        // -----------------------------------------------------------------------------
        bool RenderPlanes = Base::CProgramParameters::GetInstance().Get<bool>("mr:ar:debug:render_planes", true);

        if (RenderPlanes == false) return;

        std::vector<glm::vec3> PlaneVertices;
        std::vector<GLushort> PlaneIndices;

        glm::mat4 PlaneModelMatrix = glm::mat4(1.0f);

        auto UpdateGeometryForPlane = [&](const ArPlane* _pPlane)
        {
            // -----------------------------------------------------------------------------
            // Settings:
            // Feather distance 0.2 meters.
            // Feather scale over the distance between plane center and vertices.
            // -----------------------------------------------------------------------------
            const float kFeatherLength = 0.2f;
            const float kFeatherScale  = 0.2f;
            const float kOuterAlpha    = 0.8f;
            const float kInnerAlpha    = 0.8f;

            // -----------------------------------------------------------------------------
            // The following code generates a triangle mesh filling a convex polygon,
            // including a feathered edge for blending.
            //
            // The indices shown in the diagram are used in comments below.
            // _______________     0_______________1
            // |             |      |4___________5|
            // |             |      | |         | |
            // |             | =>   | |         | |
            // |             |      | |         | |
            // |             |      |7-----------6|
            // ---------------     3---------------2
            // -----------------------------------------------------------------------------

            PlaneVertices.clear();
            PlaneIndices .clear();

            int LengthOfPolygon;

            ArPlane_getPolygonSize(m_pARSession, _pPlane, &LengthOfPolygon);

            int NumberOfVertices = LengthOfPolygon / 2;

            std::vector<glm::vec2> VerticesRAW(NumberOfVertices);

            ArPlane_getPolygon(m_pARSession, _pPlane, &VerticesRAW.front()[0]);

            // -----------------------------------------------------------------------------
            // Fill vertex 0 to 3. Note that the vertex.xy are used for x and z
            // position. vertex.z is used for alpha. The outter polygon's alpha
            // is 0.
            // -----------------------------------------------------------------------------
            for (int IndexOfVertex = 0; IndexOfVertex < NumberOfVertices; ++IndexOfVertex)
            {
                PlaneVertices.push_back(glm::vec3(VerticesRAW[IndexOfVertex][0], VerticesRAW[IndexOfVertex][1], kOuterAlpha));
            }

            // -----------------------------------------------------------------------------
            // Generate pose and get model matrix
            // -----------------------------------------------------------------------------
            ArPose* Pose;

            ArPose_create(m_pARSession, nullptr, &Pose);

            ArPlane_getCenterPose(m_pARSession, _pPlane, Pose);

            ArPose_getMatrix(m_pARSession, Pose, glm::value_ptr(PlaneModelMatrix));

            ArPose_destroy(Pose);

            // -----------------------------------------------------------------------------
            // Get plane center in XZ axis.
            // -----------------------------------------------------------------------------
            glm::vec2 CenterOfPlane = glm::vec2(PlaneModelMatrix[3][0], PlaneModelMatrix[3][2]);

            // -----------------------------------------------------------------------------
            // Fill vertex 0 to 3, with alpha set to kAlpha.
            // -----------------------------------------------------------------------------
            for (auto Vertex : VerticesRAW)
            {
                glm::vec2 Direction = Vertex - CenterOfPlane;

                float Scale = 1.0f - std::min((kFeatherLength / glm::length(Direction)), kFeatherScale);

                glm::vec2 ResultVector = Scale * Direction + CenterOfPlane;

                PlaneVertices.push_back(glm::vec3(ResultVector[0], ResultVector[1], kInnerAlpha));
            }

            // -----------------------------------------------------------------------------
            // Generate vertices / triangles
            // -----------------------------------------------------------------------------
            NumberOfVertices = PlaneVertices.size();

            int NumberOfVerticesHalf = NumberOfVertices / 2.0f;

            // -----------------------------------------------------------------------------
            // Generate triangle (4, 5, 6) and (4, 6, 7).
            // -----------------------------------------------------------------------------
            for (int IndexOfIndice = NumberOfVerticesHalf + 1; IndexOfIndice < NumberOfVertices - 1; ++IndexOfIndice)
            {
                PlaneIndices.push_back(NumberOfVerticesHalf);
                PlaneIndices.push_back(IndexOfIndice);
                PlaneIndices.push_back(IndexOfIndice + 1);
            }

            // -----------------------------------------------------------------------------
            // Generate triangle (0, 1, 4), (4, 1, 5), (5, 1, 2), (5, 2, 6),
            // (6, 2, 3), (6, 3, 7), (7, 3, 0), (7, 0, 4)
            // -----------------------------------------------------------------------------
            for (int IndexOfIndice = 0; IndexOfIndice < NumberOfVerticesHalf; ++IndexOfIndice)
            {
                PlaneIndices.push_back(IndexOfIndice);
                PlaneIndices.push_back((IndexOfIndice + 1) % NumberOfVerticesHalf);
                PlaneIndices.push_back(IndexOfIndice + NumberOfVerticesHalf);

                PlaneIndices.push_back(IndexOfIndice + NumberOfVerticesHalf);
                PlaneIndices.push_back((IndexOfIndice + 1) % NumberOfVerticesHalf);
                PlaneIndices.push_back((IndexOfIndice + NumberOfVerticesHalf + 1) % NumberOfVerticesHalf + NumberOfVerticesHalf);
            }
        };

        // -----------------------------------------------------------------------------
        // Get trackable planes
        // -----------------------------------------------------------------------------
        ArTrackableList* ListOfPlanes = nullptr;

        ArTrackableList_create(m_pARSession, &ListOfPlanes);

        assert(ListOfPlanes != nullptr);

        ArSession_getAllTrackables(m_pARSession, AR_TRACKABLE_PLANE, ListOfPlanes);

        int NumberOfPlanes = 0;

        ArTrackableList_getSize(m_pARSession, ListOfPlanes, &NumberOfPlanes);

        // -----------------------------------------------------------------------------
        // Update every available plane
        // -----------------------------------------------------------------------------
        Gfx::ContextManager::SetBlendState(Gfx::StateManager::GetBlendState(Gfx::CBlendState::AlphaBlend));

        Gfx::ContextManager::SetShaderVS(m_PlaneVS);

        Gfx::ContextManager::SetShaderPS(m_PlanePS);

        Gfx::ContextManager::SetVertexBuffer(m_PlaneVerticesBufferPtr);

        Gfx::ContextManager::SetIndexBuffer(m_PlaneIndicesBufferPtr, 0);

        Gfx::ContextManager::SetConstantBuffer(0, m_MatrixBufferPtr);

        Gfx::ContextManager::SetConstantBuffer(1, m_ColorBufferPtr);

        Gfx::ContextManager::SetInputLayout(m_PlaneVS->GetInputLayout());

        for (int IndexOfPlane = 0; IndexOfPlane < NumberOfPlanes; ++IndexOfPlane)
        {
            ArTrackable* pTrackableItem = nullptr;

            ArTrackableList_acquireItem(m_pARSession, ListOfPlanes, IndexOfPlane, &pTrackableItem);

            ArPlane* pPlane = ArAsPlane(pTrackableItem);

            ArTrackingState TrackableTrackingState;

            ArTrackable_getTrackingState(m_pARSession, pTrackableItem, &TrackableTrackingState);

            ArPlane* pSubsumedPlane;

            ArPlane_acquireSubsumedBy(m_pARSession, pPlane, &pSubsumedPlane);

            if (pSubsumedPlane != nullptr)
            {
                ArTrackable_release(ArAsTrackable(pSubsumedPlane));

                continue;
            }

            if (TrackableTrackingState != AR_TRACKING_STATE_TRACKING) continue;

            ArTrackingState PlaneTrackingState;

            ArTrackable_getTrackingState(m_pARSession, ArAsTrackable(pPlane), &PlaneTrackingState);

            if (PlaneTrackingState != AR_TRACKING_STATE_TRACKING) continue;

            ArTrackable_release(pTrackableItem);

            // -----------------------------------------------------------------------------
            // Generate planes and upload data
            // -----------------------------------------------------------------------------
            UpdateGeometryForPlane(pPlane);

            if (PlaneIndices.size() == 0 || PlaneVertices.size() == 0) continue;

            if (PlaneVertices.size() >= s_MaxNumberOfVerticesPerPlane)
            {
                BASE_CONSOLE_WARNING("Plane could not be rendered because of too many vertices.");
                continue;
            }

            // -----------------------------------------------------------------------------
            // Prepare model-view-projection matrix
            // TODO: Change color depending on height of the plane
            // -----------------------------------------------------------------------------
            glm::mat4 PlaneMVPMatrix = Gfx::Cam::GetProjectionMatrix() * Gfx::Cam::GetViewMatrix() * glm::mat4(m_ARCToEngineMatrix) * PlaneModelMatrix;

            glm::vec4 Color = glm::vec4(GetPlaneColor(IndexOfPlane), 1.0f);

            // -----------------------------------------------------------------------------
            // Upload data
            // -----------------------------------------------------------------------------
            Gfx::BufferManager::UploadBufferData(m_PlaneVerticesBufferPtr, &PlaneVertices.front()[0]);

            Gfx::BufferManager::UploadBufferData(m_PlaneIndicesBufferPtr, &PlaneIndices.front());

            Gfx::BufferManager::UploadBufferData(m_MatrixBufferPtr, &PlaneMVPMatrix);

            Gfx::BufferManager::UploadBufferData(m_ColorBufferPtr, &Color);

            // -----------------------------------------------------------------------------
            // Draw
            // -----------------------------------------------------------------------------
            Gfx::ContextManager::DrawIndexed(PlaneIndices.size(), 0, 0);
        }

        ArTrackableList_destroy(ListOfPlanes);

        ListOfPlanes = nullptr;

        // -----------------------------------------------------------------------------
        // Render planes
        // -----------------------------------------------------------------------------
        bool RenderPoints = Base::CProgramParameters::GetInstance().Get<bool>("mr:ar:debug:render_points", true);

        if (RenderPoints == false) return;

        ArPointCloud* pPointCloud = nullptr;

        ArStatus Status = ArFrame_acquirePointCloud(m_pARSession, m_pARFrame, &pPointCloud);

        if (Status == AR_SUCCESS)
        {
            // -----------------------------------------------------------------------------
            // Generate points and upload data
            // -----------------------------------------------------------------------------
            int NumberOfPoints = 0;

            ArPointCloud_getNumberOfPoints(m_pARSession, pPointCloud, &NumberOfPoints);

            if (NumberOfPoints > 0)
            {
                const float* pPointCloudData;

                ArPointCloud_getData(m_pARSession, pPointCloud, &pPointCloudData);

                glm::mat4 PointMVPMatrix = Gfx::Cam::GetProjectionMatrix() * Gfx::Cam::GetViewMatrix() * glm::mat4(m_ARCToEngineMatrix);

                // -----------------------------------------------------------------------------
                // Upload
                // -----------------------------------------------------------------------------
                Gfx::BufferManager::UploadBufferData(m_PointVerticesBufferPtr, pPointCloudData);

                Gfx::BufferManager::UploadBufferData(m_MatrixBufferPtr, &PointMVPMatrix);

                // -----------------------------------------------------------------------------
                // Draw
                // -----------------------------------------------------------------------------
                Gfx::ContextManager::SetShaderVS(m_PointVS);

                Gfx::ContextManager::SetShaderPS(m_PointPS);

                Gfx::ContextManager::SetVertexBuffer(m_PointVerticesBufferPtr);

                Gfx::ContextManager::SetConstantBuffer(0, m_MatrixBufferPtr);

                Gfx::ContextManager::SetTopology(Gfx::STopology::PointList);

                Gfx::ContextManager::SetInputLayout(m_PlaneVS->GetInputLayout());

                Gfx::ContextManager::Draw(NumberOfPoints, 0);

                Gfx::ContextManager::SetTopology(Gfx::STopology::TriangleList);

                ArPointCloud_release(pPointCloud);
            }
        }
#endif

        Gfx::ContextManager::ResetViewPortSet();

        Gfx::ContextManager::ResetRenderContext();
    }

    // -----------------------------------------------------------------------------

    const CCamera& CMRControlManager::GetCamera()
    {
        return m_Camera;
    }

    // -----------------------------------------------------------------------------

    const CLightEstimation& CMRControlManager::GetLightEstimation()
    {
        return m_LightEstimation;
    }

    // -----------------------------------------------------------------------------

    const CMarker* CMRControlManager::AcquireNewMarker(float _X, float _Y)
    {
#ifdef PLATFORM_ANDROID
        CInternMarker* pReturnMarker = nullptr;

        if (m_pARFrame != nullptr && m_pARSession != nullptr)
        {
            ArHitResultList* pHitResultList = 0;

            ArHitResultList_create(m_pARSession, &pHitResultList);

            assert(pHitResultList);

            ArFrame_hitTest(m_pARSession, m_pARFrame, _X, _Y, pHitResultList);

            int NumberOfHits = 0;

            ArHitResultList_getSize(m_pARSession, pHitResultList, &NumberOfHits);

            // -----------------------------------------------------------------------------
            // The hitTest method sorts the resulting list by distance from the camera,
            // increasing.  The first hit result will usually be the most relevant when
            // responding to user input
            // -----------------------------------------------------------------------------
            ArHitResult* pHitResult = nullptr;

            for (int IndexOfHit = 0; IndexOfHit < NumberOfHits; ++IndexOfHit)
            {
                ArHitResult* pEstimatedHitResult = nullptr;

                ArHitResult_create(m_pARSession, &pEstimatedHitResult);

                ArHitResultList_getItem(m_pARSession, pHitResultList, IndexOfHit, pEstimatedHitResult);

                if (pEstimatedHitResult == nullptr)
                {
                    return nullptr;
                }

                // -----------------------------------------------------------------------------
                // Get trackables
                // -----------------------------------------------------------------------------
                ArTrackable* pTrackable = nullptr;

                ArHitResult_acquireTrackable(m_pARSession, pEstimatedHitResult, &pTrackable);

                ArTrackableType TrackableType = AR_TRACKABLE_NOT_VALID;

                ArTrackable_getType(m_pARSession, pTrackable, &TrackableType);

                switch (TrackableType)
                {
                    case AR_TRACKABLE_PLANE:
                    {
                        ArPose* pPose = nullptr;

                        ArPose_create(m_pARSession, nullptr, &pPose);

                        ArHitResult_getHitPose(m_pARSession, pEstimatedHitResult, pPose);

                        int32_t IsPoseInPolygon = 0;

                        ArPlane* pPlane = ArAsPlane(pTrackable);

                        ArPlane_isPoseInPolygon(m_pARSession, pPlane, pPose, &IsPoseInPolygon);

                        ArTrackable_release(pTrackable);

                        ArPose_destroy(pPose);

                        if (!IsPoseInPolygon)
                        {
                            continue;
                        }

                        pHitResult = pEstimatedHitResult;
                    } break;

                    case AR_TRACKABLE_POINT:
                    {
                        ArPoint* pPoint = ArAsPoint(pTrackable);

                        ArPointOrientationMode OrientationMode;

                        ArPoint_getOrientationMode(m_pARSession, pPoint, &OrientationMode);

                        if (OrientationMode == AR_POINT_ORIENTATION_ESTIMATED_SURFACE_NORMAL)
                        {
                            pHitResult = pEstimatedHitResult;
                        }
                    } break;

                    default:
                    {
                        ArTrackable_release(pTrackable);

                        BASE_CONSOLE_INFO("Undefined trackable type found.")
                    }
                };
            }

            if (pHitResult != nullptr)
            {
                ArAnchor* pAnchor = nullptr;

                if (ArHitResult_acquireNewAnchor(m_pARSession, pHitResult, &pAnchor) != AR_SUCCESS)
                {
                    return nullptr;
                }

                ArTrackingState TrackingState = AR_TRACKING_STATE_STOPPED;

                ArAnchor_getTrackingState(m_pARSession, pAnchor, &TrackingState);

                if (TrackingState != AR_TRACKING_STATE_TRACKING)
                {
                    ArAnchor_release(pAnchor);

                    return nullptr;
                }

                CInternMarker NewMarker;

                NewMarker.m_pAnchor = pAnchor;

                m_TrackedObjects.emplace_back(NewMarker);

                pReturnMarker = &m_TrackedObjects.back();

                ArHitResult_destroy(pHitResult);
            }

            ArHitResultList_destroy(pHitResultList);

            pHitResultList = nullptr;
        }

        return pReturnMarker;
#else
        BASE_UNUSED(_X);
        BASE_UNUSED(_Y);

        return nullptr;
#endif
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::ReleaseMarker(const CMarker* _pMarker)
    {
#ifdef PLATFORM_ANDROID
        auto MarkerIter = std::find_if(m_TrackedObjects.begin(), m_TrackedObjects.end(), [&](CInternMarker& _rObject) { return &_rObject == _pMarker; });

        if (MarkerIter == m_TrackedObjects.end()) return;

        const CInternMarker* pInternMarker = static_cast<const CInternMarker*>(_pMarker);

        ArAnchor_release(pInternMarker->m_pAnchor);

        m_TrackedObjects.erase(MarkerIter);
#else
        BASE_UNUSED(_pMarker);
#endif
    }

    // -----------------------------------------------------------------------------

    Gfx::CTexturePtr CMRControlManager::GetBackgroundTexture()
    {
        return m_BackgroundTexturePtr;
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        BASE_UNUSED(_pEntity);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnDirtyComponent(Dt::IComponent* _pComponent)
    {
        BASE_UNUSED(_pComponent);
    }
} // namespace

namespace MR
{
namespace ControlManager
{
    void OnStart()
    {
        CMRControlManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CMRControlManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CMRControlManager::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    void OnPause()
    {
        CMRControlManager::GetInstance().OnPause();
    }

    // -----------------------------------------------------------------------------

    void OnResume()
    {
        CMRControlManager::GetInstance().OnResume();
    }

    // -----------------------------------------------------------------------------

    void OnDisplayGeometryChanged(int _DisplayRotation, int _Width, int _Height)
    {
        CMRControlManager::GetInstance().OnDisplayGeometryChanged(_DisplayRotation, _Width, _Height);
    }

    // -----------------------------------------------------------------------------

    void OnDraw()
    {
        return CMRControlManager::GetInstance().OnDraw();
    }

    // -----------------------------------------------------------------------------

    const CCamera& GetCamera()
    {
        return CMRControlManager::GetInstance().GetCamera();
    }

    // -----------------------------------------------------------------------------

    const CLightEstimation& GetLightEstimation()
    {
        return CMRControlManager::GetInstance().GetLightEstimation();
    }

    // -----------------------------------------------------------------------------

    const CMarker* AcquireNewMarker(float _X, float _Y)
    {
        return CMRControlManager::GetInstance().AcquireNewMarker(_X, _Y);
    }

    // -----------------------------------------------------------------------------

    void ReleaseMarker(const CMarker* _pMarker)
    {
        CMRControlManager::GetInstance().ReleaseMarker(_pMarker);
    }

    // -----------------------------------------------------------------------------

    Gfx::CTexturePtr GetBackgroundTexture()
    {
        return CMRControlManager::GetInstance().GetBackgroundTexture();
    }
} // namespace ControlManager
} // namespace MR