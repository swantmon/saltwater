
#pragma once

//---Engine---
#include "engine/core/core_plugin_manager.h"

#include "base/base_delegate.h" // Return Result back to Engine

//---GLSL for GPU Parallel Programming---
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_buffer_manager.h"

//---FutoGmtCV---
#include "plugin\stereo\FutoGmtCV_Img.h"
#include "plugin\stereo\FutoGmtCV_Rect_Planar.h"

//---Basic Processing---
#include <vector>
#include <memory>

#include "base/base_include_glm.h" // Some warnings appears when directly #include "glm" in Engine

//---Additional Processing---
#include "opencv2/opencv.hpp" 
#include "opencv2/cudastereo.hpp"

#include "libsgm.h"



namespace Stereo
{
    class CPluginInterface : public Core::IPlugin
    {
    //---Engine Function---
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
        void OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4 &_Transform, const glm::mat4 &_Intrinsics, const std::vector<uint16_t> &_rDepthImage);

        //---GPU Computation---
        Gfx::CTexturePtr GetLatestDepthImageGPU() const;
        void OnFrameGPU(Gfx::CTexturePtr _RGBImage, const glm::mat4 &_Transform);
        
    public:

        using CStereoDelegate = Base::CDelegate<const std::vector<char>&, const std::vector<char>&, const glm::mat4&>;
        CStereoDelegate::HandleType Register(Stereo::CPluginInterface::CStereoDelegate::FunctionType _Function);

    //---plugin_stereo---
    private:
        //---ARKit Data---
        float m_FrameResolution; // Full=1, Half=0.5.

        glm::ivec2 m_OrigImgSize; // Size of original image -> x = width & y = height

        int m_KeyFrameID = 0;

        //---Keyframe---
        FutoGmtCV::CFutoImg m_OrigImg_Curt, m_OrigImg_Last; // Original Image Pair. -> Only compute 2 frames once.

        bool m_Is_KeyFrame; // The status of current keyframe.

        float m_Cdt_Keyf_BaseLineL; // Keyframe Selection: BaseLine Condition. Unit is meter.

        //---Rectification---
        FutoGmtCV::CFutoImg m_RectImg_Curt, m_RectImg_Last; // Rectified Image Pair.

        FutoGmtCV::SHomographyTransform m_Homo_Curt, m_Homo_Last;

        FutoGmtCV::CPlanarRectification m_Rectifier_Planar; // Implemant Rectification

        //---Stereo Matching---
        void imp_StereoMatching();

        std::vector<float> m_DispImg_Rect; // Disparity in Rectified Image => Using float because disparity is pixel or sub-pixel.

        int m_DispRange; // Disparity Searching Range for Stereo Matching

        std::string m_StereoMatching_Method;
        std::unique_ptr<sgm::StereoSGM> m_pStereoMatcher_LibSGM;
        cv::Ptr<cv::StereoSGBM> m_pStereoMatcher_cvSGBM;
        cv::Ptr<cv::StereoBM> m_pStereoMatcher_cvBM;
        cv::Ptr<cv::cuda::StereoBM> m_pStereoMatcher_cvBM_cuda;
        cv::Ptr<cv::StereoMatcher> m_pStereoMatcher_cvBP_cuda;
        cv::Ptr<cv::StereoMatcher> m_pStereoMatcher_cvConstBP_cuda;

        //---Disparity to Depth---
        void imp_Disp2Depth(); 

        Gfx::CShaderPtr m_Disp2Depth_CSPtr;
        Gfx::CTexturePtr m_Disp_RectImg_TexturePtr;
        Gfx::CTexturePtr m_Depth_RectImg_TexturePtr;
        Gfx::CBufferPtr m_ParaxEq_BufferPtr;

        //---Depth from Rectified to Original---
        void imp_Depth_Rect2Orig();

        std::vector<char> m_Depth_OrigImg; // Horizontal flip for reconstruction in plugin_slam.

        Gfx::CShaderPtr m_Depth_Rect2Orig_CSPtr;
        Gfx::CTexturePtr m_DepthImg_Orig_TexturePtr;
        Gfx::CBufferPtr m_Homogrampy_BufferPtr;

        //---Compare Depth between Stereo Matching & Sensor---
        bool m_Is_CompareDepth;

        void cmp_Depth();

        std::vector<uint16_t> m_DepthImg_Sensor;

        Gfx::CShaderPtr m_Compare_Depth_CSPtr;
        Gfx::CTexturePtr m_DepthImg_Sensor_TexturePtr;
        Gfx::CTexturePtr m_Depth_Difference_TexturePtr;

        //---Return Result---
        CStereoDelegate m_Delegate;

        bool m_Is_ExportResult; 
        void export_Result();
    };

} // namespace Stereo