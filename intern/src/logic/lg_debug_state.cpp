
#include "logic/lg_precompiled.h"

#include "base/base_input_event.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_debug_interface.h"

#include "gui/gui_event_handler.h"

#include "logic/lg_debug_state.h"

#include <cmath>

namespace
{
    class CLgDebugState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CLgDebugState)
        
    public:
        
        CLgDebugState();
        ~CLgDebugState();
        
    public:
        
        int OnEnter();
        int OnLeave();
        int OnRun();
        
    private:
        
        int m_State;
        
        bool m_IsGizmoVisible;
        bool m_IsShadowVisible;
        bool m_IsShadowFrustumVisible;
        bool m_IsCameraFrustumVisible;
        
    private:
        
        void OnInputEvent(const Base::CInputEvent& _rInputEvent);
        
    };
} // namespace

namespace
{
    CLgDebugState::CLgDebugState()
        : m_IsGizmoVisible        (false)
        , m_IsShadowVisible       (true)
        , m_IsShadowFrustumVisible(false)
        , m_IsCameraFrustumVisible(false)
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CLgDebugState::~CLgDebugState()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgDebugState::OnEnter()
    {
        // -----------------------------------------------------------------------------
        // Set vars
        // -----------------------------------------------------------------------------
        m_State = Lg::Debug::SResult::Debug;
        
        // -----------------------------------------------------------------------------
        // Register input event delegate @ GUI event handler
        // -----------------------------------------------------------------------------
        Gui::EventHandler::RegisterDirectUserListener(GUI_BIND_INPUT_METHOD(&CLgDebugState::OnInputEvent));
        
        return Lg::Debug::SResult::Debug;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgDebugState::OnLeave()
    {
        // -----------------------------------------------------------------------------
        // Unregister input event delegate @ Gui event handler
        // -----------------------------------------------------------------------------
        Gui::EventHandler::UnregisterDirectUserListener(GUI_BIND_INPUT_METHOD(&CLgDebugState::OnInputEvent));
        
        return Lg::Debug::SResult::Debug;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgDebugState::OnRun()
    {        
        return m_State;
    }
    
    // -----------------------------------------------------------------------------
    
    void CLgDebugState::OnInputEvent(const Base::CInputEvent& _rInputEvent)
    {
        if (_rInputEvent.GetAction() == Base::CInputEvent::KeyReleased)
        {
            if (_rInputEvent.GetKey() == Base::CInputEvent::KeyRhombos)
            {
                m_State = Lg::Debug::SResult::Play;
            }
        }
    }
} // namespace

namespace Lg
{
namespace Debug
{
    int OnEnter()
    {
        return CLgDebugState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnLeave()
    {
        return CLgDebugState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnRun()
    {
        return CLgDebugState::GetInstance().OnRun();
    }
} // namespace Debug
} // namespace Lg