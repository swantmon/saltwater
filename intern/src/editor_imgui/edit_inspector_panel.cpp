
#include "editor_imgui/edit_precompiled.h"

#include "base/base_include_glm.h"

#include "editor_imgui/edit_inspector_panel.h"
#include "editor_imgui/edit_gui_factory.h"

#include "engine/data/data_entity_manager.h"

#include "imgui.h"

namespace Edit
{
namespace GUI
{
    CInspectorPanel::CInspectorPanel()
    {

    }

    // -----------------------------------------------------------------------------

    CInspectorPanel::~CInspectorPanel()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorPanel::InspectEntity(Dt::CEntity::BID _ID)
    {
        if (_ID == Dt::CEntity::s_InvalidID)
        {
            m_pEntity = 0;
            return;
        }

        m_pEntity = Dt::EntityManager::GetEntityByID(_ID);
    }

    // -----------------------------------------------------------------------------

    void CInspectorPanel::Render()
    {
        Edit::CGUIFactory& rFactory = Edit::CGUIFactory::GetInstance();

        ImGui::Begin("Inspector");

        if (m_pEntity)
        {
            if (rFactory.HasClass<Dt::CEntity>())
            {
                auto Panel = rFactory.GetClass<Dt::CEntity>(m_pEntity);

                Panel->OnGUI();
            }
        }

        ImGui::End();
    }
} // namespace GUI
} // namespace Edit