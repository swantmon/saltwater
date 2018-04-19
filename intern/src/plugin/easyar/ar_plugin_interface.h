
#pragma once

#include "engine/core/core_plugin_manager.h"

#include "easyar/camera.hpp"
#include "easyar/engine.hpp"
#include "easyar/framestreamer.hpp"

#include <memory>

namespace HW
{
    class CPluginInterface : public Core::IPlugin
    {
    public:

        void OnStart() override;
        void OnExit() override;
        void OnPause() override;
        void OnResume() override;
        void Update() override;

    private:

        std::shared_ptr<easyar::Engine> m_Engine;
        std::shared_ptr<easyar::CameraDevice> m_Camera;
        std::shared_ptr<easyar::CameraFrameStreamer> m_CameraFrameStreamer;
    };
} // namespace HW