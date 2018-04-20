
#pragma once

#include "engine/core/core_plugin_manager.h"

#include "engine/graphic/gfx_texture.h"

#include "easyar/camera.hpp"
#include "easyar/engine.hpp"
#include "easyar/framestreamer.hpp"

#include <unordered_map>
#include <memory>
#include <vector>

namespace AR
{
    class CPluginInterface : public Core::IPlugin
    {
    public:

        void OnStart() override;
        void OnExit() override;
        void OnPause() override;
        void OnResume() override;
        void Update() override;

    public:

        Gfx::CTexturePtr GetBackgroundTexture();

    private:

        std::shared_ptr<easyar::Engine> m_Engine;
        std::shared_ptr<easyar::CameraDevice> m_Camera;
        std::shared_ptr<easyar::CameraFrameStreamer> m_CameraFrameStreamer;
        std::vector<std::shared_ptr<easyar::ImageTracker>> m_ImageTrackers;
        std::unordered_map<int, std::shared_ptr<easyar::ImageTarget>> m_TrackedTargets;

        Gfx::CTexturePtr m_BackgroundTexturePtr;
        glm::ivec2 m_CameraSize;
    };
} // namespace AR