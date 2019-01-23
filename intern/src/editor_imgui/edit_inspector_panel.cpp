
#include "editor_imgui/edit_precompiled.h"

#include "base/base_include_glm.h"

#include "editor_imgui/edit_inspector_panel.h"
#include "editor_imgui/edit_gui_factory.h"

#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_transformation_facet.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"

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
            auto Hash = rFactory.CalculateHash(m_pEntity);

            if (rFactory.HasClass(Hash))
            {
                auto Panel = rFactory.GetClass(Hash, m_pEntity);

                Panel->OnGUI();
            }

            auto pTransformationFacet = m_pEntity->GetTransformationFacet();

            Hash = rFactory.CalculateHash(pTransformationFacet);

            if (pTransformationFacet && rFactory.HasClass(Hash))
            {
                auto Panel = rFactory.GetClass(Hash, pTransformationFacet);

                Panel->OnGUI();
            }

            auto pComponentFacet = m_pEntity->GetComponentFacet();

            if (pComponentFacet)
            {
                for (auto& rComponent : pComponentFacet->GetComponents())
                {
                    size_t Hash = rFactory.CalculateHash(rComponent);

                    if (rFactory.HasClass(Hash))
                    {
                        auto Panel = rFactory.GetClass(Hash, &rComponent);

                        Panel->OnGUI();
                    }
                }
            }
        }

        ImGui::End();
    }
} // namespace GUI
} // namespace Edit