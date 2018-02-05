
#include "editor/edit_precompiled.h"

#include "base/base_console.h"

#include "camera/cam_control_manager.h"

#include "editor/edit_play_state.h"
#include "editor/edit_unload_map_state.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include "graphic/gfx_play_state.h"

#include "gui/gui_play_state.h"

#include "logic/lg_play_state.h"

namespace Edit
{
    CPlayState& CPlayState::GetInstance()
    {
        static CPlayState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Edit

namespace Edit
{
    CPlayState::CPlayState()
        : m_Action(CState::Play)
    {
        // -----------------------------------------------------------------------------
        // Register messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SGUIMessageType::App_Exit, EDIT_RECEIVE_MESSAGE(&CPlayState::OnExit));
        Edit::MessageManager::Register(Edit::SGUIMessageType::App_Edit, EDIT_RECEIVE_MESSAGE(&CPlayState::OnEdit));
    }
    
    // -----------------------------------------------------------------------------
    
    CPlayState::~CPlayState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CPlayState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("Edit> Enter play state.");

        // -----------------------------------------------------------------------------
        // Activate game control for play state
        // -----------------------------------------------------------------------------
        Cam::ControlManager::SetActiveControl(Cam::CControl::GameControl);

        // Cam::ControlManager::GetActiveControl().SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));

        // -----------------------------------------------------------------------------
        // Running states
        // -----------------------------------------------------------------------------
        Lg ::Play::OnEnter();
        Gui::Play::OnEnter();
        Gfx::Play::OnEnter();
        
        return Edit::CState::Play;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CPlayState::InternOnLeave()
    {
        // -----------------------------------------------------------------------------
        // Reset action
        // -----------------------------------------------------------------------------
        m_Action = CState::Play;

        // -----------------------------------------------------------------------------
        // Running states
        // -----------------------------------------------------------------------------
        Gfx::Play::OnLeave();
        Gui::Play::OnLeave();
        Lg ::Play::OnLeave();

        BASE_CONSOLE_STREAMINFO("Edit> Leave play state.");
        
        return Edit::CState::Play;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CPlayState::InternOnRun()
    {
        CState::EStateType NextState = CState::Play;

        switch (m_Action)
        {
        case Edit::CState::Exit:
            CUnloadMapState::GetInstance().SetNextState(CState::Exit);
            NextState = CState::UnloadMap;
            break;
        case Edit::CState::Edit:
            NextState = CState::Edit;
            break;
        }
        
        // -----------------------------------------------------------------------------
        // Update logic
        // -----------------------------------------------------------------------------
        Lg::Play::OnRun();
        
        // -----------------------------------------------------------------------------
        // Update cameras and views depending on logic and world
        // -----------------------------------------------------------------------------
        Cam::ControlManager::Update();
        
        // -----------------------------------------------------------------------------
        // Update graphic and GUI
        // -----------------------------------------------------------------------------
        Gui::Play::OnRun();
        Gfx::Play::OnRun();
        
        return NextState;
    }

    // -----------------------------------------------------------------------------

    void CPlayState::OnExit(Edit::CMessage& _rMessage)
    {
        m_Action = CState::Exit;
    }

    // -----------------------------------------------------------------------------

    void CPlayState::OnEdit(Edit::CMessage& _rMessage)
    {
        m_Action = CState::Edit;
    }
} // namespace Edit