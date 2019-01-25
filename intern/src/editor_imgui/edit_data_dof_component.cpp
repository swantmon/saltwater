
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_component_factory.h" 
#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_data_dof_component.h"

namespace Dt
{
    REGISTER_GUI(CDOFComponentGUI, CDOFComponent);

    REGISTER_COMPONENT(CDOFComponentGUI);
} // namespace Dt
