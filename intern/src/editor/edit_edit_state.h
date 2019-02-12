
#pragma once

#include "base/base_input_event.h"

#include "editor/edit_state.h"

#include "engine/gui/gui_event_handler.h"

namespace Gfx
{
    class CSelectionTicket;
} // namespace Gfx

namespace Edit
{
    class CEditState : public CState
    {
        
    public:
        
        static CEditState& GetInstance();

    private:

        CState::EStateType m_Action;

        Gfx::CSelectionTicket* m_pSelectionTicket;

        Gui::EventHandler::CEventDelegate::HandleType m_OnEventDelegate;

    private:
        
        CEditState();
        ~CEditState();
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();

    private:

        void OnEvent(const Base::CInputEvent& _rInputEvent);
    };
} // namespace Edit