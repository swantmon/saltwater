
#include "editor/edit_precompiled.h"

#include "editor/edit_component_factory.h" 
#include "editor/edit_gui_factory.h"
#include "editor/edit_script_slam.h"

namespace Scpt
{
    REGISTER_GUI(CSLAMScriptGUI, CSLAMScript);

    REGISTER_COMPONENT(CSLAMScriptGUI);
} // namespace Scpt
