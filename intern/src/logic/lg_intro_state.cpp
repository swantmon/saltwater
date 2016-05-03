
#include "logic/lg_precompiled.h"

#include "base/base_input_event.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "gui/gui_event_handler.h"

#include "logic/lg_intro_state.h"

namespace
{
    class CLgIntroState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CLgIntroState)
        
    public:
        
        CLgIntroState();
        
    public:
        
        int OnEnter();
        int OnLeave();
        int OnRun();
        
    private:
        
        void OnInputEvent(const Base::CInputEvent& _rInputEvent);
        
    private:
        
        bool m_SkipIntro;
        
    };
} // namespace

namespace
{
    CLgIntroState::CLgIntroState()
        : m_SkipIntro(false)
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgIntroState::OnEnter()
    {
        // -----------------------------------------------------------------------------
        // Register input event delegate @ gui event handler
        // -----------------------------------------------------------------------------
        Gui::EventHandler::RegisterDirectUserListener(GUI_BIND_INPUT_METHOD(&CLgIntroState::OnInputEvent));
        
        return Lg::Intro::SResult::Intro;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgIntroState::OnLeave()
    {
        return Lg::Intro::SResult::Intro;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgIntroState::OnRun()
    {
        return Lg::Intro::SResult::MainMenu;
    }
    
    // -----------------------------------------------------------------------------
    
    void CLgIntroState::OnInputEvent(const Base::CInputEvent& _rInputEvent)
    {
        BASE_UNUSED(_rInputEvent);
    }
} // namespace

namespace Lg
{
namespace Intro
{
    int OnEnter()
    {
        return CLgIntroState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnLeave()
    {
        return CLgIntroState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnRun()
    {
        return CLgIntroState::GetInstance().OnRun();
    }
} // namespace Intro
} // namespace Lg