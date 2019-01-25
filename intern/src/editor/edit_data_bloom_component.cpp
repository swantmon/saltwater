
#include "editor/edit_precompiled.h"

#include "editor/edit_component_factory.h" 
#include "editor/edit_gui_factory.h"
#include "editor/edit_data_bloom_component.h"

namespace Dt
{
    REGISTER_GUI(CBloomComponentGUI, CBloomComponent);

    REGISTER_COMPONENT(CBloomComponentGUI);
} // namespace Dt
