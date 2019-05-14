
#include "editor/edit_precompiled.h"

#include "base/base_crc.h"

#include "editor/edit_asset_helper.h"

#include "editor/imgui/imgui_internal.h"
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
            Image((void*)(intptr_t)GfxImagePtr->GetNativeHandle(), Size);

            if (ImGui::IsItemHovered())
            {
                BeginTooltip();
                Image((void*)(intptr_t)GfxImagePtr->GetNativeHandle(), TooltipSize);
                EndTooltip();
            }
        }
        else
        {
            ColorButton(_pID, DefaultColor, ImGuiColorEditFlags_NoTooltip, Size);
        }

        bool HasNewValue = false;

        if (BeginDragDropTarget())
        {
            if (const ImGuiPayload * _pPayload = AcceptDragDropPayload("ASSETS_DRAGDROP", _ImGuiDragDropFlags))
            {
                auto& DraggedAsset = *static_cast<Edit::CAsset*>(_pPayload->Data);

                if (DraggedAsset.GetType() == Edit::CAsset::Texture)
                {
                    _rTexture = DraggedAsset.GetPathToFile();

                    HasNewValue = true;
                }
            }

            EndDragDropTarget();
        }

        return HasNewValue;
    }

    // -----------------------------------------------------------------------------

    bool TextureField(const char* _pID, const char* _pLabel, std::string& _rTexture, const char* _pNoTexture, ImVec4 _DefaultColor, ImVec2 _Size, ImVec2 _TooltipSize, ImGuiDragDropFlags _ImGuiDragDropFlags)
    {
        bool Result = TextureSlot(_pID, _rTexture, _DefaultColor, _Size, _TooltipSize, _ImGuiDragDropFlags); SameLine();

        auto pText = (char*)_pNoTexture;

        if (_rTexture.length() > 0) pText = (char*)_rTexture.c_str();

        InputText(_pLabel, pText, strlen(pText), ImGuiInputTextFlags_ReadOnly);

        return Result;
    }
} // namespace ImGui