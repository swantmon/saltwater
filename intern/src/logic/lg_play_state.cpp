
#include "engine/engine_precompiled.h"

#include "base/base_console.h"
#include "base/base_input_event.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "camera/cam_control_manager.h"
#include "camera/cam_game_control.h"

#include "core/core_time.h"

#include "data/data_camera_component.h"
#include "data/data_component.h"
#include "data/data_component_facet.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"

#include "gui/gui_event_handler.h"

#include "logic/lg_play_state.h"

#include "script/script_script_manager.h"

namespace
{
    class CLgPlayState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CLgPlayState)
        
    public:
        
        int OnEnter();
        int OnLeave();
        int OnRun();
    
    private:
        
        int m_State;
        
    private:
        
        void OnInputEvent(const Base::CInputEvent& _rInputEvent);
        
    };
} // namespace

namespace
{
    int CLgPlayState::OnEnter()
    {
        // -----------------------------------------------------------------------------
        // Define variables
        // -----------------------------------------------------------------------------
        m_State = Lg::Play::SResult::Play;
        
        // -----------------------------------------------------------------------------
        // Register input event delegate @ gui event handler
        // -----------------------------------------------------------------------------
        Gui::EventHandler::RegisterDirectUserListener(GUI_BIND_INPUT_METHOD(&CLgPlayState::OnInputEvent));
        
        return Lg::Play::SResult::Play;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgPlayState::OnLeave()
    {        
        // -----------------------------------------------------------------------------
        // Unregister input event delegate @ gui event handler
        // -----------------------------------------------------------------------------
        Gui::EventHandler::UnregisterDirectUserListener(GUI_BIND_INPUT_METHOD(&CLgPlayState::OnInputEvent));
        
        return Lg::Play::SResult::Play;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgPlayState::OnRun()
    {
        // -----------------------------------------------------------------------------
        // Update data manager
        // -----------------------------------------------------------------------------
        Dt::EntityManager::Update();

        Scpt::ScriptManager::Update();

        // -----------------------------------------------------------------------------
        // Return state changes
        // -----------------------------------------------------------------------------
        return m_State;
    }
    
    // -----------------------------------------------------------------------------
    
    void CLgPlayState::OnInputEvent(const Base::CInputEvent& _rInputEvent)
    {
        auto AddLightToScene = [&] (const glm::vec3& _rPosition, float _Range, const glm::vec3& _rColor)
        {
            BASE_UNUSED(_rPosition);
            BASE_UNUSED(_Range);
            BASE_UNUSED(_rColor);
        };
        
        if (_rInputEvent.GetAction() == Base::CInputEvent::KeyReleased)
        {
            
            if (_rInputEvent.GetKey() == Base::CInputEvent::Key1)
            {
                glm::vec3 CurrentPosition = Cam::ControlManager::GetActiveControl().GetPosition();
                
                AddLightToScene(CurrentPosition, 8.0f, glm::vec3(1.0f, 0.0f, 0.0f));
            }
            
            if (_rInputEvent.GetKey() == Base::CInputEvent::Key2)
            {
                glm::vec3 CurrentPosition = Cam::ControlManager::GetActiveControl().GetPosition();
                
                AddLightToScene(CurrentPosition, 8.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            }
            
            if (_rInputEvent.GetKey() == Base::CInputEvent::Key3)
            {
                glm::vec3 CurrentPosition = Cam::ControlManager::GetActiveControl().GetPosition();
                
                AddLightToScene(CurrentPosition, 8.0f, glm::vec3(0.0f, 0.0f, 1.0f));
            }
            
            if (_rInputEvent.GetKey() == Base::CInputEvent::KeyRhombos)
            {
                m_State = Lg::Play::SResult::Debug;
            }
        }
    }
} // namespace

namespace Lg
{
namespace Play
{
    int OnEnter()
    {
        return CLgPlayState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnLeave()
    {
        return CLgPlayState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnRun()
    {
        return CLgPlayState::GetInstance().OnRun();
    }
} // namespace Play
} // namespace Lg