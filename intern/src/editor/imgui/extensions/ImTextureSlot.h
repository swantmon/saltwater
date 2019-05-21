#pragma once

namespace ImGui
{
    bool TextureSlot(const char* _pID, std::string& _rTexture, const ImVec4& DefaultColor = ImVec4(0.2f, 0.2f, 0.2f, 0.0f), const ImVec2& Size = ImVec2(20.0f, 20.0f), const ImVec2& TooltipSize = ImVec2(128.0f, 128.0f), ImGuiDragDropFlags _ImGuiDragDropFlags = 0);

    bool TextureField(const char* _pID, const char* _pLabel, std::string& _rTexture, const char* _pNoTexture = "No texture set.", const ImVec4& DefaultColor = ImVec4(0.2f, 0.2f, 0.2f, 0.0f), const ImVec2& Size = ImVec2(20.0f, 20.0f), const ImVec2& TooltipSize = ImVec2(128.0f, 128.0f), ImGuiDragDropFlags _ImGuiDragDropFlags = 0);
} // namespace ImGui