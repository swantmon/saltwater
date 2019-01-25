
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_component_factory.h" 
#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_data_volume_fog_component.h"

namespace Dt
{
    REGISTER_GUI(CVolumeFogComponentGUI, CVolumeFogComponent);

    REGISTER_COMPONENT(CVolumeFogComponentGUI);
} // namespace Dt
