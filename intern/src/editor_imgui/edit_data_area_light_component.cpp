
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_component_factory.h" 
#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_data_area_light_component.h"

namespace Dt
{
    REGISTER_GUI(CAreaLightComponentGUI, CAreaLightComponent);

    REGISTER_COMPONENT(CAreaLightComponentGUI);
} // namespace Dt
