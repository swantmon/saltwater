
#pragma once

//---Engine---
#include "engine/core/core_plugin_manager.h"

#include "base/base_delegate.h" // Return Result back to Engine

//---GLSL for GPU Parallel Programming---
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_buffer_manager.h"

//---FutoGmtCV---
#include "plugin\stereo\futogcv_futoimg.h"
#include "plugin\stereo\futogcv_rectification_planar.h"
#include "plugin\stereo\futogcv_upsampling_fgi.h"

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

    //===== Engine =====

    public:

        void OnStart() override;
        void OnExit() override;
        void OnPause() override;
        void OnResume() override;
        void Update() override;

    public:

        void EventHook();

    //===== plugin_slam =====

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

    //===== plugin_stereo =====

    private:

        //===== 00. Input Data =====

        glm::ivec3 m_OrigImgSize; // Width, Height, Channel

        Gfx::CTexturePtr m_OrigImg_TexturePtr; // Temporary TexturePtr for each input frame.

        //===== 00. Select Keyframe =====

        FutoGCV::SFutoImg m_OrigKeyframe_Curt, m_OrigKeyframe_Last; // Original images of keyframes.

        bool m_KeyfStatus = false; // The status of current keyframe.

        float m_KeyfCondition_BaseLineL; // BaseLine Condition. Unit is meter.

        int m_KeyfID = 0;

        //===== 01. Epipolarization =====

        FutoGCV::CPlanarRectification m_Rectifier_Planar; 

        FutoGCV::SFutoImg m_EpiKeyframe_Curt, m_EpiKeyframe_Last;

        Gfx::CBufferPtr m_Homography_Curt_BufferPtr, m_Homography_Last_BufferPtr;

        //===== 02. Stereo Matching =====

        int m_DisparityRange;

        std::unique_ptr<sgm::StereoSGM> m_pStereoMatcher_LibSGM;

        Gfx::CTexturePtr m_EpiDisparity_TexturePtr;

        //===== 03. Disparity to Depth =====

        void imp_Disp2Depth();

        Gfx::CShaderPtr m_Disp2Depth_CSPtr;

        Gfx::CTexturePtr m_EpiDepth_TexturePtr;

        Gfx::CBufferPtr m_ParaxEq_BufferPtr;

        //===== EpiDepth to OrigDepth =====

        void imp_Depth_Epi2Orig();

        Gfx::CShaderPtr m_Depth_Rect2Orig_CSPtr;

        Gfx::CTexturePtr m_OrigDepth_TexturePtr;



        //--- Output Result---
        CStereoDelegate m_Delegate; // Return results to plugin_slam.

        bool m_IsExport_OrigImg;
        void export_OrigImg();

        bool m_IsExport_EpiImg;
        void export_RectImg();

        bool m_IsExport_Depth;
        void export_Depth();

    // *** OLD ***
    private:

        //---03 Disparity Up-Sampling---
        void UpSampling();
        Gfx::CTexturePtr m_Disp_HR_BiLinear_TexturePtr;

        FutoGCV::CFGI m_FGI_UpSampler;

        //---03 Disparity to Depth in Rectified Current Image---

        Gfx::CShaderPtr m_UpSampling_BiLinear_CSPtr;
        Gfx::CTexturePtr m_Disp_LR_TexturePtr;

        //---04 Depth from Rectified to Original Current Image---

        std::vector<char> m_DepthImg_Orig; // Horizontal flip for reconstruction in plugin_slam.

        cv::Ptr<cv::ximgproc::FastBilateralSolverFilter> m_pFilter_cvFGS;

        cv::Ptr<cv::ximgproc::FastGlobalSmootherFilter> m_pSmoother_cvFGS;

        //---05 Compare Depth between plugin_stereo & Sensor---
        std::vector<uint16_t> m_DepthImg_Sensor;

        bool m_Is_CompareDepth;
        void cmp_Depth(); 

        Gfx::CShaderPtr m_Compare_Depth_CSPtr;
        Gfx::CTexturePtr m_OrigDepth_Sensor_TexturePtr;
        Gfx::CTexturePtr m_OrigDepth_Diff_TexturePtr;

    };

} // namespace Stereo