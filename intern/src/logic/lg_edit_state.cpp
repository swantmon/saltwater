
#include "logic/lg_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_ar_actor_manager.h"
#include "data/data_camera_actor_manager.h"
#include "data/data_entity_manager.h"
#include "data/data_light_probe_manager.h"
#include "data/data_mesh_manager.h"
#include "data/data_point_light_manager.h"
#include "data/data_script_manager.h"
#include "data/data_sky_manager.h"
#include "data/data_sun_manager.h"

#include "logic/lg_edit_state.h"

namespace
{
    class CLgEditState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CLgEditState)
        
    public:
        
        int OnEnter();
        int OnLeave();
        int OnRun();
    };
} // namespace

namespace
{
    int CLgEditState::OnEnter()
    {
        // -----------------------------------------------------------------------------
        // Define variables
        // -----------------------------------------------------------------------------
        return Lg::Edit::SResult::Edit;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgEditState::OnLeave()
    {        
        return Lg::Edit::SResult::Edit;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgEditState::OnRun()
    {
        // -----------------------------------------------------------------------------
        // Update data manager
        // -----------------------------------------------------------------------------
        Dt::MeshActorManager  ::Update();
        Dt::ARActorManager    ::Update();
        Dt::CameraActorManager::Update();
        Dt::EntityManager     ::Update();
        Dt::SunManager        ::Update();
        Dt::LightProbeManager ::Update();
        Dt::PointLightManager ::Update();
        Dt::SkyManager        ::Update();  
        Dt::ScriptManager     ::Update();
        
        // -----------------------------------------------------------------------------
        // Return state changes
        // -----------------------------------------------------------------------------
        return Lg::Edit::SResult::Edit;
    }
} // namespace

namespace Lg
{
namespace Edit
{
    int OnEnter()
    {
        return CLgEditState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnLeave()
    {
        return CLgEditState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnRun()
    {
        return CLgEditState::GetInstance().OnRun();
    }
} // namespace Edit
} // namespace Lg