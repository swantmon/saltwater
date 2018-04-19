
#include "plugin/easyar/ar_precompiled.h"

#include "base/base_include_glm.h"

#include "engine/core/core_console.h"

#include "plugin/easyar/ar_plugin_interface.h"

#include "easyar/imagetarget.hpp"
#include "easyar/imagetracker.hpp"
#include "easyar/vector.hpp"

CORE_PLUGIN_INFO(HW::CPluginInterface, "EasyAR", "2.2.0", "This plugin enables the possibility to have AR on desktop computer.")

namespace HW
{
    void CPluginInterface::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Engine
        // -----------------------------------------------------------------------------
        m_Engine = std::make_shared<easyar::Engine>();

        m_Engine->initialize("vMBusxFqGilqdUO2ApXVEwDmEAj1uGi6UJ1hrHNUKGm9f2DpzIcsCnXwXESx25AKbGht2CILIs2t1UEAeXFUS5did4IUWlxb2vizoKIfK2i6znxkCWEn6a8uiWmWUPKbYAbWfeo9FXsqu0kFAz4qw2rg6piPrqfdFm6ZUpxlkRZf1Nqr8u90kedBnXORmv0dlDoEm2uZ");

        // -----------------------------------------------------------------------------
        // Camera
        // -----------------------------------------------------------------------------
        m_Camera = std::make_shared<easyar::CameraDevice>();

        m_Camera->setFocusMode(easyar::CameraDeviceFocusMode::Continousauto);

        // -----------------------------------------------------------------------------
        // Frame Streamer
        // -----------------------------------------------------------------------------
        m_CameraFrameStreamer = std::make_shared<easyar::CameraFrameStreamer>();

        m_CameraFrameStreamer->attachCamera(m_Camera);

        // -----------------------------------------------------------------------------
        // Prepare camera
        // -----------------------------------------------------------------------------
        m_Camera->setFocusMode(easyar::CameraDeviceFocusMode::Continousauto);
        m_Camera->setSize(easyar::Vec2I{ { 1280, 720 } });
        m_Camera->open(static_cast<int>(easyar::CameraDeviceType::Default));

        // -----------------------------------------------------------------------------
        // Start everything
        // -----------------------------------------------------------------------------
        m_Camera->start();
        m_CameraFrameStreamer->start();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        m_Camera->stop();
        m_CameraFrameStreamer->stop();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
        auto Frame = m_CameraFrameStreamer->peek();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnPause()
    {
        m_Engine->onPause();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
        m_Engine->onResume();
    }
} // namespace HW

extern "C" CORE_PLUGIN_API_EXPORT void SayHelloWorld()
{
    ENGINE_CONSOLE_INFOV("Hello world!");
}