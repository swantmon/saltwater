
#include "engine/engine_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_component.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"

#include "logic/lg_start_state.h"

#include "script/script_script_manager.h"

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
        Scpt::ScriptManager::OnStart();
        Dt::EntityManager::OnStart();

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