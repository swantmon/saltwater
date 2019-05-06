
#pragma once

#include "engine/core/core_plugin_manager.h"

#include "engine/graphic/gfx_shader_manager.h" // To manage GPU Shader
#include "engine/graphic/gfx_texture_manager.h" // To manage GPU Texture (Data in GPU processing) 
#include "engine/graphic/gfx_buffer_manager.h" // To manage GPU Buffer (Memory in GPU) 

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
        //---Program Design Setting---
        bool m_Is_ARKitData, m_Is_TestData_MyMMS; // Select Data Set.

        bool m_Is_imwrite; // Export Image Result by OpenCV?

        void ShowImg(const std::vector<char>& Img_RGBA) const;

        //---ARKit Data---
        float m_FrameResolution;
        glm::ivec2 m_OrigImgSize; // Size of original image -> x = width & y = height

        //---Keyframe---
        FutoGmtCV::FutoImg m_Keyframe_Curt, m_Keyframe_Last; // Only compute 2 frames first.
        bool m_idx_Keyf_Curt, m_idx_Keyf_Last; // The status of current & last keyframes.

        std::size_t m_Cdt_Keyf_MaxNum; // Maximal keyframes for calculation once
        float m_Cdt_Keyf_BaseLineL; // Keyframe Selection: BaseLine Condition. Unit is meter.

        //---Rectification---
        FutoGmtCV::FutoImg m_RectImg_Curt, m_RectImg_Last;
        FutoGmtCV::SHomographyTransform m_Homo_Curt, m_Homo_Last;

        FutoGmtCV::CRectification_Planar m_PlanarRectifier; // Implemant Rectification

        //---Stereo Matching---
        std::vector<float> m_Disparity_RectImg;

        int m_DispRange;

        std::string m_StereoMatching_Method;
        std::unique_ptr<sgm::StereoSGM> m_pStereoMatcher_LibSGM;
        cv::Ptr<cv::StereoSGBM> m_pStereoMatcher_cvSGBM;
        cv::Ptr<cv::StereoBM> m_pStereoMatcher_cvBM;
        cv::Ptr<cv::cuda::StereoBM> m_pStereoMatcher_cvBM_cuda;
        cv::Ptr<cv::StereoMatcher> m_pStereoMatcher_cvBP_cuda;
        cv::Ptr<cv::StereoMatcher> m_pStereoMatcher_cvConstBP_cuda;

        //---Disparity to Depth---
        Gfx::CShaderPtr m_Disp2Depth_CSPtr;
        Gfx::CTexturePtr m_Disp_RectImg_TexturePtr;
        Gfx::CTexturePtr m_Depth_RectImg_TexturePtr;
        Gfx::CBufferPtr m_ParaxEq_BufferPtr;

        void imp_Disp2Depth(); // Transform Disparity to Depth in Rectified Image

        //---Depth from Rectified to Original---

    };

} // namespace HW