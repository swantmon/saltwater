
#include "plugin/arcore/mr_precompiled.h"

#include "engine/engine.h"

#include "plugin/arcore/mr_camera.h"
#include "plugin/arcore/mr_control_manager.h"
#include "plugin/arcore/mr_plugin_interface.h"
#include "plugin/arcore/mr_plane_renderer.h"
#include "plugin/arcore/mr_point_renderer.h"

CORE_PLUGIN_INFO(MR::CPluginInterface, "ArCore", "1.0", "This plugin use ARCore 1.10.0 to enable augmented reality features.")

namespace MR
{
    void CPluginInterface::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Hooks
        // -----------------------------------------------------------------------------
        m_EventDelegateHandle = Engine::RegisterEventHandler(Engine::EEvent::Engine_OnUpdate, std::bind(&CPluginInterface::Gfx_OnUpdate, this));

        // -----------------------------------------------------------------------------
        // Manager and renderer
        // -----------------------------------------------------------------------------
        MR::ControlManager::OnStart();
        MR::PointRenderer::OnStart();
        MR::PlaneRenderer::OnStart();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        m_EventDelegateHandle = nullptr;

        MR::ControlManager::OnExit();
        MR::PointRenderer::OnExit();
        MR::PlaneRenderer::OnExit();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
        MR::ControlManager::Update();
        MR::PointRenderer::Update();
        MR::PlaneRenderer::Update();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnPause()
    {
        MR::ControlManager::OnPause();
        MR::PointRenderer::OnPause();
        MR::PlaneRenderer::OnPause();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
        MR::ControlManager::OnResume();
        MR::PointRenderer::OnResume();
        MR::PlaneRenderer::OnResume();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Gfx_OnUpdate()
    {
        MR::ControlManager::UpdateBackground();
    }
} // namespace MR