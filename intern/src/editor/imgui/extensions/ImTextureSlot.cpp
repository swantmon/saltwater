
#include "editor/edit_precompiled.h"

#include "base/base_crc.h"

#include "editor/edit_asset_helper.h"

#include "editor/imgui/extensions/ImTextureSlot.h"

#include "engine/graphic/gfx_texture_manager.h"

namespace ImGui
{
    bool TextureSlot(const char* _pID, std::string& _rTexture, ImVec4 DefaultColor, ImVec2 Size, ImVec2 TooltipSize, ImGuiDragDropFlags _ImGuiDragDropFlags)
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

        bool HasNewValue = false;

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload * _pPayload = ImGui::AcceptDragDropPayload("ASSETS_DRAGDROP", _ImGuiDragDropFlags))
            {
                auto& DraggedAsset = *static_cast<Edit::CAsset*>(_pPayload->Data);

                if (DraggedAsset.GetType() == Edit::CAsset::Texture)
                {
                    _rTexture = DraggedAsset.GetPathToFile();

                    HasNewValue = true;
                }
            }

            ImGui::EndDragDropTarget();
        }

        return HasNewValue;
    }

    // -----------------------------------------------------------------------------

    bool TextureField(const char* _pID, const char* _pLabel, std::string& _rTexture, const char* _pNoTexture, ImVec4 DefaultColor, ImVec2 Size, ImVec2 TooltipSize, ImGuiDragDropFlags _ImGuiDragDropFlags)
    {
        bool Result = ImGui::TextureSlot(_pID, _rTexture); ImGui::SameLine();

        char* pText = (char*)_pNoTexture;

        if (_rTexture.length() > 0) pText = (char*)_rTexture.c_str();

        ImGui::InputText(_pLabel, pText, strlen(pText), ImGuiInputTextFlags_ReadOnly);

        return Result;
    }
} // namespace ImGui