
#pragma once

#include "editor/edit_state.h"

#include "editor_port/edit_message.h"

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
        
    private:

        void OnExit(Edit::CMessage& _rMessage);
        void OnPlay(Edit::CMessage& _rMessage);
        void OnNewMap(Edit::CMessage& _rMessage);

        void OnHighlightEntity(Edit::CMessage& _rMessage);

        void OnMouseLeftReleased(Edit::CMessage& _rMessage);
    };
} // namespace Edit