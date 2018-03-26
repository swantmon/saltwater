
#include "mr/mr_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "mr/mr_session_manager.h"
#include "mr/mr_light_estimation_manager.h"

#include "arcore_c_api.h"

using namespace MR;

namespace 
{
    class CMRLightEstimationManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CMRLightEstimationManager)

    public:

        CMRLightEstimationManager();
        ~CMRLightEstimationManager();

    public:

        void OnStart();
        void OnExit();

        void Update();

        const CLightEstimation& GetLightEstimation();

    private:

        class CInternLightEstimation : public CLightEstimation
        {
        private:

            friend class CMRLightEstimationManager;
        };

    private:

        CInternLightEstimation m_LightEstimation;
    };
} // namespace 

namespace 
{
    CMRLightEstimationManager::CMRLightEstimationManager()
    {

    }

    // -----------------------------------------------------------------------------

    CMRLightEstimationManager::~CMRLightEstimationManager()
    {

    }

    // -----------------------------------------------------------------------------

    void CMRLightEstimationManager::OnStart()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CMRLightEstimationManager::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CMRLightEstimationManager::Update()
    {
#ifdef PLATFORM_ANDROID
        const CSession& rActiveSession = MR::SessionManager::GetSession();

        if (rActiveSession.GetSessionState() != CSession::Success) return;

        ArSession* pARSession = static_cast<ArSession*>(rActiveSession.GetSession());
        ArFrame* pARFrame = static_cast<ArFrame*>(rActiveSession.GetFrame());

        // -----------------------------------------------------------------------------
        // Light estimation
        // Intensity value ranges from 0.0f to 1.0f.
        // -----------------------------------------------------------------------------
        ArLightEstimate* ARLightEstimate;
        ArLightEstimateState ARLightEstimateState;

        ArLightEstimate_create(pARSession, &ARLightEstimate);

        ArFrame_getLightEstimate(pARSession, pARFrame, ARLightEstimate);

        ArLightEstimate_getState(pARSession, ARLightEstimate, &ARLightEstimateState);

        m_LightEstimation.m_EstimationState = CLightEstimation::NotValid;

        if (ARLightEstimateState == AR_LIGHT_ESTIMATE_STATE_VALID)
        {
            ArLightEstimate_getPixelIntensity(pARSession, ARLightEstimate, &m_LightEstimation.m_Intensity);

            m_LightEstimation.m_EstimationState = CLightEstimation::Valid;
        }

        ArLightEstimate_destroy(ARLightEstimate);

        ARLightEstimate = nullptr;
#endif // PLATFORM_ANDROID
    }

    // -----------------------------------------------------------------------------

    const CLightEstimation& CMRLightEstimationManager::GetLightEstimation()
    {
        return m_LightEstimation;
    }
} // namespace 

namespace MR
{
namespace LightEstimationManager
{
    void OnStart()
    {
        CMRLightEstimationManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CMRLightEstimationManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CMRLightEstimationManager::GetInstance().Update();
    }

    // -----------------------------------------------------------------------------

    const CLightEstimation& GetLightEstimation()
    {
        return CMRLightEstimationManager::GetInstance().GetLightEstimation();
    }
} // namespace LightEstimationManager
} // namespace MR