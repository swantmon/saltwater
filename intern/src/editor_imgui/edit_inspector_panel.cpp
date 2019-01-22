
#include "editor_imgui/edit_precompiled.h"

#include "base/base_include_glm.h"

#include "editor_imgui/edit_inspector_panel.h"

#include "imgui.h"

namespace Edit
{
namespace GUI
{
    CInspectorPanel::CInspectorPanel()
    {

    }

    // -----------------------------------------------------------------------------

    CInspectorPanel::~CInspectorPanel()
    {

    }
        
    // -----------------------------------------------------------------------------

    void CInspectorPanel::Render()
    {
        static bool open = false;

        ImGui::ShowDemoWindow(&open);

        static float f = 0.0f;
        static int counter = 0;
        static glm::vec3 clearColor;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::BeginChild("Hello, universe!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is another useful text.");               // Display some text (you can use a format strings too)

        ImGui::EndChild();

        ImGui::BeginChild("Hello, universe!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &open);      // Edit bools storing our window open/close state

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", &clearColor.x); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
        ImGui::End();
    }
} // namespace GUI
} // namespace Edit