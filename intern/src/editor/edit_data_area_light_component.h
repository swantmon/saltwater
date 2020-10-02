
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

        bool OnGUI()
        {
            bool HasChanged = false;

            HasChanged |= ImGui::ColorEdit3("Color", &m_Color.r);

            HasChanged |= ImGui::TextureField("##AREA_LIGHT_TEXTURE", "Texture", m_Texture);

            HasChanged |= ImGui::DragFloat("Intensity", &m_Intensity, 10.0f);

            HasChanged |= ImGui::DragFloat3("Direction", &m_Direction.x);

			float DegreeAngle = glm::degrees(m_Rotation);

            HasChanged |= ImGui::DragFloat("Rotation", &DegreeAngle, 0.1f, 0.0f, 0.0f, "%.2f");

			m_Rotation = glm::radians(DegreeAngle);

            HasChanged |= ImGui::DragFloat("Width", &m_Width);

            HasChanged |= ImGui::DragFloat("Height", &m_Height);

            HasChanged |= ImGui::Checkbox("Is Two Sided", &m_IsTwoSided);

            if (HasChanged) UpdateLightness();

            return HasChanged;
        }

        // -----------------------------------------------------------------------------

        static void OnNewComponent(Dt::CEntity::BID _ID)
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
