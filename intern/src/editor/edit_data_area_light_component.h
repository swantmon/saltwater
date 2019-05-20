
#pragma once

#include "engine/data/data_area_light_component.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity_manager.h"

#include "editor/imgui/imgui.h"

#include "editor/imgui/extensions/ImTextureSlot.h"

namespace Dt
{
    class CAreaLightComponentGUI : public CAreaLightComponent
    {
    public:

        void OnGUI()
        {
            ImGui::ColorEdit3("Color", &m_Color.r);

            ImGui::TextureField("##AREA_LIGHT_TEXTURE", "Texture", m_Texture);

            ImGui::DragFloat("Intensity", &m_Intensity, 10.0f);

            ImGui::DragFloat3("Direction", &m_Direction.x);

			float DegreeAngle = glm::degrees(m_Rotation);

            ImGui::DragFloat("Rotation", &DegreeAngle, 0.1f, 0.0f, 0.0f, "%.2f");

			m_Rotation = glm::radians(DegreeAngle);

            ImGui::DragFloat("Width", &m_Width);

            ImGui::DragFloat("Height", &m_Height);

            ImGui::Checkbox("Is Two Sided", &m_IsTwoSided);

            UpdateLightness();
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Area Light";
        }

        // -----------------------------------------------------------------------------

        void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::CEntityManager::GetInstance().GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CAreaLightComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CAreaLightComponent::DirtyCreate);
        }

        // -----------------------------------------------------------------------------

        void OnDropAsset(const Edit::CAsset&)
        {
        }
    };
} // namespace Dt
