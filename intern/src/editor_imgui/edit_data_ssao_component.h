
#pragma once

#include "engine/data/data_ssao_component.h"

#include "editor_imgui/imgui/imgui.h"

namespace Dt
{
    class CSSAOComponentGUI : public CSSAOComponent
    {
    public:

        void OnGUI()
        {
            
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Ambient Occlusion";
        }
    };
} // namespace Dt
