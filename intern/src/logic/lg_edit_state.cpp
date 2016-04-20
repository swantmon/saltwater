
#include "base/base_console.h"
#include "base/base_input_event.h"
#include "base/base_math_operations.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "camera/cam_ar_control.h"
#include "camera/cam_control_manager.h"

#include "core/core_time.h"

#include "data/data_actor_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_light_facet.h"
#include "data/data_light_manager.h"
#include "data/data_map.h"

#include "gui/gui_event_handler.h"

#include "logic/lg_edit_state.h"

#include "mr/mr_control_manager.h"
#include "mr/mr_marker_info.h"
#include "mr/mr_tracker_manager.h"
#include "mr/mr_webcam_control.h"

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
        Dt::ActorManager ::Update();
        Dt::EntityManager::Update();
        Dt::LightManager ::Update();       
        
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