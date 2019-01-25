
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_component_factory.h" 
#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_data_bloom_component.h"

namespace Dt
{
    REGISTER_GUI(CBloomComponentGUI, CBloomComponent);

    REGISTER_COMPONENT(CBloomComponentGUI);
} // namespace Dt
