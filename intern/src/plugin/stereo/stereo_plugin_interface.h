
#pragma once

#include "engine/core/core_plugin_manager.h"

#include "base/base_include_glm.h"

#include <vector>

#include "plugin\stereo\stereo_futogmtcv.h"

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
        void OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4& _Transform, const glm::mat4& _Intrinsics);

        // GPU

        Gfx::CTexturePtr GetLatestDepthImageGPU() const;
        void OnFrameGPU(Gfx::CTexturePtr _RGBImage, const glm::mat4& _Transform);

    private:

        cv::Mat R_1st;
        cv::Mat PC_1st;

        glm::mat3 m_Camera_mtx;
        glm::ivec2 m_ImageSize;
        glm::vec4 m_DisCoeff;

        std::vector<FutoGmtCV> Keyframes;
        std::size_t Cdt_Keyf_MaxNum = 2; // Maximal images for calculation once
        float Cnd_Keyf_BaseLine = 0.05; // Keyframe Selection: BaseLine Condition. Unit is meter.
        
    private:
        void glm2cv(cv::Mat* cvmat, const glm::mat3& glmmat);
        void glm2cv(cv::Mat* cvmat, const glm::vec3& glmmat);
        void glm2cv(cv::Mat* cvmat, const glm::vec4& glmmat);
        void glm2cv(cv::Mat* cvmat, const glm::mat3x4& glmmat);
        void ShowImg(const std::vector<char>& Img_RGBA) const;
    };
} // namespace HW