
#include "editor_imgui/edit_precompiled.h"

#include "base/base_include_glm.h"

#include "editor_imgui/edit_inspector_panel.h"

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

    void CInspectorPanel::InspectEntity(int _ID)
    {
        m_pEntity = Dt::EntityManager::GetEntityByID(_ID);
    }

    // -----------------------------------------------------------------------------

    void CInspectorPanel::Render()
    {
        ImGui::Begin("Inspector");

        if (m_pEntity)
        {
            m_pEntity->OnGUI();
        }

        ImGui::End();
    }
} // namespace GUI
} // namespace Edit