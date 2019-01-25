
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_component_factory.h" 
#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_data_post_aa_component.h"

namespace Dt
{
    REGISTER_GUI(CPostAAComponentGUI, CPostAAComponent);

    REGISTER_COMPONENT(CPostAAComponentGUI);
} // namespace Dt
