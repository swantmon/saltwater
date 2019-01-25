
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_component_factory.h" 
#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_script_slam.h"

namespace Scpt
{
    REGISTER_GUI(CSLAMScriptGUI, CSLAMScript);

    REGISTER_COMPONENT(CSLAMScriptGUI);
} // namespace Scpt
