
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_component_factory.h"
#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_data_camera_component.h"

namespace Dt
{
    REGISTER_GUI(CCameraComponentGUI, CCameraComponent);

    REGISTER_COMPONENT(CCameraComponentGUI);
} // namespace Dt
