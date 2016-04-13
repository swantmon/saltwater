
#include "base/base_input_event.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "gui/gui_event_handler.h"

#include "logic/lg_main_menu_state.h"

namespace
{
    class CLgMainMenuState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CLgMainMenuState)
        
    public:
        
        CLgMainMenuState();
        
    public:
        
        int OnEnter();
        int OnLeave();
        int OnRun();
        
    private:
        
        void OnInputEvent(const Base::CInputEvent& _rInputEvent);
        
    };
} // namespace

namespace
{
    CLgMainMenuState::CLgMainMenuState()
    {
        
    }

    // -----------------------------------------------------------------------------

    int CLgMainMenuState::OnEnter()
    {
        // -----------------------------------------------------------------------------
        // Register input event delegate @ gui event handler
        // -----------------------------------------------------------------------------
        Gui::EventHandler::RegisterDirectUserListener(GUI_BIND_INPUT_METHOD(&CLgMainMenuState::OnInputEvent));
        
        return Lg::MainMenu::SResult::MainMenu;
    }

    // -----------------------------------------------------------------------------

    int CLgMainMenuState::OnLeave()
    {
        return Lg::MainMenu::SResult::MainMenu;
    }

    // -----------------------------------------------------------------------------

    int CLgMainMenuState::OnRun()
    { 
        return Lg::MainMenu::SResult::LoadMap;
    }

    // -----------------------------------------------------------------------------

    void CLgMainMenuState::OnInputEvent(const Base::CInputEvent& _rInputEvent)
    {
        BASE_UNUSED(_rInputEvent);
    }
} // namespace

namespace Lg
{
namespace MainMenu
{
    int OnEnter()
    {
        return CLgMainMenuState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnLeave()
    {
        return CLgMainMenuState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnRun()
    {
        return CLgMainMenuState::GetInstance().OnRun();
    }
} // namespace MainMenu
} // namespace Lg
