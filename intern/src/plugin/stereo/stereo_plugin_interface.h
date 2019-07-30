
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


namespace FutoGCV
{
    struct SFutoImg
    {
        //---Member---
        Gfx::CTexturePtr m_Img_TexturePtr; // 2D Texture

        glm::ivec3 m_ImgSize; // Width, Height, Channel

        glm::mat3 m_Camera; // Interior Orientations of Camera.
        glm::mat3 m_Rotation; // Rotation from World to Camera.
        glm::vec3 m_Position; // Position of Camera in World.
        glm::mat4x3 m_PPM; // Perspective Projection Matrix

        //---Constructors & Destructore---
        SFutoImg(Gfx::CTexturePtr _ImgData, const glm::ivec3& _ImgSize, 
                const glm::mat3& _Camera, const glm::mat3& _Rotation, const glm::vec3& _Position)
            : m_Img_TexturePtr(nullptr),
              m_ImgSize(_ImgSize),
              m_Camera(_Camera),
              m_Rotation(_Rotation),
              m_Position(_Position)
        {
            m_Img_TexturePtr = _ImgData;

            m_PPM = glm::mat4x3(m_Rotation[0], m_Rotation[1], m_Rotation[2], -m_Rotation * m_Position);
            m_PPM = m_Camera * m_PPM;
        }

        ~SFutoImg()
        {
            m_Img_TexturePtr = nullptr;
        }
    };
} // namespace FutoGCV

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

        //---00 Input Data---
        glm::ivec3 m_OrigImgSize; // Width, Height, Channel

        //---00 Select Keyframe---
        Gfx::CTexturePtr m_OrigImg_TexturePtr; // Temporary TexturePtr for each input frame.
        FutoGCV::SFutoImg m_OrigKeyframe_Curt, m_OrigKeyframe_Last; // Original images of keyframes.

        bool m_IsKeyfExist = false; // The status of current keyframe.

        float m_SelectKeyf_BaseLineL; // BaseLine Condition. Unit is meter.

        int m_KeyfID = 0;

        //---01 Calculate Disparity---
        std::string m_Strategy;
        glm::ivec3 m_EpiImgSize; // Width, Height, Channel

        FutoGCV::CPlanarRectification m_Rectifier_Planar; // Implement planar rectification

        FutoGCV::SFutoImg m_EpiKeyframe_Curt, m_Keyframe_Last; // Epipolar images of keyframes.

        FutoGCV::SHomography m_Homo_Curt, m_Hmom_Last;




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
        Gfx::CTexturePtr m_Disp_HR_BiLinear_TexturePtr;

        FutoGCV::CFGI m_FGI_UpSampler;

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

    };

} // namespace Stereo