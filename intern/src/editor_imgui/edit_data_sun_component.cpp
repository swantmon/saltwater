
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_component_factory.h" 
#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_data_sun_component.h"

namespace Dt
{
    REGISTER_GUI(CSunComponentGUI, CSunComponent);

    REGISTER_COMPONENT(CSunComponentGUI);
} // namespace Dt
