
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

        void Start() override
        {
            Engine::LoadPlugin("plugin_slam");

            if (!Core::PluginManager::HasPlugin("SLAM"))
            {
                throw Base::CException(__FILE__, __LINE__, "SLAM plugin could not be loaded");
            }

            OnPluginExit = (PluginCallback)Core::PluginManager::GetPluginFunction("SLAM", "Exit");
            OnPluginUpdate = (PluginCallback)Core::PluginManager::GetPluginFunction("SLAM", "Update");

        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {
            OnPluginExit();
        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            OnPluginUpdate();
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            
        }

    private:

        typedef void(*PluginCallback)(void);

        PluginCallback OnPluginExit;
        PluginCallback OnPluginUpdate;
    };
} // namespace Scpt