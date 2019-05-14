
#pragma once

#include "base/base_crc.h"

#include "editor/edit_asset_helper.h"

#include "editor/imgui/imgui.h"

#include "engine/data/data_material_component.h"

#include "engine/graphic/gfx_material_manager.h"

#include "engine/graphic/gfx_texture_manager.h"

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

            auto ImGuiTexturePlace = [&](const char* _pID, std::string & _rTexture, ImVec4 DefaultColor = ImVec4(0.2, 0.2, 0.2, 0), ImVec2 Size = ImVec2(20, 20), ImVec2 TooltipSize = ImVec2(128, 128))
            {
                auto GetHash = [](const std::string & _rText)
                {
                    auto NumberOfBytes = static_cast<unsigned int>(_rText.length());
                    const auto* pData = static_cast<const void*>(_rText.c_str());

                    return Base::CRC32(pData, NumberOfBytes);
                };

                // -----------------------------------------------------------------------------

                auto GfxImagePtr = Gfx::TextureManager::GetTextureByHash(GetHash(_rTexture));

                if (_rTexture.length() > 0 || GfxImagePtr != nullptr)
                {
                    ImGui::Image((void*)(intptr_t)GfxImagePtr->GetNativeHandle(), Size);

                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        ImGui::Image((void*)(intptr_t)GfxImagePtr->GetNativeHandle(), TooltipSize);
                        ImGui::EndTooltip();
                    }
                }
                else
                {
                    ImGui::ColorButton(_pID, DefaultColor, ImGuiColorEditFlags_NoTooltip, Size);
                }

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload * _pPayload = ImGui::AcceptDragDropPayload("ASSETS_DRAGDROP", ImGuiTargetFlags))
                    {
                        auto& DraggedAsset = *static_cast<Edit::CAsset*>(_pPayload->Data);

                        if (DraggedAsset.GetType() == Edit::CAsset::Texture) _rTexture = DraggedAsset.GetPathToFile();
                    }

                    ImGui::EndDragDropTarget();
                }
            };

            // -----------------------------------------------------------------------------

            ImGui::BeginChild("MATERIAL_COMPONENT_GUI_CHILD");

            ImGuiTexturePlace("##ALBEDO_TEXTURE", m_ColorTexture); ImGui::SameLine();

            ImGui::ColorEdit4("Albedo", &m_Color.x);

            // -----------------------------------------------------------------------------

            ImGuiTexturePlace("##ALPHA_TEXTURE", m_AlphaTexture); ImGui::SameLine();

            ImGui::DragFloat("Alpha", &m_Color.w, 1.0f / 255.0f, 0.0f, 1.0f, "A:%0.3f");

            // -----------------------------------------------------------------------------

            ImGuiTexturePlace("##NORMAL_TEXTURE", m_NormalTexture); ImGui::SameLine();

            char* pText = "No texture set.";

            if (m_NormalTexture.length() > 0) pText = (char*)m_NormalTexture.c_str();

            ImGui::InputText("Normal", pText, strlen(pText), ImGuiInputTextFlags_ReadOnly);

            // -----------------------------------------------------------------------------

            ImGuiTexturePlace("##ROUGHNESS_TEXTURE", m_RoughnessTexture); ImGui::SameLine();

            ImGui::SliderFloat("Roughness", &m_Roughness, 0.0f, 1.0f);

            // -----------------------------------------------------------------------------

            ImGui::Dummy(ImVec2(20, 20)); ImGui::SameLine();

            ImGui::SliderFloat("Reflectance", &m_Reflectance, 0.0f, 1.0f);

            // -----------------------------------------------------------------------------

            ImGuiTexturePlace("##METALLIC_TEXTURE", m_MetalTexture); ImGui::SameLine();

            ImGui::SliderFloat("Metallic", &m_MetalMask, 0.0f, 1.0f);

            // -----------------------------------------------------------------------------

            ImGuiTexturePlace("##BUMP_TEXTURE", m_BumpTexture); ImGui::SameLine();

            ImGui::SliderFloat("Displacement", &m_Displacement, 0.0f, 1.0f);

            // -----------------------------------------------------------------------------

            ImGuiTexturePlace("##AO_TEXTURE", m_AmbientOcclusionTexture); ImGui::SameLine();

            pText = "No texture set.";

            if (m_AmbientOcclusionTexture.length() > 0) pText = (char*)m_AmbientOcclusionTexture.c_str();

            ImGui::InputText("AO", pText, strlen(pText), ImGuiInputTextFlags_ReadOnly);

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
