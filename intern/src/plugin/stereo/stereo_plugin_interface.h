
#pragma once

#include "engine/core/core_plugin_manager.h"

#include "base/base_include_glm.h" // Some warnings appears when directly #include glm 

#include "plugin\stereo\FutoGmtCV_Img.h"
#include "plugin\stereo\FutoGmtCV_Rect_Planar.h"

#include <vector>
#include <memory>

#include "opencv2/opencv.hpp" 
#include "opencv2/cudastereo.hpp"

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

        //---CPU Computation---

        std::vector<char> GetLatestDepthImageCPU() const;
        void OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4 &_Transform, const glm::mat4 &_Intrinsics, const std::vector<uint16_t> &_rDepthImage);

        //---GPU Computation---

        Gfx::CTexturePtr GetLatestDepthImageGPU() const;
        void OnFrameGPU(Gfx::CTexturePtr _RGBImage, const glm::mat4 &_Transform);

    private:
        //---Inputs from plugin_slam---
        float m_FrameResolution;
        glm::ivec2 m_OrigImgSize; // Size of original image -> x = width & y = height

        //---Keyframe---
        FutoGmtCV::FutoImg m_Keyframe_Curt, m_Keyframe_Last; // Only compute 2 frames first. -> In the future, I will modify if it needs to compute more images at once.
        bool m_idx_Keyf_Curt, m_idx_Keyf_Last; // To judge the current & last keyframes are exist or not.

        //---Keyframe Selection---
        std::size_t m_Cdt_Keyf_MaxNum; // Maximal keyframes for calculation once
        float m_Cdt_Keyf_BaseLineL; // Keyframe Selection: BaseLine Condition. Unit is meter.

        //---Rectification---
        glm::ivec2 m_RectImgSize;

        //---Stereo Matching---
        int m_DisparityCount;

        std::unique_ptr<sgm::StereoSGM> m_pStereoMatcher_LibSGM;
        cv::Ptr<cv::StereoSGBM> m_pStereoMatcher_cvSGBM;
        cv::Ptr<cv::StereoBM> m_pStereoMatcher_cvBM;
        cv::Ptr<cv::cuda::StereoBM> m_pStereoMatcher_cvBM_cuda;
        cv::Ptr<cv::StereoMatcher> m_pStereoMatcher_cvBP_cuda;
        cv::Ptr<cv::StereoMatcher> m_pStereoMatcher_cvConstBP_cuda;
        
    private:
        void ShowImg(const std::vector<char>& Img_RGBA) const;
    };
} // namespace HW