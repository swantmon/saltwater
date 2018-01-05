
#include "logic/lg_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_area_light_manager.h"
#include "data/data_bloom_manager.h"
#include "data/data_camera_actor_manager.h"
#include "data/data_dof_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_post_aa_manager.h"
#include "data/data_light_probe_manager.h"
#include "data/data_material_manager.h"
#include "data/data_mesh_manager.h"
#include "data/data_model_manager.h"
#include "data/data_point_light_manager.h"
#include "data/data_sky_manager.h"
#include "data/data_ssao_manager.h"
#include "data/data_ssr_manager.h"
#include "data/data_sun_manager.h"
#include "data/data_texture_manager.h"
#include "data/data_volume_fog_manager.h"

#include "logic/lg_start_state.h"

/*#include "mr/mr_control_manager.h"*/

namespace
{
    class CLgStartState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CLgStartState)
        
    public:
        
        int OnEnter();
        int OnLeave();
        int OnRun();
        
    };
} // namespace

namespace
{
    int CLgStartState::OnEnter()
    {        
        // -----------------------------------------------------------------------------
        // Data
        // -----------------------------------------------------------------------------
        Dt::MeshActorManager  ::OnStart();
        Dt::CameraActorManager::OnStart();
        Dt::EntityManager     ::OnStart();
        Dt::SunManager        ::OnStart();
        Dt::LightProbeManager ::OnStart();
        Dt::PointLightManager ::OnStart();
        Dt::AreaLightManager  ::OnStart();
        Dt::SkyManager        ::OnStart();
        Dt::BloomManager      ::OnStart();
        Dt::DOFManager        ::OnStart();
        Dt::PostAAManager     ::OnStart();
        Dt::SSAOManager       ::OnStart();
        Dt::SSRFXManager      ::OnStart();
        Dt::VolumeFogManager  ::OnStart();
        Dt::ModelManager      ::OnStart();
        Dt::MaterialManager   ::OnStart();
        Dt::TextureManager    ::OnStart();

        // -----------------------------------------------------------------------------
        // External
        // -----------------------------------------------------------------------------
/*        MR::ControlManager::OnStart();*/

        return Lg::Start::SResult::Start;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgStartState::OnLeave()
    {
        return Lg::Start::SResult::Start;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgStartState::OnRun()
    {
        return Lg::Start::SResult::Intro;
    }
} // namespace

namespace Lg
{
namespace Start
{
    int OnEnter()
    {
        return CLgStartState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnLeave()
    {
        return CLgStartState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnRun()
    {
        return CLgStartState::GetInstance().OnRun();
    }
} // namespace Start
} // namespace Lg