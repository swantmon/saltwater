
#include "editor/edit_precompiled.h"

#include "base/base_console.h"

#include "camera/cam_control_manager.h"

#include "editor/edit_edit_state.h"
#include "editor/edit_unload_map_state.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include "graphic/gfx_edit_state.h"

#include "gui/gui_edit_state.h"

#include "logic/lg_edit_state.h"

namespace Edit
{
    CEditState& CEditState::GetInstance()
    {
        static CEditState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Edit

namespace Edit
{
    CEditState::CEditState()
        : m_Action(CState::Edit)
    {
        // -----------------------------------------------------------------------------
        // Register messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SGUIMessageType::App_Exit  , EDIT_RECEIVE_MESSAGE(&CEditState::OnExit));
        Edit::MessageManager::Register(Edit::SGUIMessageType::App_Play  , EDIT_RECEIVE_MESSAGE(&CEditState::OnPlay));
    }
    
    // -----------------------------------------------------------------------------
    
    CEditState::~CEditState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CEditState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("Edit> Enter edit state.");

        // -----------------------------------------------------------------------------
        // Set editor camera in edit state
        // -----------------------------------------------------------------------------
        Cam::ControlManager::SetActiveControl(Cam::CControl::EditorControl);

        // Cam::ControlManager::GetActiveControl().SetPosition(Base::Float3(0.0f, 0.0f, 10.0f));

        // -----------------------------------------------------------------------------
        // Running states
        // -----------------------------------------------------------------------------
        Lg ::Edit::OnEnter();
        Gui::Edit::OnEnter();
        Gfx::Edit::OnEnter();        
        
        return Edit::CState::Edit;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CEditState::InternOnLeave()
    {
        // -----------------------------------------------------------------------------
        // Reset action
        // -----------------------------------------------------------------------------
        m_Action = CState::Edit;

        // -----------------------------------------------------------------------------
        // Running states
        // -----------------------------------------------------------------------------
        Gfx::Edit::OnLeave();
        Gui::Edit::OnLeave();
        Lg ::Edit::OnLeave();

        BASE_CONSOLE_STREAMINFO("Edit> Leave edit state.");
        
        return Edit::CState::Edit;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CEditState::InternOnRun()
    {
        CState::EStateType NextState = CState::Edit;

        switch (m_Action)
        {
        case Edit::CState::Exit:
            CUnloadMapState::GetInstance().SetNextState(CState::Exit);
            NextState = Edit::CState::UnloadMap;
            break;
        case Edit::CState::Play:
            NextState = Edit::CState::Play;
            break;
        }

        // -----------------------------------------------------------------------------
        // Update logic
        // -----------------------------------------------------------------------------
        Lg::Edit::OnRun();

        // -----------------------------------------------------------------------------
        // Update cameras and views depending on logic and world
        // -----------------------------------------------------------------------------
        Cam::ControlManager::Update();

        // -----------------------------------------------------------------------------
        // Update graphic and GUI
        // -----------------------------------------------------------------------------
        Gui::Edit::OnRun();
        Gfx::Edit::OnRun();

        return NextState;
    }

    // -----------------------------------------------------------------------------

    void CEditState::OnExit(Edit::CMessage& _rMessage)
    {
        m_Action = CState::Exit;
    }

    // -----------------------------------------------------------------------------

    void CEditState::OnPlay(Edit::CMessage& _rMessage)
    {
        m_Action = CState::Play;
    }
} // namespace Edit