
#include "mr/mr_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "mr/mr_light_estimation_manager.h"

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
} // namespace LightEstimationManager
} // namespace MR