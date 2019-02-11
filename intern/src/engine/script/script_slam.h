
#pragma once

#include "base/base_compression.h"
#include "base/base_exception.h"
#include "base/base_include_glm.h"

#include "engine/camera/cam_control_manager.h"
#include "engine/camera/cam_editor_control.h"

#include "engine/core/core_program_parameters.h"

#include "engine/core/core_plugin_manager.h"

#include "engine/engine.h"

#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

#include "engine/script/script_script.h"

#include "engine/network/core_network_manager.h"

namespace Scpt
{
    class CSLAMScript : public CScript<CSLAMScript>
    {
    public:

        bool m_IsSelectionEnabled = false;

        void Start() override
        {
            if (!Core::PluginManager::LoadPlugin("SLAM"))
            {
                throw Base::CException(__FILE__, __LINE__, "SLAM plugin could not be loaded");
            }

            InputCallback = (FInputCallback)(Core::PluginManager::GetPluginFunction("SLAM", "OnInput"));

            SetActivateSelection = (FSetActivateSelection)(Core::PluginManager::GetPluginFunction("SLAM", "SetActivateSelection"));
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {
            
        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            SetActivateSelection(m_IsSelectionEnabled);
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            InputCallback(_rEvent);
        }

    private:

        typedef void(*FInputCallback)(const Base::CInputEvent& _rEvent);
        FInputCallback InputCallback;

        typedef void(*FSetActivateSelection)(bool _Flag);
        FSetActivateSelection SetActivateSelection;
    };
} // namespace Scpt