
#include "plugin/arcore/mr_precompiled.h"

#include "base/base_coordinate_system.h"
#include "base/base_input_event.h"
#include "base/base_pool.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "engine/engine.h"

#include "engine/core/core_plugin.h"
#include "engine/core/core_jni_interface.h"

#include "engine/data/data_ar_controller_component.h"
#include "engine/data/data_camera_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_map.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

#include "engine/gui/gui_event_handler.h"

#include "engine/script/script_ar_settings_script.h"

#include "plugin/arcore/mr_control_manager.h"

#include <array>
#include <vector>
#include <engine/data/data_script_component.h>
#include <engine/script/script_ar_camera_control_script.h>

#include "arcore_c_api.h"

using namespace MR;
using namespace MR::ControlManager;

namespace
{
    const float c_Uvs[] = {
            0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    constexpr char c_VertexShaderWebcam[] = R"(
        #version 320 es

        precision mediump float;

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
        #version 320 es

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

        void UpdateBackground();

        void OnPause();
        void OnResume();

        const CCamera& GetCamera();

        const CLightEstimation& GetLightEstimation();

        const CMarker* AcquireNewMarker(float _X, float _Y);
        void ReleaseMarker(const CMarker* _pMarker);

        Gfx::CTexturePtr GetBackgroundTexture();

        const SSettings& GetSettings();
        void SetSettings(const SSettings& _rSettings);
        void ResetSettings();

        ArSession* GetCurrentSession();
        ArFrame* GetCurrentFrame();

    private:

        static std::string s_Permissions[];

    private:

        class CInternCamera : public CCamera
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

        SSettings m_Settings;

        ArSession* m_pARSession;
        ArFrame* m_pARFrame;
        CTrackedObjects m_TrackedObjects;

        CInternCamera m_Camera;

        CLightEstimation m_LightEstimation;

        glm::mat3 m_ARCToEngineMatrix;

        Gfx::CTexturePtr m_BackgroundTexturePtr;

        Gfx::CTexturePtr m_ExternalTexturePtr;

        Gfx::CTargetSetPtr m_BackgroundTargetSetPtr;

        Gfx::CShaderPtr m_WebcamVSPtr;

        Gfx::CShaderPtr m_WebcamPSPtr;

        Gfx::CBufferPtr m_WebcamUVBufferPtr;

        Gfx::Main::CResizeDelegate::HandleType m_ResizeHandle;

        Dt::CComponentManager::CComponentDelegate::HandleType m_OnDirtyComponentDelegate;

        Scpt::CARSettingsScript* m_pARSettings;

    private:

        void OnDirtyComponent(Dt::IComponent* _pComponent);

        void OnResize(int _Width, int _Height);
    };
} // namespace

namespace
{
    std::string CMRControlManager::s_Permissions[] = { "android.permission.CAMERA" };
}

namespace
{
    CMRControlManager::CMRControlManager()
        : m_InstallRequested (false)
        , m_Settings         ( )
        , m_pARSession       (0)
        , m_pARFrame         (0)
        , m_TrackedObjects   ( )
        , m_ARCToEngineMatrix(1.0f)
        , m_pARSettings      (nullptr)
    {
        m_ARCToEngineMatrix = Base::CCoordinateSystem::GetBaseMatrix(glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,0,-1));

        m_ResizeHandle = Gfx::Main::RegisterResizeHandler(std::bind(&CMRControlManager::OnResize, this, std::placeholders::_1, std::placeholders::_2));

        m_OnDirtyComponentDelegate = Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(std::bind(&CMRControlManager::OnDirtyComponent, this, std::placeholders::_1));
    }

    // -----------------------------------------------------------------------------

    CMRControlManager::~CMRControlManager()
    {
        m_ResizeHandle = nullptr;

        m_OnDirtyComponentDelegate = nullptr;

        m_pARSettings = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Shader for background and debug
        // -----------------------------------------------------------------------------
        m_WebcamVSPtr = Gfx::ShaderManager::CompileVS(c_VertexShaderWebcam, "main", nullptr, nullptr, 0, false, false, true);

        m_WebcamPSPtr = Gfx::ShaderManager::CompilePS(c_FragmentShaderWebcam, "main", nullptr, nullptr, 0, false, false, true);

        const Gfx::SInputElementDescriptor InputLayoutWebcam[] =
        {
            { "UV", 0, Gfx::CInputLayout::Float2Format, 0, 0, 8, Gfx::CInputLayout::PerVertex, 0, },
        };

        Gfx::ShaderManager::CreateInputLayout(InputLayoutWebcam, 1, m_WebcamVSPtr);


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

        m_ExternalTexturePtr = Gfx::TextureManager::CreateExternalTexture();

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
        ConstanteBufferDesc.m_NumberOfBytes = 4 * sizeof(glm::vec2);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_WebcamUVBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        // Settings
        // -----------------------------------------------------------------------------
        ResetSettings();

        // -----------------------------------------------------------------------------
        // Initialize ARCore if needed
        // -----------------------------------------------------------------------------
        OnResume();
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnExit()
    {
        m_TrackedObjects.clear();

        // -----------------------------------------------------------------------------
        // Release resources
        // -----------------------------------------------------------------------------
        m_BackgroundTexturePtr   = nullptr;
        m_ExternalTexturePtr     = nullptr;
        m_BackgroundTargetSetPtr = nullptr;
        m_WebcamVSPtr            = nullptr;
        m_WebcamPSPtr            = nullptr;
        m_WebcamUVBufferPtr      = nullptr;

        // -----------------------------------------------------------------------------
        // AR session and frame
        // -----------------------------------------------------------------------------
        ArSession_destroy(m_pARSession);

        ArFrame_destroy(m_pARFrame);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::Update()
    {
        if (m_pARSession == nullptr) return;

        ArStatus Result;

        ArSession_setCameraTextureName(m_pARSession, m_ExternalTexturePtr->GetNativeHandle());

        Result = ArSession_update(m_pARSession, m_pARFrame);

        if (Result != AR_SUCCESS) return;

        // -----------------------------------------------------------------------------
        // Update camera
        // -----------------------------------------------------------------------------
        ArCamera* pARCamera;

        ArFrame_acquireCamera(m_pARSession, m_pARFrame, &pARCamera);

        ArCamera_getViewMatrix(m_pARSession, pARCamera, glm::value_ptr(m_Camera.m_ViewMatrix));

        ArCamera_getProjectionMatrix(m_pARSession, pARCamera, m_Settings.m_Near, m_Settings.m_Far, glm::value_ptr(m_Camera.m_ProjectionMatrix));

        m_Camera.m_Near = m_Camera.m_ProjectionMatrix[2][3] / (m_Camera.m_ProjectionMatrix[2][2] - 1.0f);
        m_Camera.m_Far  = m_Camera.m_ProjectionMatrix[2][3] / (m_Camera.m_ProjectionMatrix[2][2] + 1.0f);

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

        // -----------------------------------------------------------------------------
        // Light estimation
        // -----------------------------------------------------------------------------
        m_LightEstimation.Detect();

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
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::UpdateBackground()
    {
        if (m_pARSession == nullptr) return;

        Gfx::ContextManager::SetTargetSet(m_BackgroundTargetSetPtr);

        Gfx::ContextManager::SetBlendState(Gfx::StateManager::GetBlendState(Gfx::CBlendState::Default));

        Gfx::ContextManager::SetDepthStencilState(Gfx::StateManager::GetDepthStencilState(Gfx::CDepthStencilState::NoDepth));

        Gfx::ContextManager::SetRasterizerState(Gfx::StateManager::GetRasterizerState(Gfx::CRasterizerState::NoCull));

        Gfx::ContextManager::SetViewPortSet(Gfx::ViewManager::GetViewPortSet());

        int32_t HasGeometryChanged = 0;

        ArFrame_getDisplayGeometryChanged(m_pARSession, m_pARFrame, &HasGeometryChanged);

        if (HasGeometryChanged != 0 || g_IsUVsInitialized == false)
        {
            ArFrame_transformCoordinates2d(m_pARSession, m_pARFrame, AR_COORDINATES_2D_VIEW_NORMALIZED, s_NumberOfVertices, c_Uvs, AR_COORDINATES_2D_TEXTURE_NORMALIZED, g_TransformedUVs);

            Gfx::BufferManager::UploadBufferData(m_WebcamUVBufferPtr, &g_TransformedUVs);

            g_IsUVsInitialized = true;
        }

        Gfx::ContextManager::SetTopology(Gfx::STopology::TriangleStrip);

        Gfx::ContextManager::SetShaderVS(m_WebcamVSPtr);

        Gfx::ContextManager::SetShaderPS(m_WebcamPSPtr);

        Gfx::ContextManager::SetVertexBuffer(m_WebcamUVBufferPtr);

        Gfx::ContextManager::SetInputLayout(m_WebcamVSPtr->GetInputLayout());

        Gfx::ContextManager::SetTexture(0, m_ExternalTexturePtr);

        Gfx::ContextManager::Draw(4, 0);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnPause()
    {
        if (m_pARSession == nullptr) return;

        ArSession_pause(m_pARSession);
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnResume()
    {
        void* pEnvironment = Core::JNI::GetJavaEnvironment();
        void* pActivity    = Core::JNI::GetActivity();
        void* pContext     = Core::JNI::GetContext();

        if(!Core::JNI::CheckPermission(s_Permissions[0]))
        {
            ENGINE_CONSOLE_DEBUGV("Acquire permission %s", s_Permissions[0].c_str());

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
                case AR_INSTALL_STATUS_INSTALL_REQUESTED:
                    {
                        ENGINE_CONSOLE_INFO("ArCore is not installed on this device.");

                        m_InstallRequested = true;

                        return;
                    }
                    break;
                default:
                case AR_INSTALL_STATUS_INSTALLED:
                    ENGINE_CONSOLE_DEBUG("ArCore is installed");
                    break;
            }

            // -----------------------------------------------------------------------------
            // Create session
            // -----------------------------------------------------------------------------
            Status = ArSession_create(pEnvironment, pContext, &m_pARSession);

            if (Status != AR_SUCCESS) BASE_THROWM("Application has to be closed because of unsupported ArCore.");

            assert(m_pARSession != 0);

            ArConfig* ARConfig = 0;

            ArConfig_create(m_pARSession, &ARConfig);

            ArSession_getConfig(m_pARSession, ARConfig);

            ArConfig_setLightEstimationMode(m_pARSession, ARConfig, AR_LIGHT_ESTIMATION_MODE_ENVIRONMENTAL_HDR);

            ArConfig_setUpdateMode(m_pARSession, ARConfig, AR_UPDATE_MODE_LATEST_CAMERA_IMAGE);

            ArConfig_setPlaneFindingMode(m_pARSession, ARConfig, AR_PLANE_FINDING_MODE_HORIZONTAL_AND_VERTICAL);

            if (m_pARSettings != nullptr)
            {
                switch(m_pARSettings->m_PlaneFindingMode)
                {
                    case Scpt::CARSettingsScript::SPlaneFindingMode::Disabled:
                        ArConfig_setPlaneFindingMode(m_pARSession, ARConfig, AR_PLANE_FINDING_MODE_DISABLED);
                        break;
                    case Scpt::CARSettingsScript::SPlaneFindingMode::Horizontal:
                        ArConfig_setPlaneFindingMode(m_pARSession, ARConfig, AR_PLANE_FINDING_MODE_HORIZONTAL);
                        break;
                    case Scpt::CARSettingsScript::SPlaneFindingMode::Vertical:
                        ArConfig_setPlaneFindingMode(m_pARSession, ARConfig, AR_PLANE_FINDING_MODE_VERTICAL);
                        break;
                    case Scpt::CARSettingsScript::SPlaneFindingMode::HorizontalAndVertical:
                        ArConfig_setPlaneFindingMode(m_pARSession, ARConfig, AR_PLANE_FINDING_MODE_HORIZONTAL_AND_VERTICAL);
                        break;
                }

                switch(m_pARSettings->m_LightEstimation)
                {
                    case Scpt::CARSettingsScript::SLightingEstimation::Disabled:
                        ArConfig_setLightEstimationMode(m_pARSession, ARConfig, AR_LIGHT_ESTIMATION_MODE_DISABLED);
                        break;
                    case Scpt::CARSettingsScript::SLightingEstimation::AmbientIntensity:
                        ArConfig_setLightEstimationMode(m_pARSession, ARConfig, AR_LIGHT_ESTIMATION_MODE_AMBIENT_INTENSITY);
                        break;
                    case Scpt::CARSettingsScript::SLightingEstimation::HDREnvironment:
                        ArConfig_setLightEstimationMode(m_pARSession, ARConfig, AR_LIGHT_ESTIMATION_MODE_ENVIRONMENTAL_HDR);
                        break;
                }
            }

            assert(ARConfig != 0);

            Status = ArSession_configure(m_pARSession, ARConfig);

            assert(Status == AR_SUCCESS);

            ArConfig_destroy(ARConfig);

            ArFrame_create(m_pARSession, &m_pARFrame);

            assert(m_pARFrame != 0);



            // -----------------------------------------------------------------------------
            // Default geometry
            // -----------------------------------------------------------------------------
            int Rotation = Core::JNI::GetDeviceRotation();
            int Width    = Gfx::Main::GetActiveNativeWindowSize()[0];
            int Height   = Gfx::Main::GetActiveNativeWindowSize()[1];

            ArSession_setDisplayGeometry(m_pARSession, Rotation, Width, Height);
        }

        if (m_pARSession != nullptr)
        {
            ArSession_resume(m_pARSession);
        }
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnResize(int _Width, int _Height)
    {
        int Rotation = Core::JNI::GetDeviceRotation();

        ArSession_setDisplayGeometry(m_pARSession, Rotation, _Width, _Height);
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

                        ENGINE_CONSOLE_INFO("Undefined trackable type found.")
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
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::ReleaseMarker(const CMarker* _pMarker)
    {
        auto MarkerIter = std::find_if(m_TrackedObjects.begin(), m_TrackedObjects.end(), [&](CInternMarker& _rObject) { return &_rObject == _pMarker; });

        if (MarkerIter == m_TrackedObjects.end()) return;

        const CInternMarker* pInternMarker = static_cast<const CInternMarker*>(_pMarker);

        ArAnchor_release(pInternMarker->m_pAnchor);

        m_TrackedObjects.erase(MarkerIter);
    }

    // -----------------------------------------------------------------------------

    Gfx::CTexturePtr CMRControlManager::GetBackgroundTexture()
    {
        return m_BackgroundTexturePtr;
    }

    // -----------------------------------------------------------------------------

    const SSettings& CMRControlManager::GetSettings()
    {
        return m_Settings;
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::SetSettings(const SSettings& _rSettings)
    {
        m_Settings = _rSettings;
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::ResetSettings()
    {
        m_Settings.m_Near = Core::CProgramParameters::GetInstance().Get<float>("mr:camera:projection:near", 0.1f);
        m_Settings.m_Far  = Core::CProgramParameters::GetInstance().Get<float>("mr:camera:projection:far", 100.0f);
    }

    // -----------------------------------------------------------------------------

    ArSession* CMRControlManager::GetCurrentSession()
    {
        return m_pARSession;
    }

    // -----------------------------------------------------------------------------

    ArFrame* CMRControlManager::GetCurrentFrame()
    {
        return m_pARFrame;
    }

    // -----------------------------------------------------------------------------

    void CMRControlManager::OnDirtyComponent(Dt::IComponent* _pComponent)
    {
        if (!Base::CTypeInfo::IsEqualName(_pComponent->GetTypeInfo(), Base::CTypeInfo::Get<Dt::CScriptComponent>())) return;

        auto* pScriptComponent = static_cast<Dt::CScriptComponent*>(_pComponent);

        if (!pScriptComponent->IsActiveAndUsable()) return;

        if (Base::CTypeInfo::IsEqualName(pScriptComponent->GetScriptTypeInfo(), Base::CTypeInfo::Get<Scpt::CARSettingsScript>()))
        {
            m_pARSettings = static_cast<Scpt::CARSettingsScript*>(pScriptComponent);

            if (m_pARSession != nullptr)
            {
                ArConfig* ARConfig = 0;

                ArSession_getConfig(m_pARSession, ARConfig);

                if (m_pARSettings != nullptr)
                {
                    switch(m_pARSettings->m_PlaneFindingMode)
                    {
                        case Scpt::CARSettingsScript::SPlaneFindingMode::Disabled:
                            ArConfig_setPlaneFindingMode(m_pARSession, ARConfig, AR_PLANE_FINDING_MODE_DISABLED);
                            break;
                        case Scpt::CARSettingsScript::SPlaneFindingMode::Horizontal:
                            ArConfig_setPlaneFindingMode(m_pARSession, ARConfig, AR_PLANE_FINDING_MODE_HORIZONTAL);
                            break;
                        case Scpt::CARSettingsScript::SPlaneFindingMode::Vertical:
                            ArConfig_setPlaneFindingMode(m_pARSession, ARConfig, AR_PLANE_FINDING_MODE_VERTICAL);
                            break;
                        case Scpt::CARSettingsScript::SPlaneFindingMode::HorizontalAndVertical:
                            ArConfig_setPlaneFindingMode(m_pARSession, ARConfig, AR_PLANE_FINDING_MODE_HORIZONTAL_AND_VERTICAL);
                            break;
                    }

                    switch(m_pARSettings->m_LightEstimation)
                    {
                        case Scpt::CARSettingsScript::SLightingEstimation::Disabled:
                            ArConfig_setLightEstimationMode(m_pARSession, ARConfig, AR_LIGHT_ESTIMATION_MODE_DISABLED);
                            break;
                        case Scpt::CARSettingsScript::SLightingEstimation::AmbientIntensity:
                            ArConfig_setLightEstimationMode(m_pARSession, ARConfig, AR_LIGHT_ESTIMATION_MODE_AMBIENT_INTENSITY);
                            break;
                        case Scpt::CARSettingsScript::SLightingEstimation::HDREnvironment:
                            ArConfig_setLightEstimationMode(m_pARSession, ARConfig, AR_LIGHT_ESTIMATION_MODE_ENVIRONMENTAL_HDR);
                            break;
                    }
                }

                ArSession_configure(m_pARSession, ARConfig);
            }
        }
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

    void UpdateBackground()
    {
        CMRControlManager::GetInstance().UpdateBackground();
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

    // -----------------------------------------------------------------------------

    const SSettings& GetSettings()
    {
        return CMRControlManager::GetInstance().GetSettings();
    }

    // -----------------------------------------------------------------------------

    void SetSettings(const SSettings& _rSettings)
    {
        CMRControlManager::GetInstance().SetSettings(_rSettings);
    }

    // -----------------------------------------------------------------------------

    void ResetSettings()
    {
        CMRControlManager::GetInstance().ResetSettings();
    }

    // -----------------------------------------------------------------------------

    ArSession* GetCurrentSession()
    {
        return CMRControlManager::GetInstance().GetCurrentSession();
    }

    // -----------------------------------------------------------------------------

    ArFrame* GetCurrentFrame()
    {
        return CMRControlManager::GetInstance().GetCurrentFrame();
    }
} // namespace ControlManager
} // namespace MR

extern "C" CORE_PLUGIN_API_EXPORT const MR::CCamera* GetCamera()
{
    return &MR::ControlManager::GetCamera();
}

extern "C" CORE_PLUGIN_API_EXPORT const MR::CLightEstimation* GetLightEstimation()
{
    return &MR::ControlManager::GetLightEstimation();
}

extern "C" CORE_PLUGIN_API_EXPORT const MR::CMarker* AcquireNewMarker(float _X, float _Y)
{
    return MR::ControlManager::AcquireNewMarker(_X, _Y);
}

extern "C" CORE_PLUGIN_API_EXPORT void ReleaseMarker(const MR::CMarker* _pMarker)
{
    return MR::ControlManager::ReleaseMarker(_pMarker);
}

extern "C" CORE_PLUGIN_API_EXPORT Gfx::CTexturePtr GetBackgroundTexture()
{
    return MR::ControlManager::GetBackgroundTexture();
}

extern "C" CORE_PLUGIN_API_EXPORT void SetControlSettings(MR::ControlManager::SSettings _Settings)
{
    MR::ControlManager::SetSettings(_Settings);
}