
#include "editor/edit_precompiled.h"

#include "editor/edit_edit_state.h"
#include "editor/edit_unload_map_state.h"
#include "editor/edit_inspector_panel.h"

#include "engine/camera/cam_control_manager.h"

#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"

#include "engine/graphic/gfx_selection_renderer.h"

#include "engine/gui/gui_input_manager.h"
#include "engine/gui/gui_event_handler.h"

#include "engine/script/script_script_manager.h"

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
    }
    
    // -----------------------------------------------------------------------------
    
    CEditState::~CEditState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CEditState::InternOnEnter()
    {
        // -----------------------------------------------------------------------------
        // Input
        // -----------------------------------------------------------------------------
        m_EventDelegate = Gui::EventHandler::RegisterDirectUserListener(std::bind(&CEditState::OnInputEvent, this, std::placeholders::_1));

        // -----------------------------------------------------------------------------
        // Acquire an selection ticket at selection renderer
        // -----------------------------------------------------------------------------
        assert(m_pSelectionTicket == 0);

        m_pSelectionTicket = &Gfx::SelectionRenderer::AcquireTicket(-1, -1, 1, 1, Gfx::SPickFlag::Everything);
        
        return Edit::CState::Edit;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CEditState::InternOnLeave()
    {
		// -----------------------------------------------------------------------------
		// Unregister event
		// -----------------------------------------------------------------------------
		m_EventDelegate = 0;

        // -----------------------------------------------------------------------------
        // Clear ticket
        // -----------------------------------------------------------------------------
        Gfx::SelectionRenderer::Clear(*m_pSelectionTicket);

        m_pSelectionTicket = 0;

        // -----------------------------------------------------------------------------
        // Unselect entity
        // -----------------------------------------------------------------------------
        Gfx::SelectionRenderer::UnselectEntity();

        Edit::GUI::CInspectorPanel::GetInstance().InspectEntity(Dt::CEntity::s_InvalidID);

        // -----------------------------------------------------------------------------
        // Reset action
        // -----------------------------------------------------------------------------
        m_Action = CState::Edit;
        
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

                Edit::GUI::CInspectorPanel::GetInstance().InspectEntity(pEntity->GetID());
            }
            else
            {
                Gfx::SelectionRenderer::UnselectEntity();

                Edit::GUI::CInspectorPanel::GetInstance().InspectEntity(Dt::CEntity::s_InvalidID);
            }
        }

        return NextState;
    }

    // -----------------------------------------------------------------------------

    void CEditState::OnInputEvent(const Base::CInputEvent& _rInputEvent)
    {
        if (_rInputEvent.GetType() == Base::CInputEvent::Exit)
        {
            m_Action = CState::Exit;
        }
        else if (_rInputEvent.GetType() == Base::CInputEvent::Input)
        {
            if (_rInputEvent.GetAction() == Base::CInputEvent::MouseLeftReleased && m_pSelectionTicket != 0)
            {
                Gfx::SelectionRenderer::PushPick(*m_pSelectionTicket, _rInputEvent.GetLocalCursorPosition());
            }
        }
    }
} // namespace Edit