
#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_actor_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_fx_manager.h"
#include "data/data_light_manager.h"
#include "data/data_material_manager.h"
#include "data/data_model_manager.h"
#include "data/data_texture_manager.h"

#include "logic/lg_exit_state.h"

#include "mr/mr_control_manager.h"

namespace
{
    class CLgExitState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CLgExitState)
        
    public:
        
        int OnEnter();
        int OnLeave();
        int OnRun();
        
    };
} // namespace

namespace
{
    int CLgExitState::OnEnter()
    {
        MR::ControlManager::OnExit();

        Dt::TextureManager ::OnExit();
        Dt::MaterialManager::OnExit();
        Dt::ModelManager   ::OnExit();
        Dt::FXManager      ::OnExit();
        Dt::ActorManager   ::OnExit();
        Dt::EntityManager  ::OnExit();
        Dt::LightManager   ::OnExit();

        return Lg::Exit::SResult::Exit;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgExitState::OnLeave()
    {
        return Lg::Exit::SResult::Exit;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgExitState::OnRun()
    {
        return Lg::Exit::SResult::Exit;
    }
} // namespace

namespace Lg
{
namespace Exit
{
    int OnEnter()
    {
        return CLgExitState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnLeave()
    {
        return CLgExitState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnRun()
    {
        return CLgExitState::GetInstance().OnRun();
    }
} // namespace Exit
} // namespace Lg