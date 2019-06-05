
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

        bool OnGUI()
        {
            bool HasChanged = false;

            HasChanged |= ImGui::TextureSlot("##ALBEDO_TEXTURE", m_ColorTexture); ImGui::SameLine();

            ImGui::SetNextItemWidth(-(ImGui::GetWindowContentRegionWidth() - ImGui::CalcItemWidth()));

            HasChanged |= ImGui::ColorEdit4("Albedo", &m_Color.x);

            // -----------------------------------------------------------------------------

            HasChanged |= ImGui::TextureSlot("##ALPHA_TEXTURE", m_AlphaTexture); ImGui::SameLine();

            ImGui::SetNextItemWidth(-(ImGui::GetWindowContentRegionWidth() - ImGui::CalcItemWidth()));

            HasChanged |= ImGui::DragFloat("Alpha", &m_Color.w, 1.0f / 255.0f, 0.0f, 1.0f, "A:%0.3f");

            // -----------------------------------------------------------------------------

            HasChanged |= ImGui::TextureField("##NORMAL_TEXTURE", "Normal", m_NormalTexture);

            // -----------------------------------------------------------------------------

            HasChanged |= ImGui::TextureSlot("##ROUGHNESS_TEXTURE", m_RoughnessTexture); ImGui::SameLine();

            ImGui::SetNextItemWidth(-(ImGui::GetWindowContentRegionWidth() - ImGui::CalcItemWidth()));

            HasChanged |= ImGui::SliderFloat("Roughness", &m_Roughness, 0.0f, 1.0f);

            // -----------------------------------------------------------------------------
            
            HasChanged |= ImGui::SliderFloat("Reflectance", &m_Reflectance, 0.0f, 1.0f);

            // -----------------------------------------------------------------------------

            HasChanged |= ImGui::TextureSlot("##METALLIC_TEXTURE", m_MetalTexture); ImGui::SameLine();

            ImGui::SetNextItemWidth(-(ImGui::GetWindowContentRegionWidth() - ImGui::CalcItemWidth()));

            HasChanged |= ImGui::SliderFloat("Metallic", &m_MetalMask, 0.0f, 1.0f);

            // -----------------------------------------------------------------------------

            HasChanged |= ImGui::TextureSlot("##BUMP_TEXTURE", m_BumpTexture); ImGui::SameLine();

            ImGui::SetNextItemWidth(-(ImGui::GetWindowContentRegionWidth() - ImGui::CalcItemWidth()));

            HasChanged |= ImGui::SliderFloat("Displacement", &m_Displacement, 0.0f, 1.0f);

            // -----------------------------------------------------------------------------

            HasChanged |= ImGui::TextureField("##AO_TEXTURE", "AO", m_AmbientOcclusionTexture);

            // -----------------------------------------------------------------------------

            HasChanged |= ImGui::SliderFloat("Refraction", &m_RefractionIndex, 0.0f, 1.0f);

            // -----------------------------------------------------------------------------
            
            HasChanged |= ImGui::DragFloat2("Tilling", &m_TilingOffset.x);

            // -----------------------------------------------------------------------------
            
            HasChanged |= ImGui::DragFloat2("Offset", &m_TilingOffset.z);

            return HasChanged;
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Material";
        }

        // -----------------------------------------------------------------------------

        void OnDropAsset(const Edit::CAsset& _rAsset)
        {
            if (_rAsset.GetType() == Edit::CAsset::Material)
            {
                Gfx::MaterialManager::CreateMaterialFromXML(_rAsset.GetPathToFile(), this);
            }
        }
    };
} // namespace Dt
