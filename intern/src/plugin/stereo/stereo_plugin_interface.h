
#pragma once

#include "engine/core/core_plugin_manager.h"

#include "base/base_include_glm.h"

#include <vector>
#include <memory>

#include "plugin\stereo\stereo_futogmtcv.h"
#include "libsgm.h"

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

        void SetIntrinsics(const glm::vec2 &_rFocalLength, const glm::vec2 &_rFocalPoint, const glm::ivec2 &_rImageSize);

        // CPU

        std::vector<char> GetLatestDepthImageCPU() const;
        void OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4 &_Transform, const glm::mat4 &_Intrinsics, const std::vector<uint16_t> &_rDepthImage);

        // GPU

        Gfx::CTexturePtr GetLatestDepthImageGPU() const;
        void OnFrameGPU(Gfx::CTexturePtr _RGBImage, const glm::mat4 &_Transform);

    private:
        glm::ivec2 m_ImageSize;

		FutoGmtCV Keyframe_Curt, Keyframe_Last; // Only compute 2 frames first. -> In the future, I will modify if it needs to compute more images at once.
		bool Idx_Keyf_Curt, Idx_Keyf_Last;

        std::size_t Cdt_Keyf_MaxNum = 2; // Maximal images for calculation once
        float Cdt_Keyf_BaseLineL = 0.05; // Keyframe Selection: BaseLine Condition. Unit is meter.


        
    private:
        void glm2cv(cv::Mat* cvmat, const glm::mat3& glmmat);
        void glm2cv(cv::Mat* cvmat, const glm::vec3& glmmat);
        void glm2cv(cv::Mat* cvmat, const glm::vec4& glmmat);
        void glm2cv(cv::Mat* cvmat, const glm::mat3x4& glmmat);
        void ShowImg(const std::vector<char>& Img_RGBA) const;
    };
} // namespace HW