
#include "editor/edit_precompiled.h"

#include "base/base_console.h"

#include "editor/edit_intro_state.h"

#include "editor_port/edit_message_manager.h"

#include "graphic/gfx_intro_state.h"

#include "gui/gui_intro_state.h"

#include "logic/lg_intro_state.h"

namespace Edit
{
    CIntroState& CIntroState::GetInstance()
    {
        static CIntroState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Edit

namespace Edit
{
    CIntroState::CIntroState()
        : m_CurrentState(EStateType::Intro)
    {
        MessageManager::Register(SGUIMessageType::App_LoadMap, EDIT_RECEIVE_MESSAGE(&CIntroState::OnLoadMap));
        MessageManager::Register(Edit::SGUIMessageType::App_NewMap, EDIT_RECEIVE_MESSAGE(&CIntroState::OnNewMap));
    }
    
    // -----------------------------------------------------------------------------
    
    CIntroState::~CIntroState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CIntroState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("Edit> Enter intro state.");

        // -----------------------------------------------------------------------------
        // Load default state behavior
        // -----------------------------------------------------------------------------
        m_CurrentState = EStateType::Intro;

        Lg ::Intro::OnEnter();
        Gfx::Intro::OnEnter();
        Gui::Intro::OnEnter();
        
        return Edit::CState::Intro;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CIntroState::InternOnLeave()
    {
        Gui::Intro::OnLeave();
        Gfx::Intro::OnLeave();
        Lg ::Intro::OnLeave();

        BASE_CONSOLE_STREAMINFO("Edit> Leave intro state.");
        
        return Edit::CState::Intro;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CIntroState::InternOnRun()
    {
        Lg ::Intro::OnRun();
        Gfx::Intro::OnRun();
        Gui::Intro::OnRun();
        
        return m_CurrentState;
    }

    // -----------------------------------------------------------------------------

    void CIntroState::OnNewMap(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Allocate a map
        // -----------------------------------------------------------------------------
        int MapX = _rMessage.GetInt();
        int MapY = _rMessage.GetInt();

        BASE_UNUSED(MapX);
        BASE_UNUSED(MapY);

        m_CurrentState = Edit::CState::LoadMap;
    }

    // -----------------------------------------------------------------------------

    void CIntroState::OnLoadMap(Edit::CMessage& _rMessage)
    {
        (void)_rMessage;

        m_CurrentState = Edit::CState::LoadMap;
    }
} // namespace Edit