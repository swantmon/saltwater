
#pragma once

#include "editor_imgui/edit_state.h"

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

    private:
        
        CEditState();
        ~CEditState();
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
    };
} // namespace Edit