
#include "editor/edit_precompiled.h"

#include "editor/edit_component_factory.h" 
#include "editor/edit_gui_factory.h"
#include "editor/edit_script_camera_control.h"

namespace Scpt
{
    REGISTER_GUI(CCameraControlScriptGUI, CCameraControlScript);

    REGISTER_COMPONENT(CCameraControlScriptGUI);
} // namespace Scpt
