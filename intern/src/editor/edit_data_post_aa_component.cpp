
#include "editor/edit_precompiled.h"

#include "editor/edit_component_factory.h" 
#include "editor/edit_gui_factory.h"
#include "editor/edit_data_post_aa_component.h"

namespace Dt
{
    REGISTER_GUI(CPostAAComponentGUI, CPostAAComponent);

    REGISTER_COMPONENT(CPostAAComponentGUI);
} // namespace Dt
