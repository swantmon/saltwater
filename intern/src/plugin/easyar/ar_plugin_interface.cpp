
#include "plugin/easyar/ar_precompiled.h"

#include "base/base_include_glm.h"

#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"

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
        // Parameters
        // -----------------------------------------------------------------------------
        std::string Key = Core::CProgramParameters::GetInstance().Get("mr:plugin:easyar:key", "<INSERT YOUR EASYAR KEY HERE>");

        easyar::CameraDeviceFocusMode CameraFocusMode = Core::CProgramParameters::GetInstance().Get("mr:camera:focus_mode", easyar::CameraDeviceFocusMode::Normal);

        easyar::CameraDeviceType CameraDeviceType = Core::CProgramParameters::GetInstance().Get("mr:camera:device_type", easyar::CameraDeviceType::Default);

        glm::ivec2 CameraSize = Core::CProgramParameters::GetInstance().Get("mr:camera:size", glm::ivec2(1280, 720));

        // -----------------------------------------------------------------------------
        // Engine
        // -----------------------------------------------------------------------------
        m_Engine = std::make_shared<easyar::Engine>();

        m_Engine->initialize(Key);

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
        m_Camera->setFocusMode(CameraFocusMode);
        m_Camera->setSize(easyar::Vec2I{ { CameraSize[0], CameraSize[1] } });
        m_Camera->open(static_cast<int>(CameraDeviceType));

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