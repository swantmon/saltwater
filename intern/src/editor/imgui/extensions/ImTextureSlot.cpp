
#include "editor/edit_precompiled.h"

#include "base/base_crc.h"

#include "editor/edit_asset_helper.h"

#include "editor/imgui/imgui_internal.h"
#include "editor/imgui/extensions/ImTextureSlot.h"

#include "engine/graphic/gfx_texture_manager.h"

namespace ImGui
{
    bool TextureSlot(const char* _pID, std::string& _rTexture, const ImVec4& DefaultColor, const ImVec2& Size, const ImVec2& TooltipSize, ImGuiDragDropFlags _ImGuiDragDropFlags)
    {
        auto GetHash = [](const std::string & _rText)
        {
            auto NumberOfBytes = static_cast<unsigned int>(_rText.length());
            auto pData = static_cast<const void*>(_rText.c_str());

            return Base::CRC32(pData, NumberOfBytes);
        };

        ImGuiWindow* _pWindow = GetCurrentWindow();

        if (_pWindow->SkipItems) return false;

        ImGuiID GuiID = _pWindow->GetID(_pID);

        // -----------------------------------------------------------------------------
        // Item
        // -----------------------------------------------------------------------------
        auto GfxImagePtr = Gfx::TextureManager::GetTextureByHash(GetHash(_rTexture));

        if (_rTexture.length() > 0 && GfxImagePtr != nullptr)
        {
            Image((void*)(intptr_t)GfxImagePtr->GetNativeHandle(), Size);

            ImGuiContext& rG = *GImGui;

            float ButtonRadius = rG.FontSize * 0.5f;
            ImVec2 ButtonCenter = ImVec2(ImMin(_pWindow->DC.LastItemRect.Max.x, _pWindow->ClipRect.Max.x) - rG.Style.FramePadding.x - ButtonRadius, _pWindow->DC.LastItemRect.GetCenter().y);

            if (ImGui::IsItemHovered())
            {
                BeginTooltip();
                Image((void*)(intptr_t)GfxImagePtr->GetNativeHandle(), TooltipSize);
                EndTooltip();
            }

            if (CloseButton(_pWindow->GetID((void*)((intptr_t)GuiID + 1)), ButtonCenter, ButtonRadius))
            {
                _rTexture.clear();
            }
        }
        else
        {
            ColorButton(_pID, DefaultColor, ImGuiColorEditFlags_NoTooltip, Size);
        }

        // -----------------------------------------------------------------------------
        // Drag & drop
        // -----------------------------------------------------------------------------
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

    bool TextureField(const char* _pID, const char* _pLabel, std::string& _rTexture, const char* _pNoTexture, const ImVec4& _DefaultColor, const ImVec2& _Size, const ImVec2& _TooltipSize, ImGuiDragDropFlags _ImGuiDragDropFlags)
    {
        ImGuiWindow* _pWindow = GetCurrentWindow();

        if (_pWindow->SkipItems) return false;

        // -----------------------------------------------------------------------------

        bool Result = TextureSlot(_pID, _rTexture, _DefaultColor, _Size, _TooltipSize, _ImGuiDragDropFlags); SameLine();

        auto pText = (char*)_pNoTexture;

        if (_rTexture.length() > 0) pText = (char*)_rTexture.c_str();

        SetNextItemWidth(-(GetWindowContentRegionWidth() - CalcItemWidth()));

        InputText(_pID, pText, strlen(pText), ImGuiInputTextFlags_ReadOnly); SameLine();

        Text(_pLabel);

        return Result;
    }
} // namespace ImGui