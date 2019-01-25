
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_component_factory.h"
#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_data_ssao_component.h"

namespace Dt
{
    REGISTER_GUI(CSSAOComponentGUI, CSSAOComponent);

    REGISTER_COMPONENT(CSSAOComponentGUI);
} // namespace Dt
