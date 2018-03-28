
#include "editor/edit_precompiled.h"

#include "base/base_console.h"

#include "camera/cam_control_manager.h"

#include "core/core_plugin_manager.h"

#include "data/data_entity.h"

#include "editor/edit_edit_state.h"
#include "editor/edit_unload_map_state.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include "graphic/gfx_edit_state.h"
#include "graphic/gfx_selection_renderer.h"

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
        Edit::MessageManager::Register(Edit::SGUIMessageType::App_NewMap, EDIT_RECEIVE_MESSAGE(&CEditState::OnNewMap));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Graphic_HighlightEntity, EDIT_RECEIVE_MESSAGE(&CEditState::OnHighlightEntity));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Input_MouseLeftReleased, EDIT_RECEIVE_MESSAGE(&CEditState::OnMouseLeftReleased));
    }
    
    // -----------------------------------------------------------------------------
    
    CEditState::~CEditState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CEditState::InternOnEnter()
    {
        // -----------------------------------------------------------------------------
        // Load plugins
        // -----------------------------------------------------------------------------
        Core::PluginManager::LoadPlugin("arcore");

        auto& Plugin = Core::PluginManager::GetPlugin("arcore");

        Plugin.OnStart();

        BASE_CONSOLE_STREAMINFO("Edit> Enter edit state.");

        // -----------------------------------------------------------------------------
        // Acquire an selection ticket at selection renderer
        // -----------------------------------------------------------------------------
        assert(m_pSelectionTicket == 0);

        m_pSelectionTicket = &Gfx::SelectionRenderer::AcquireTicket(-1, -1, 1, 1, Gfx::SPickFlag::Actor);

        // -----------------------------------------------------------------------------
        // Set editor camera in edit state
        // -----------------------------------------------------------------------------
        Cam::ControlManager::SetActiveControl(Cam::CControl::EditorControl);

        // Cam::ControlManager::GetActiveControl().SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));

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
        // Clear ticket
        // -----------------------------------------------------------------------------
        Gfx::SelectionRenderer::Clear(*m_pSelectionTicket);

        m_pSelectionTicket = 0;

        // -----------------------------------------------------------------------------
        // Unselect entity
        // -----------------------------------------------------------------------------
        Gfx::SelectionRenderer::UnselectEntity();

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
        // Selection
        // -----------------------------------------------------------------------------
        assert(m_pSelectionTicket != 0);

        Gfx::CSelectionTicket& rSelectionTicket = *m_pSelectionTicket;

        if (Gfx::SelectionRenderer::PopPick(rSelectionTicket))
        {
            if (rSelectionTicket.m_HitFlag == Gfx::SHitFlag::Entity && rSelectionTicket.m_pObject != nullptr)
            {
                Dt::CEntity* pEntity = (Dt::CEntity*)rSelectionTicket.m_pObject;

                Gfx::SelectionRenderer::SelectEntity(pEntity->GetID());

                // -----------------------------------------------------------------------------
                // Send entity to editor
                // -----------------------------------------------------------------------------
                Edit::CMessage NewMessage;

                NewMessage.Put(pEntity->GetID());

                NewMessage.Reset();

                Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Entity_Selected, NewMessage);
            }
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
        BASE_UNUSED(_rMessage);

        m_Action = CState::Exit;
    }

    // -----------------------------------------------------------------------------

    void CEditState::OnPlay(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);

        m_Action = CState::Play;
    }

    // -----------------------------------------------------------------------------

    void CEditState::OnNewMap(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);

        m_Action = CState::Intro;
    }

    // -----------------------------------------------------------------------------

    void CEditState::OnHighlightEntity(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        if (EntityID != static_cast<Base::ID>(-1))
        {
            Gfx::SelectionRenderer::SelectEntity(EntityID);
        }
        else
        {
            Gfx::SelectionRenderer::UnselectEntity();
        }
    }

    // -----------------------------------------------------------------------------

    void CEditState::OnMouseLeftReleased(Edit::CMessage& _rMessage)
    {
        int GlobalMousePositionX = _rMessage.Get<int>();
        int GlobalMousePositionY = _rMessage.Get<int>();
        int LocalMousePositionX  = _rMessage.Get<int>();
        int LocalMousePositionY  = _rMessage.Get<int>();

        BASE_UNUSED(GlobalMousePositionX);
        BASE_UNUSED(GlobalMousePositionY);

        if (m_pSelectionTicket != 0)
        {
            Gfx::SelectionRenderer::PushPick(*m_pSelectionTicket, glm::ivec2(LocalMousePositionX, LocalMousePositionY));
        }
    }
} // namespace Edit