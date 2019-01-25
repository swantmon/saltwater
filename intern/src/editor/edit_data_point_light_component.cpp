
#include "editor/edit_precompiled.h"

#include "editor/edit_component_factory.h" 
#include "editor/edit_gui_factory.h"
#include "editor/edit_data_point_light_component.h"

namespace Dt
{
    REGISTER_GUI(CPointLightComponentGUI, CPointLightComponent);

    REGISTER_COMPONENT(CPointLightComponentGUI);
} // namespace Dt
