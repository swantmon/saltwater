
#pragma once

//---Engine---
#include "engine/core/core_plugin_manager.h"

#include "base/base_delegate.h" // Return Result back to Engine

//---GLSL for GPU Parallel Programming---
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_buffer_manager.h"

//---FutoGmtCV---
#include "plugin\stereo\FutoGCV_Img.h"
#include "plugin\stereo\FutoGCV_Rectification_Planar.h"
#include "plugin\stereo\FutoGCV_upsampling_fgi.h"

//---Basic Processing---
#include <vector>
#include <memory>

#include "base/base_include_glm.h" // Some warnings appears when directly #include "glm" in Engine

//---Additional Processing---
#include "opencv2/opencv.hpp" 
#include "opencv2/cudastereo.hpp"
#include "opencv2/ximgproc/edge_filter.hpp"

#include "libsgm.h"



namespace Stereo
{
    class CPluginInterface : public Core::IPlugin
    {
    //---Engine---
    public:

        void OnStart() override;
        void OnExit() override;
        void OnPause() override;
        void OnResume() override;
        void Update() override;

    public:

        void EventHook();

    //---plugin_slam---
    public:

        void SetIntrinsics(const glm::vec2 &_rFocalLength, const glm::vec2 &_rFocalPoint, const glm::ivec2 &_rImageSize);

        //---CPU Computation---
        bool GetLatestFrameCPU(std::vector<char>& _ColorImage, std::vector<char>& _rDepthImage, glm::mat4& _rTransform);
        void OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4 &_Transform, const glm::vec2& _FocalLength, const glm::vec2& _FocalPoint, const std::vector<uint16_t> &_rDepthImage);

        //---GPU Computation---
        Gfx::CTexturePtr GetLatestDepthImageGPU() const;
        void OnFrameGPU(Gfx::CTexturePtr _RGBImage, const glm::mat4 &_Transform);
        
    public:

        using CStereoDelegate = Base::CDelegate<const std::vector<char>&, const std::vector<char>&, const glm::mat4&, const glm::vec2&, const glm::vec2&>;
        CStereoDelegate::HandleType Register(Stereo::CPluginInterface::CStereoDelegate::FunctionType _Function);

    //---plugin_stereo---
    private:
        //---00 Input---
        float m_FrameResolution; // Full=1, Half=0.5.

        glm::ivec2 m_OrigImgSize; // Size of original image -> x = width & y = height

        //---00 Keyframe---
        FutoGCV::CFutoImg m_OrigImg_Curt, m_OrigImg_Last; // Original Image Pair. -> Only compute 2 frames once.

        bool m_Is_KeyFrame; // The status of current keyframe.

        int m_KeyFrameID = 0;

        float m_Cdt_Keyf_BaseLineL; // Keyframe Selection: BaseLine Condition. Unit is meter.

        //---01 Rectification---
        FutoGCV::CFutoImg m_RectImg_Curt, m_RectImg_Last; // Rectified Image Pair.

        FutoGCV::SHomographyTransform m_Homo_Curt, m_Homo_Last; // Homography

        FutoGCV::CPlanarRectification m_Rectifier_Planar; // Implement planar rectification

        bool m_Is_RectSubImg, m_Is_Scaling;
        glm::ivec2 m_RectImgSize_Sub, m_RectImgSize_DownSample;

        //---02 Stereo Matching---
        void imp_StereoMatching();
        void imp_StereoMatching_Tile();
        void imp_StereoMatching_Sub();
        void imp_StereoMatching_Fix();
        void imp_StereoMatching_Scaling(const std::vector<char>& RectImg_Curt_DownSample, const std::vector<char>& RectImg_Last_DownSample);

        std::vector<float> m_DispImg_Rect; // Disparity in Rectified Image => Using float because disparity is pixel or sub-pixel.

        int m_DispRange; // Disparity Searching Range for Stereo Matching

        std::string m_StereoMatching_Method; // Select stereo matching method.

        std::string m_StereoMatching_Mode; // Select calculation mode of stereo matching.

        std::unique_ptr<sgm::StereoSGM> m_pStereoMatcher_LibSGM;
        cv::Ptr<cv::StereoSGBM> m_pStereoMatcher_cvSGBM;
        cv::Ptr<cv::StereoBM> m_pStereoMatcher_cvBM;
        cv::Ptr<cv::cuda::StereoBM> m_pStereoMatcher_cvBM_cuda;
        cv::Ptr<cv::StereoMatcher> m_pStereoMatcher_cvBP_cuda;
        cv::Ptr<cv::StereoMatcher> m_pStereoMatcher_cvConstBP_cuda;

        //---03 Disparity Up-Sampling---
        FutoGCV::CFGI m_FGI_UpSampler;

        void ColorGuidedFGS();

        //---03 Disparity to Depth in Rectified Current Image---
        void imp_Disp2Depth(); 

        Gfx::CShaderPtr m_Disp2Depth_CSPtr;
        Gfx::CTexturePtr m_DispImg_Rect_TexturePtr;
        Gfx::CTexturePtr m_DepthImg_Rect_TexturePtr;
        Gfx::CBufferPtr m_ParaxEq_BufferPtr;

        Gfx::CShaderPtr m_UpSampling_BiLinear_CSPtr;
        Gfx::CTexturePtr m_Disp_LR_TexturePtr;

        //---04 Depth from Rectified to Original Current Image---
        void imp_Depth_Rect2Orig();

        std::vector<char> m_DepthImg_Orig; // Horizontal flip for reconstruction in plugin_slam.

        cv::Ptr<cv::ximgproc::FastBilateralSolverFilter> m_pFilter_cvFGS;

        Gfx::CShaderPtr m_Depth_Rect2Orig_CSPtr;
        Gfx::CTexturePtr m_DepthImg_Orig_TexturePtr;
        Gfx::CBufferPtr m_Homogrampy_BufferPtr;

        cv::Ptr<cv::ximgproc::FastGlobalSmootherFilter> m_pSmoother_cvFGS;

        //---05 Compare Depth between plugin_stereo & Sensor---
        std::vector<uint16_t> m_DepthImg_Sensor;

        bool m_Is_CompareDepth;
        void cmp_Depth(); 

        Gfx::CShaderPtr m_Compare_Depth_CSPtr;
        Gfx::CTexturePtr m_DepthImg_Sensor_TexturePtr;
        Gfx::CTexturePtr m_Depth_Difference_TexturePtr;

        //---06 Return Results---
        CStereoDelegate m_Delegate; // Return results to plugin_slam.

        bool m_Is_ExportOrigImg, m_Is_ExportRectImg, m_Is_ExportDepth; // Export results

        void export_OrigImg();
        void export_RectImg();
        void export_Depth();
    };

} // namespace Stereo