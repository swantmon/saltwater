
#include "editor/edit_precompiled.h"

#include "editor/edit_component_factory.h" 
#include "editor/edit_gui_factory.h"
#include "editor/edit_data_sky_component.h"

namespace Dt
{
    REGISTER_GUI(CSkyComponentGUI, CSkyComponent);

    REGISTER_COMPONENT(CSkyComponentGUI);
} // namespace Dt
