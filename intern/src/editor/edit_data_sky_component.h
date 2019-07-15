
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

        bool OnGUI()
        {
            bool HasChanged = false;

            // -----------------------------------------------------------------------------
            // Refresh mode
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Static", "Dynamic" };

                auto Index = static_cast<int>(GetRefreshMode());

                HasChanged |= ImGui::Combo("Refresh Mode", &Index, Text, 2);

                SetRefreshMode(static_cast<ERefreshMode>(Index));
            }

            // -----------------------------------------------------------------------------
            // Type
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Procedural", "Panorama", "Cubemap" };

                auto Index = static_cast<int>(GetType());

                HasChanged |= ImGui::Combo("Type", &Index, Text, 3);

                SetType(static_cast<EType>(Index));
            }

            // -----------------------------------------------------------------------------
            // Quality
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "64", "128", "256", "512", "1024", "2048" };

                auto Index = static_cast<int>(GetQuality());

                HasChanged |= ImGui::Combo("Quality", &Index, Text, 6);

                SetQuality(static_cast<EQuality>(Index));
            }

            // -----------------------------------------------------------------------------
            // Texture
            // -----------------------------------------------------------------------------
            HasChanged |= ImGui::TextureField("##SKY_TEXTURE", "Texture", m_Texture);

            // -----------------------------------------------------------------------------
            // Intensity
            // -----------------------------------------------------------------------------
            HasChanged |= ImGui::DragFloat("Intensity", &m_Intensity);

            return HasChanged;
        }

        // -----------------------------------------------------------------------------

        static void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::CEntityManager::GetInstance().GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CSkyComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CSkyComponent::DirtyCreate);
        }

        // -----------------------------------------------------------------------------

        void OnDropAsset(const Edit::CAsset&)
        {
        }
    };
} // namespace Dt
