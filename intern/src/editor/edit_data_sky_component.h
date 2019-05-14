
#pragma once

#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_sky_component.h"

#include "editor/imgui/imgui.h"

#include "editor/imgui/extensions/ImTextureSlot.h"

#include "engine/graphic/gfx_texture_manager.h"

namespace Dt
{
    class CSkyComponentGUI : public CSkyComponent
    {
    public:

        void OnGUI()
        {
            // -----------------------------------------------------------------------------
            // Refresh mode
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Static", "Dynamic" };

                int Index = static_cast<int>(GetRefreshMode());

                ImGui::Combo("Refresh Mode", &Index, Text, 2);

                SetRefreshMode(static_cast<ERefreshMode>(Index));
            }

            // -----------------------------------------------------------------------------
            // Type
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Procedural", "Panorama", "Cubemap" };

                int Index = static_cast<int>(GetType());

                ImGui::Combo("Type", &Index, Text, 3);

                SetType(static_cast<EType>(Index));
            }

            // -----------------------------------------------------------------------------
            // Quality
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "64", "128", "256", "512", "1024", "2048" };

                int Index = static_cast<int>(GetQuality());

                ImGui::Combo("Quality", &Index, Text, 6);

                SetQuality(static_cast<EQuality>(Index));
            }

            // -----------------------------------------------------------------------------
            // Texture
            // -----------------------------------------------------------------------------
            ImGui::TextureField("##SKY_TEXTURE", "Texture", m_Texture);

            // -----------------------------------------------------------------------------
            // Intensity
            // -----------------------------------------------------------------------------
            ImGui::DragFloat("Intensity", &m_Intensity);
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Sky";
        }

        // -----------------------------------------------------------------------------

        void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CSkyComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CSkyComponent::DirtyCreate);
        }
    };
} // namespace Dt
