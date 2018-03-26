
#include "mr/mr_precompiled.h"

#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_jni_interface.h"

#include "gui/gui_event_handler.h"

#include "mr/mr_session_manager.h"

#include <vector>

#include "arcore_c_api.h"

using namespace MR;
using namespace MR::SessionManager;

namespace
{
    class CMRSessionManager : private Base::CUncopyable
    {
    BASE_SINGLETON_FUNC(CMRSessionManager);

    public:

        CMRSessionManager();
        ~CMRSessionManager();

    public:

        void OnStart();
        void OnExit();
        void Update();

        void Initialize();

        void OnDisplayGeometryChanged(SDisplayRotation::EDisplayRotation _DisplayRotation, int _Width, int _Height);

        void SetTexture(int _TextureId);

        void OnPause();

        void OnResume();

        const CSession& GetSession();

    private:

        class CInternSession : public CSession
        {
        public:

            bool m_IsUVsInitialized;
            ArSession* m_pARSession;
            ArFrame* m_pARFrame;

        private:

            friend class CMRSessionManager;
        };

    private:

        CInternSession m_Session;
    };
} // namespace

namespace
{
    CMRSessionManager::CMRSessionManager()
    {
    }

    // -----------------------------------------------------------------------------

    CMRSessionManager::~CMRSessionManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CMRSessionManager::OnStart()
    {

    }

    // -----------------------------------------------------------------------------

    void CMRSessionManager::OnExit()
    {
#if PLATFORM_ANDROID
        if (m_Session.m_pARSession == nullptr) return;

        ArSession_destroy(m_Session.m_pARSession);

        ArFrame_destroy(m_Session.m_pARFrame);
#endif
    }

    // -----------------------------------------------------------------------------

    void CMRSessionManager::Update()
    {
#if PLATFORM_ANDROID
        if (m_Session.m_pARSession == nullptr) return;

        ArStatus Result;

        Result = ArSession_update(m_Session.m_pARSession, m_Session.m_pARFrame);

        switch (Result)
        {
        case AR_ERROR_FATAL:
            m_Session.m_State = CSession::Error;
            break;
        case AR_ERROR_SESSION_PAUSED:
            m_Session.m_State = CSession::Paused;
            break;
        case AR_ERROR_TEXTURE_NOT_SET:
            m_Session.m_State = CSession::TextureNotSet;
            break;
        case AR_ERROR_MISSING_GL_CONTEXT:
            m_Session.m_State = CSession::MissingGLContext;
            break;
        case AR_ERROR_CAMERA_NOT_AVAILABLE:
            m_Session.m_State = CSession::CameraNotAvailable;
            break;
        }

        if (Result != AR_SUCCESS) return;

        m_Session.m_HasGeometryChanged = false;

        const float c_Uvs[] = {
            0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        };

        int32_t HasGeometryChanged = 0;

        ArFrame_getDisplayGeometryChanged(m_Session.m_pARSession, m_Session.m_pARFrame, &HasGeometryChanged);

        if (HasGeometryChanged != 0 || m_Session.m_IsUVsInitialized == false)
        {
            ArFrame_transformDisplayUvCoords(m_Session.m_pARSession, m_Session.m_pARFrame, 8, c_Uvs, &m_Session.m_UVs[0][0]);

            m_Session.m_IsUVsInitialized = true;

            m_Session.m_HasGeometryChanged = true;
        }
#endif
    }

    // -----------------------------------------------------------------------------

    void CMRSessionManager::OnPause()
    {
#if PLATFORM_ANDROID
        if (m_Session.m_pARSession == nullptr) return;

        ArSession_pause(m_Session.m_pARSession);
#endif // PLATFORM_ANDROID
    }

    // -----------------------------------------------------------------------------

    void CMRSessionManager::OnResume()
    {
#ifdef PLATFORM_ANDROID
        if (m_Session.m_pARSession != nullptr)
        {
            ArSession_resume(m_Session.m_pARSession);
        }
#endif // PLATFORM_ANDROID
    }

    // -----------------------------------------------------------------------------

    void CMRSessionManager::SetTexture(int _TextureId)
    {
#ifdef PLATFORM_ANDROID
        if (m_Session.m_pARSession == nullptr) return;

        ArSession_setCameraTextureName(m_Session.m_pARSession, _TextureId);
#endif // PLATFORM_ANDROID
    }

    // -----------------------------------------------------------------------------

    void CMRSessionManager::Initialize()
    {
#ifdef PLATFORM_ANDROID
        void* pEnvironment = Core::JNI::GetJavaEnvironment();
        void* pContext = Core::JNI::GetContext();

        if (pEnvironment == nullptr || pContext == nullptr) return;

        ArStatus Status;

        // -----------------------------------------------------------------------------
        // AR session and frame
        // -----------------------------------------------------------------------------
        Status = ArSession_create(pEnvironment, pContext, &m_Session.m_pARSession);

        if (Status != AR_SUCCESS) BASE_THROWM("Application has to be closed because of unsupported ArCore.");

        assert(m_Session.m_pARSession != 0);

        ArConfig* ARConfig = 0;

        ArConfig_create(m_Session.m_pARSession, &ARConfig);

        assert(ARConfig != 0);

        Status = ArSession_checkSupported(m_Session.m_pARSession, ARConfig);

        assert(Status == AR_SUCCESS);

        Status = ArSession_configure(m_Session.m_pARSession, ARConfig);

        assert(Status == AR_SUCCESS);

        ArConfig_destroy(ARConfig);

        ArFrame_create(m_Session.m_pARSession, &m_Session.m_pARFrame);

        assert(m_Session.m_pARFrame != 0);

        // -----------------------------------------------------------------------------
        // Set external vars
        // -----------------------------------------------------------------------------
        m_Session.m_pSession = m_Session.m_pARSession;
        m_Session.m_pFrame = m_Session.m_pARFrame;
#endif // PLATFORM_ANDROID
    }

    // -----------------------------------------------------------------------------

    void CMRSessionManager::OnDisplayGeometryChanged(SDisplayRotation::EDisplayRotation _DisplayRotation, int _Width, int _Height)
    {
#if PLATFORM_ANDROID
        ArSession_setDisplayGeometry(m_Session.m_pARSession, _DisplayRotation, _Width, _Height);
#else
        BASE_UNUSED(_DisplayRotation);
        BASE_UNUSED(_Width);
        BASE_UNUSED(_Height);
#endif
    }

    // -----------------------------------------------------------------------------

    const CSession& CMRSessionManager::GetSession()
    {
        return m_Session;
    }
} // namespace

namespace MR
{
namespace SessionManager
{
    void OnStart()
    {
        CMRSessionManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CMRSessionManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CMRSessionManager::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    void Initialize()
    {
        CMRSessionManager::GetInstance().Initialize();
    }

    // -----------------------------------------------------------------------------

    void OnDisplayGeometryChanged(SDisplayRotation::EDisplayRotation _DisplayRotation, int _Width, int _Height)
    {
        CMRSessionManager::GetInstance().OnDisplayGeometryChanged(_DisplayRotation, _Width, _Height);
    }

    // -----------------------------------------------------------------------------

    void SetTexture(int _TextureId)
    {
        CMRSessionManager::GetInstance().SetTexture(_TextureId);
    }

    // -----------------------------------------------------------------------------

    void OnPause()
    {
        CMRSessionManager::GetInstance().OnPause();
    }

    // -----------------------------------------------------------------------------

    void OnResume()
    {
        CMRSessionManager::GetInstance().OnResume();
    }

    // -----------------------------------------------------------------------------

    const CSession& GetSession()
    {
        return CMRSessionManager::GetInstance().GetSession();
    }
} // namespace SessionManager
} // namespace MR