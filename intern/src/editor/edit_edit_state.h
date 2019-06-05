
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

    public:

        enum EOperation
        {
            Hand,
            Translate,
            Rotate,
            Scale
        };

        enum EMode
        {
            Local,
            World
        };

    public:

        void SetOperation(EOperation _Operation);
        EOperation GetOperation() const;

        void SetMode(EMode _Mode);
        EMode GetMode() const;

        void SetDirty(bool _Flag = true);
        bool IsDirty() const;

    private:

        EOperation m_CurrentOperation;

        EMode m_CurrentMode;

        bool m_DirtyFlag;

        Gfx::CSelectionTicket* m_pSelectionTicket;

        Gui::EventHandler::CEventDelegate::HandleType m_OnEventDelegate;

    private:
        
        CEditState();
        ~CEditState();
        
    private:
        
        void InternOnEnter() override;
        void InternOnLeave() override;
        CState::EStateType InternOnRun() override;

    private:

        void OnEvent(const Base::CInputEvent& _rInputEvent);
    };
} // namespace Edit