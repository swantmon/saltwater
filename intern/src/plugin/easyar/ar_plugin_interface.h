
#pragma once

#include "engine/core/core_plugin_manager.h"

#include "engine/graphic/gfx_texture.h"

#include "plugin/easyar/ar_camera.h"
#include "plugin/easyar/ar_target.h"

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

        CPluginInterface();
        ~CPluginInterface();

    public:

        void OnStart() override;
        void OnExit() override;
        void OnPause() override;
        void OnResume() override;
        void Update() override;

    public:

        const CCamera& GetCamera();

        CTarget* AcquireNewTarget(const std::string& _rPathToFile);
        void ReleaseTarget(CTarget* _pTarget);

        Gfx::CTexturePtr GetBackgroundTexture();

    private:

        class CInternCamera : public CCamera
        {
        public:

            std::shared_ptr<easyar::CameraDevice> m_Native;

        private:

            friend class CPluginInterface;
        };

        class CInternTarget : public CTarget
        {
        public:

            std::shared_ptr<easyar::ImageTarget> m_Native;

        private:

            friend class CPluginInterface;
        };

    private:

        bool m_IsActive;
        bool m_FirstTargetIsWorldCenter;

        CInternCamera m_Camera;
        std::unordered_map<int, CInternTarget> m_TrackedTargets;

        std::shared_ptr<easyar::Engine> m_Engine;
        std::shared_ptr<easyar::CameraFrameStreamer> m_CameraFrameStreamer;
        std::vector<std::shared_ptr<easyar::ImageTracker>> m_ImageTrackers;

        Gfx::CTexturePtr m_BackgroundTexturePtr;
        glm::ivec2 m_CameraSize;
    };
} // namespace AR