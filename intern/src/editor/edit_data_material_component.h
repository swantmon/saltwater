
#pragma once

#include "editor/edit_asset_helper.h"

#include "editor/imgui/imgui.h"

#include "editor/imgui/extensions/ImTextureSlot.h"

#include "engine/data/data_material_component.h"

#include "engine/graphic/gfx_material_manager.h"

namespace Dt
{
    class CMaterialComponentGUI : public CMaterialComponent
    {
    public:

        void OnGUI()
        {
            ImGuiDragDropFlags ImGuiTargetFlags = 0;

            // ImGuiTargetFlags |= ImGuiDragDropFlags_AcceptBeforeDelivery;    // Don't wait until the delivery (release mouse button on a target) to do something
            // ImGuiTargetFlags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect; // Don't display the yellow rectangle

            // -----------------------------------------------------------------------------

            ImGui::BeginChild("MATERIAL_COMPONENT_GUI_CHILD");

            ImGui::TextureSlot("##ALBEDO_TEXTURE", m_ColorTexture); ImGui::SameLine();

            ImGui::ColorEdit4("Albedo", &m_Color.x);

            // -----------------------------------------------------------------------------

            ImGui::TextureSlot("##ALPHA_TEXTURE", m_AlphaTexture); ImGui::SameLine();

            ImGui::DragFloat("Alpha", &m_Color.w, 1.0f / 255.0f, 0.0f, 1.0f, "A:%0.3f");

            // -----------------------------------------------------------------------------

            ImGui::TextureField("##NORMAL_TEXTURE", "Normal", m_AmbientOcclusionTexture);

            // -----------------------------------------------------------------------------

            ImGui::TextureSlot("##ROUGHNESS_TEXTURE", m_RoughnessTexture); ImGui::SameLine();

            ImGui::SliderFloat("Roughness", &m_Roughness, 0.0f, 1.0f);

            // -----------------------------------------------------------------------------

            ImGui::Dummy(ImVec2(20, 20)); ImGui::SameLine();

            ImGui::SliderFloat("Reflectance", &m_Reflectance, 0.0f, 1.0f);

            // -----------------------------------------------------------------------------

            ImGui::TextureSlot("##METALLIC_TEXTURE", m_MetalTexture); ImGui::SameLine();

            ImGui::SliderFloat("Metallic", &m_MetalMask, 0.0f, 1.0f);

            // -----------------------------------------------------------------------------

            ImGui::TextureSlot("##BUMP_TEXTURE", m_BumpTexture); ImGui::SameLine();

            ImGui::SliderFloat("Displacement", &m_Displacement, 0.0f, 1.0f);

            // -----------------------------------------------------------------------------

            ImGui::TextureField("##AO_TEXTURE", "AO", m_AmbientOcclusionTexture);

            // -----------------------------------------------------------------------------

            ImGui::Dummy(ImVec2(20, 20)); ImGui::SameLine();

            ImGui::SliderFloat("Refraction", &m_RefractionIndex, 0.0f, 1.0f);

            // -----------------------------------------------------------------------------

            ImGui::Dummy(ImVec2(20, 20)); ImGui::SameLine();

            ImGui::DragFloat2("Tilling", &m_TilingOffset.x);

            // -----------------------------------------------------------------------------

            ImGui::Dummy(ImVec2(20, 20)); ImGui::SameLine();

            ImGui::DragFloat2("Offset", &m_TilingOffset.z);

            // -----------------------------------------------------------------------------

            ImGui::EndChild();

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* _pPayload = ImGui::AcceptDragDropPayload("ASSETS_DRAGDROP", ImGuiTargetFlags))
                {
                    auto& DraggedAsset = *static_cast<Edit::CAsset*>(_pPayload->Data);

                    if (DraggedAsset.GetType() == Edit::CAsset::Material)
                    {
                        Gfx::MaterialManager::CreateMaterialFromXML(DraggedAsset.GetPathToFile(), this);
                    }
                }

                ImGui::EndDragDropTarget();
            }
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Material";
        }
    };
} // namespace Dt
