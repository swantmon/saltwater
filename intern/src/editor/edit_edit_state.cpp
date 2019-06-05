
#include "editor/edit_precompiled.h"

#include "editor/edit_edit_state.h"
#include "editor/edit_unload_map_state.h"
#include "editor/edit_inspector_panel.h"

#include "engine/camera/cam_control_manager.h"
#include "engine/camera/cam_editor_control.h"

#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"

#include "engine/graphic/gfx_selection_renderer.h"
#include "engine/graphic/gfx_highlight_renderer.h"

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
        : CState            (Edit)
        , m_CurrentOperation(Hand)
        , m_CurrentMode     (World)
        , m_DirtyFlag       (false)
        , m_pSelectionTicket(nullptr)
    {
        m_NextState = CState::Edit;
    }
    
    // -----------------------------------------------------------------------------
    
    CEditState::~CEditState()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CEditState::SetOperation(EOperation _Operation)
    {
        m_CurrentOperation = _Operation;
    }

    // -----------------------------------------------------------------------------

    CEditState::EOperation CEditState::GetOperation() const
    {
        return m_CurrentOperation;
    }

    // -----------------------------------------------------------------------------

    void CEditState::SetMode(EMode _Mode)
    {
        m_CurrentMode = _Mode;
    }

    // -----------------------------------------------------------------------------

    CEditState::EMode CEditState::GetMode() const
    {
        return m_CurrentMode;
    }

    // -----------------------------------------------------------------------------

    void CEditState::SetDirty(bool _Flag)
    {
        m_DirtyFlag = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CEditState::IsDirty() const
    {
        return m_DirtyFlag;
    }
    
    // -----------------------------------------------------------------------------
    
    void CEditState::InternOnEnter()
    {
        // -----------------------------------------------------------------------------
        // Input
        // -----------------------------------------------------------------------------
        m_OnEventDelegate = Gui::EventHandler::RegisterEventHandler(std::bind(&CEditState::OnEvent, this, std::placeholders::_1));

        // -----------------------------------------------------------------------------
        // Acquire an selection ticket at selection renderer
        // -----------------------------------------------------------------------------
        assert(m_pSelectionTicket == nullptr);

        m_pSelectionTicket = &Gfx::SelectionRenderer::AcquireTicket(-1, -1, 1, 1, Gfx::SPickFlag::Everything);
    }
    
    // -----------------------------------------------------------------------------
    
    void CEditState::InternOnLeave()
    {
        // -----------------------------------------------------------------------------
        // Unregister event
        // -----------------------------------------------------------------------------
        m_OnEventDelegate = nullptr;

        // -----------------------------------------------------------------------------
        // Clear ticket
        // -----------------------------------------------------------------------------
        Gfx::SelectionRenderer::Clear(*m_pSelectionTicket);

        m_pSelectionTicket = nullptr;

        // -----------------------------------------------------------------------------
        // Unselect entity
        // -----------------------------------------------------------------------------
        Gfx::HighlightRenderer::Reset();

        Edit::GUI::CInspectorPanel::GetInstance().InspectEntity(Dt::CEntity::s_InvalidID);

        // -----------------------------------------------------------------------------
        // Reset state
        // -----------------------------------------------------------------------------
        m_NextState = Edit;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CEditState::InternOnRun()
    {
        // -----------------------------------------------------------------------------
        // Selection
        // -----------------------------------------------------------------------------
        assert(m_pSelectionTicket != nullptr);

        Gfx::CSelectionTicket& rSelectionTicket = *m_pSelectionTicket;

        if (Gfx::SelectionRenderer::PopPick(rSelectionTicket))
        {
            if (rSelectionTicket.m_HitFlag == Gfx::SHitFlag::Entity && rSelectionTicket.m_pObject != nullptr)
            {
                auto pEntity = static_cast<Dt::CEntity*>(rSelectionTicket.m_pObject);

                Gfx::HighlightRenderer::HighlightEntity(pEntity->GetID());

                Edit::GUI::CInspectorPanel::GetInstance().InspectEntity(pEntity->GetID());
            }
            else
            {
                Gfx::HighlightRenderer::Reset();

                Edit::GUI::CInspectorPanel::GetInstance().InspectEntity(Dt::CEntity::s_InvalidID);
            }
        }

        return m_NextState;
    }

    // -----------------------------------------------------------------------------

    void CEditState::OnEvent(const Base::CInputEvent& _rInputEvent)
    {
        if (_rInputEvent.GetType() == Base::CInputEvent::Input)
        {
            if (_rInputEvent.GetAction() == Base::CInputEvent::MouseLeftReleased && m_pSelectionTicket != nullptr)
            {
                Gfx::SelectionRenderer::PushPick(*m_pSelectionTicket, _rInputEvent.GetLocalCursorPosition());
            }

            auto EditorControl = static_cast<Cam::CEditorControl&>(Cam::ControlManager::GetActiveControl());
            
            if (_rInputEvent.GetAction() == Base::CInputEvent::KeyReleased && !EditorControl.IsFlying())
            {
                if (_rInputEvent.GetKey() == 'q')      m_CurrentOperation = EOperation::Hand;
                else if (_rInputEvent.GetKey() == 'w') m_CurrentOperation = EOperation::Translate;
                else if (_rInputEvent.GetKey() == 'e') m_CurrentOperation = EOperation::Rotate;
                else if (_rInputEvent.GetKey() == 'r') m_CurrentOperation = EOperation::Scale;
            }
        }
    }
} // namespace Edit