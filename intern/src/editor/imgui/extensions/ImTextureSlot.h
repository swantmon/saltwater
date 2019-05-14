#pragma once

namespace ImGui
{
    bool TextureSlot(const char* _pID, std::string& _rTexture, ImVec4 DefaultColor = ImVec4(0.2, 0.2, 0.2, 0), ImVec2 Size = ImVec2(20, 20), ImVec2 TooltipSize = ImVec2(128, 128), ImGuiDragDropFlags _ImGuiDragDropFlags = 0);

    bool TextureField(const char* _pID, const char* _pLabel, std::string& _rTexture, const char* _pNoTexture = "No texture set.", ImVec4 DefaultColor = ImVec4(0.2, 0.2, 0.2, 0), ImVec2 Size = ImVec2(20, 20), ImVec2 TooltipSize = ImVec2(128, 128), ImGuiDragDropFlags _ImGuiDragDropFlags = 0);
} // namespace ImGui