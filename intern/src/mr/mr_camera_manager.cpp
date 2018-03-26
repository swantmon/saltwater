
#include "mr/mr_precompiled.h"

#include "base/base_program_parameters.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "mr/mr_camera_manager.h"
#include "mr/mr_session_manager.h"

#include "arcore_c_api.h"

using namespace MR;

namespace 
{
    class CMRCameraManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CMRCameraManager)

    public:

        CMRCameraManager();
        ~CMRCameraManager();

    public:

        void OnStart();
        void OnExit();

        void Update();

        const CCamera& GetCamera();

    private:

        class CInternCamera : public CCamera
        {
        private:

            friend class CMRCameraManager;
        };

    private:

        CInternCamera m_Camera;
    };
} // namespace 

namespace 
{
    CMRCameraManager::CMRCameraManager()
    {

    }

    // -----------------------------------------------------------------------------

    CMRCameraManager::~CMRCameraManager()
    {

    }

    // -----------------------------------------------------------------------------

    void CMRCameraManager::OnStart()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CMRCameraManager::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CMRCameraManager::Update()
    {
#if PLATFORM_ANDROID
        const CSession& rActiveSession = MR::SessionManager::GetSession();

        if (rActiveSession.GetSessionState() != CSession::Success) return;

        ArSession* pARSession = static_cast<ArSession*>(rActiveSession.GetSession());
        ArFrame* pARFrame     = static_cast<ArFrame*>(rActiveSession.GetFrame());

        m_Camera.m_Near = Base::CProgramParameters::GetInstance().Get<float>("mr:ar:camera:near", 0.1f);
        m_Camera.m_Far  = Base::CProgramParameters::GetInstance().Get<float>("mr:ar:camera:far", 100.0f);

        ArCamera* pARCamera;

        ArFrame_acquireCamera(pARSession, pARFrame, &pARCamera);

        ArCamera_getViewMatrix(pARSession, pARCamera, glm::value_ptr(m_Camera.m_ViewMatrix));

        ArCamera_getProjectionMatrix(pARSession, pARCamera, m_Camera.m_Near, m_Camera.m_Far, glm::value_ptr(m_Camera.m_ProjectionMatrix));

        ArTrackingState CameraTrackingState;

        ArCamera_getTrackingState(pARSession, pARCamera, &CameraTrackingState);

        ArCamera_release(pARCamera);

        switch (CameraTrackingState)
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
#endif
    }

    // -----------------------------------------------------------------------------

    const CCamera& CMRCameraManager::GetCamera()
    {
        return m_Camera;
    }
} // namespace 

namespace MR
{
namespace CameraManager
{
    void OnStart()
    {
        CMRCameraManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CMRCameraManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CMRCameraManager::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    const CCamera& GetCamera()
    {
        return CMRCameraManager::GetInstance().GetCamera();
    }
} // namespace CameraManager
} // namespace MR