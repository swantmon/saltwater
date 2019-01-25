
#pragma once

#include "engine/data/data_area_light_component.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity_manager.h"

#include "editor_imgui/imgui/imgui.h"

namespace Dt
{
    class CAreaLightComponentGUI : public CAreaLightComponent
    {
    public:

        void OnGUI()
        {
            ImGui::ColorEdit3("Color", &m_Color.r);

            static char PathToTexture[255] = {};

            strcpy_s(PathToTexture, m_Texture.c_str());

            if (ImGui::InputText("Name", PathToTexture, 255))
            {
                m_Texture = PathToTexture;
            }

            ImGui::DragFloat("Intensity", &m_Intensity);

            ImGui::DragFloat3("Direction", &m_Direction.x);

            ImGui::DragFloat("Rotation", &m_Rotation);

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
            Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CAreaLightComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CAreaLightComponent::DirtyCreate);
        }
    };
} // namespace Dt
