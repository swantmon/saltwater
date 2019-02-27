
#pragma once

#include "engine/core/core_plugin_manager.h"

#include "base/base_include_glm.h"

#include <vector>

#include "plugin\stereo\stereo_photogrammmetry.h"

namespace Stereo
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

        void EventHook();

    public:

        void SetIntrinsics(const glm::vec2& _rFocalLength, const glm::vec2& _rFocalPoint, const glm::ivec2& _rImageSize);

        // CPU

        std::vector<char> GetLatestDepthImageCPU() const;
        void OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4& _Transform);

        // GPU

        Gfx::CTexturePtr GetLatestDepthImageGPU() const;
        void OnFrameGPU(Gfx::CTexturePtr _RGBImage, const glm::mat4& _Transform);

    private:

        glm::vec2 m_FocalLength;
        glm::vec2 m_FocalPoint;
        glm::ivec2 m_ImageSize;

        std::vector<Photogrammmetry> Img_RGB;
        
    private:
        
        void ShowImg(const std::vector<char>& Img_RGBA) const;
    };
} // namespace HW