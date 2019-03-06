
#pragma once

#include "engine/core/core_plugin_manager.h"

#include "base/base_include_glm.h"

#include <vector>

#include "plugin\stereo\stereo_fu_fotogmtcv.h"

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

        glm::mat3 m_Camera_mtx;
        glm::ivec2 m_ImageSize;

        std::vector<Fu_FotoGmtCV> SeqImg_RGB;
        int ImgMaxCal = 2; // Maximal images for calculation once
        
    private:
        void glm2cv(cv::Mat* cvmat, const glm::mat3& glmmat);
        void glm2cv(cv::Mat* cvmat, const glm::vec3& glmmat);
        void ShowImg(const std::vector<char>& Img_RGBA) const;
    };
} // namespace HW