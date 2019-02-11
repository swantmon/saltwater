
#include "plugin/arcore/mr_precompiled.h"

#include "engine/engine.h"

#include "plugin/arcore/mr_camera.h"
#include "plugin/arcore/mr_control_manager.h"
#include "plugin/arcore/mr_plugin_interface.h"

CORE_PLUGIN_INFO(MR::CPluginInterface, "ArCore", "1.6.0", "This plugin use ARCore 1.6.0 to enable augmented reality features.")

namespace MR
{
    void CPluginInterface::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Hooks
        // -----------------------------------------------------------------------------
        m_EventDelegateHandle = Engine::RegisterEventHandler(Engine::EEvent::Engine_OnUpdate, std::bind(&CPluginInterface::Gfx_OnUpdate, this));

        // -----------------------------------------------------------------------------
        // Control manager
        // -----------------------------------------------------------------------------
        MR::ControlManager::OnStart();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        m_EventDelegateHandle = nullptr;
        MR::ControlManager::OnExit();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnPause()
    {
        MR::ControlManager::OnPause();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
        MR::ControlManager::OnResume();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Gfx_OnUpdate()
    {
        MR::ControlManager::Update();
    }
} // namespace MR