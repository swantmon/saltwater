
#include "editor/edit_precompiled.h"

#include "editor/edit_component_factory.h" 
#include "editor/edit_gui_factory.h"
#include "editor/edit_data_light_probe_component.h"

namespace Dt
{
    REGISTER_GUI(CLightProbeComponentGUI, CLightProbeComponent);

    REGISTER_COMPONENT(CLightProbeComponentGUI);
} // namespace Dt
