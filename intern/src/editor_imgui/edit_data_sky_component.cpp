
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_component_factory.h" 
#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_data_sky_component.h"

namespace Dt
{
    REGISTER_GUI(CSkyComponentGUI, CSkyComponent);

    REGISTER_COMPONENT(CSkyComponentGUI);
} // namespace Dt
