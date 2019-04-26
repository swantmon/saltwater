
#include "plugin/stereo/stereo_precompiled.h"

#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h" // For controlling parameters in Config.
#include "engine/engine.h"
#include "engine/graphic/gfx_texture.h" // For Transmit Texture between CPU & GPU

#include "plugin/stereo/stereo_plugin_interface.h"


CORE_PLUGIN_INFO(Stereo::CPluginInterface, "Stereo Matching", "1.0", "This plugin takes RGB and transformation data and provides 2.5D depth maps")

namespace Stereo
{
    void CPluginInterface::SetIntrinsics(const glm::vec2& _rFocalLength, const glm::vec2& _rFocalPoint, const glm::ivec2& _rImageSize)
    {
        //---For variables which has not been used yet. Use this statement to avoid warning.---
        BASE_UNUSED(_rFocalLength); 
        BASE_UNUSED(_rFocalPoint); 
        //------

        m_OrigImgSize = _rImageSize;
    }

    // -----------------------------------------------------------------------------

    std::vector<char> CPluginInterface::GetLatestDepthImageCPU() const
    {
        return std::vector<char>();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4& _Transform, const glm::mat4& _Intrinsics, const std::vector<uint16_t>& _rDepthImage)
    {
        glm::mat3 Cam_mtx = glm::mat3(_Intrinsics) * m_FrameResolution; // Intrinsic should be modified according to frame resolution.
        Cam_mtx[2].z = 1;

        glm::mat3 Rot_mtx = glm::transpose(glm::mat3(_Transform));// Rotation given by ARKit is Camera2World, but Rotation in Photogrammetry needs World2Camera.

        glm::vec3 PC_vec = glm::vec3(_Transform[3]); // The last column of _Transform given by ARKit is the Position of Camera in World.

        m_ARKImg_RGBA = _rRGBImage; // vector size = m_OrigImgSize.x * m_OrigImgSize.y * 4

        FutoGmtCV::FutoImg frame(m_ARKImg_RGBA, m_OrigImgSize, Cam_mtx, Rot_mtx, PC_vec);
        
        //---Select Keyframe for Computation---
        if (!m_idx_Keyf_Curt) // Current keyframe is empty -> Set current keyframe.
        {
            m_Keyframe_Curt = frame;
            m_idx_Keyf_Curt = true;
        }
        else if (m_idx_Keyf_Curt && !m_idx_Keyf_Last) // Current keyframe exists but Last keyframe is empty -> Set both current & last keyframes.
        {
            glm::vec3 BaseLine = frame.get_PC() - m_Keyframe_Curt.get_PC();
            float BaseLineLength = glm::l2Norm(BaseLine);

            if (BaseLineLength >= m_Cdt_Keyf_BaseLineL) // Select Keyframe: Baseline condition
            {
                m_Keyframe_Last = m_Keyframe_Curt;
                m_Keyframe_Curt = frame;
                m_idx_Keyf_Last = true;
            }
        }
        else // Both current & last keyframes exist. -> Processing -> Free last keyframe.
        {
            //---Test: Show Rectified Images---
            cv::Mat cvOrigImg_Curt(m_OrigImgSize.y, m_OrigImgSize.x, CV_8UC4);
            memcpy(cvOrigImg_Curt.data, m_Keyframe_Curt.get_Img().data(), m_Keyframe_Curt.get_Img().size());
            cv::cvtColor(cvOrigImg_Curt, cvOrigImg_Curt, cv::COLOR_BGRA2RGBA); // Transform to RGB before imshow & imwrite
            cv::imwrite("E:\\Project_ARCHITECT\\OrigImg_Curt.png", cvOrigImg_Curt);

            cv::Mat cvOrigImg_Last(m_OrigImgSize.y, m_OrigImgSize.x, CV_8UC4);
            memcpy(cvOrigImg_Last.data, m_Keyframe_Last.get_Img().data(), m_Keyframe_Last.get_Img().size());
            cv::cvtColor(cvOrigImg_Last, cvOrigImg_Last, cv::COLOR_BGRA2RGBA); // Transform to RGB before imshow & imwrite
            cv::imwrite("E:\\Project_ARCHITECT\\OrigImg_Last.png", cvOrigImg_Last);

            //---Rectification---
            FutoGmtCV::FutoImg RectImg_Curt, RectImg_Last;
            FutoGmtCV::CRectification_Planar PlanarRectifier = FutoGmtCV::CRectification_Planar(m_Keyframe_Curt.get_ImgSize(), m_RectImgSize);

            PlanarRectifier.execute(RectImg_Curt, RectImg_Last, m_Keyframe_Curt, m_Keyframe_Last);

            //---Test: Show Rectified Images---
            cv::Mat cvRectImg_Curt(m_RectImgSize.y, m_RectImgSize.x, CV_8UC4);
            memcpy(cvRectImg_Curt.data, RectImg_Curt.get_Img().data(), RectImg_Curt.get_Img().size());
            cv::cvtColor(cvRectImg_Curt, cvRectImg_Curt, cv::COLOR_BGRA2RGBA); // Transform to RGB before imshow & imwrite
            cv::imwrite("E:\\Project_ARCHITECT\\RectImg_Curt.png", cvRectImg_Curt);

            cv::Mat cvRectImg_Last(m_RectImgSize.y, m_RectImgSize.x, CV_8UC4);
            memcpy(cvRectImg_Last.data, RectImg_Last.get_Img().data(), RectImg_Last.get_Img().size());
            cv::cvtColor(cvRectImg_Last, cvRectImg_Last, cv::COLOR_BGRA2RGBA); // Transform to RGB before imshow & imwrite
            cv::imwrite("E:\\Project_ARCHITECT\\RectImg_Last.png", cvRectImg_Last);

            

            //===Verify by Test Data===
            /*
               cv::Mat TestInputL = cv::imread("E:\\Project_ARCHITECT\\cvSGBM_Test Data\\01-002570.jpg");
               cv::cvtColor(TestInputL, TestInputL, cv::COLOR_BGRA2GRAY);
               cv::Mat TestInputR = cv::imread("E:\\Project_ARCHITECT\\cvSGBM_Test Data\\02-002570.jpg");
               cv::cvtColor(TestInputR, TestInputR, cv::COLOR_BGRA2GRAY);

               cv::Mat K_L = cv::Mat::zeros(3, 3, CV_32F);
               K_L.at<float>(0, 0) = 1280.465;
               K_L.at<float>(1, 1) = 1280.465;
               K_L.at<float>(0, 2) = 712.961;
               K_L.at<float>(1, 2) = 515.829;
               K_L.at<float>(2, 2) = 1;
               cv::Mat K_R = cv::Mat::zeros(3, 3, CV_32F);
               K_R.at<float>(0, 0) = 1281.566;
               K_R.at<float>(1, 1) = 1281.566;
               K_R.at<float>(0, 2) = 698.496;
               K_R.at<float>(1, 2) = 511.008;
               K_R.at<float>(2, 2) = 1;
               cv::Mat PC_L = cv::Mat::zeros(3, 1, CV_32F);
               cv::Mat PC_R = cv::Mat::zeros(3, 1, CV_32F);
               PC_R.at<float>(0, 0) = 1.630;
               PC_R.at<float>(1, 0) = 0.016;
               PC_R.at<float>(2, 0) = -0.192;
               cv::Mat R_L = cv::Mat::eye(3, 3, CV_32F);
               cv::Mat R_R = cv::Mat::eye(3, 3, CV_32F);
               R_R.at<float>(0, 0) = 0.9999;
               R_R.at<float>(0, 1) = 0.0085;
               R_R.at<float>(0, 2) = -0.0132;
               R_R.at<float>(1, 0) = -0.0084;
               R_R.at<float>(1, 1) = 0.9999;
               R_R.at<float>(1, 2) = 0.0123;
               R_R.at<float>(2, 0) = 0.0132;
               R_R.at<float>(2, 1) = -0.0122;
               R_R.at<float>(2, 2) = 0.9999;

               FutoGmtCV::FutoImg TestImgL = FutoGmtCV(TestInputL, K_L, R_L, PC_L);
               FutoGmtCV::FutoImg TestImgR = FutoGmtCV(TestInputR, K_R, R_R, PC_R);

               PlanarRectifier.execute(RectImg_Curt, RectImg_Last, TestImgL, TestImgR);
               */

            //===== OLD =====
            /*
            //---Stereo Matching---
            cv::Mat DispImg_Rect(m_RectImgSize.y, m_RectImgSize.x, CV_8UC1);
            //Keyframe_Curt.imp_cvSGBM(DispImg_Rect, RectImg_Curt.get_Img(), RectImg_Last.get_Img());
            assert(RectImg_Curt.get_Img().type() == CV_8U); // If the condition is false, make alert. => Useful for debugging~~~!!!
            assert(RectImg_Last.get_Img().type() == CV_8U);
            assert(DispImg_Rect.type() == CV_8U);

            imshow("Left", RectImg_Curt.get_Img());
            imshow("Right", RectImg_Last.get_Img());

            cv::waitKey(0);

            m_pStereoMatcherCUDA->execute(RectImg_Curt.get_Img().data, RectImg_Last.get_Img().data, DispImg_Rect.data);

            imshow("Left", RectImg_Curt.get_Img());
            imshow("Right", RectImg_Last.get_Img());
            imshow("Disp", DispImg_Rect);

            cv::waitKey(0);

            //---test SGBM in OpenCV---
            
                    cv::Mat TestImgL = cv::imread("C:\\saltwater\\intern\\src\\plugin\\stereo\\Test\\im2.png");
                    cv::Mat TestImgR = cv::imread("C:\\saltwater\\intern\\src\\plugin\\stereo\\Test\\im6.png");
                    cv::imshow("TestImgL", TestImgL);
                    cv::imshow("TestImgR", TestImgR);
                    iter->imp_cvSGBM(DispImg_Curt, TestImgL, TestImgR);
            
            //---

                    //DispImg_Rect.convertTo(DispImg_Rect, CV_32F, 1.0 / 16); // Disparity Image is in 16-bit -> Divide by 16 to get real Disparity.
            cv::Mat DispImg_Orig(m_Keyframe_Curt.get_Img().size(), CV_32F);
            cv::remap(DispImg_Rect, DispImg_Orig, Orig2Rect_Curt_x, Orig2Rect_Curt_y, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT); // !!! Warning: Using interpolation may cause additional errors !!!

            //---Show Disparity Image generated from Stereo Matching---
            cv::Mat DispImg_Rect_8U(DispImg_Rect.size(), CV_8UC1);
            cv::normalize(DispImg_Rect, DispImg_Rect_8U, 0, 255, cv::NORM_MINMAX, CV_8UC1);
            cv::imwrite("E:\\Project_ARCHITECT\\Disp_Rect.png", DispImg_Rect_8U);

            cv::Mat DispImg_Orig_8U(DispImg_Orig.size(), CV_8UC1);
            cv::normalize(DispImg_Orig, DispImg_Orig_8U, 0, 255, cv::NORM_MINMAX, CV_8UC1);
            cv::imwrite("E:\\Project_ARCHITECT\\Disp_Orig.png", DispImg_Orig_8U);

            //---Transform Disparity to Depth: Using Parallax Equation---
            cv::Mat DepthImg_Rect = cv::Mat::zeros(DispImg_Rect.size(), CV_32F);

            for (int x_idx = 0; x_idx < DepthImg_Rect.size().width; x_idx++)
            {
                for (int y_idx = 0; y_idx < DepthImg_Rect.size().height; y_idx++)
                {
                    DepthImg_Rect.ptr<float>(y_idx)[x_idx] = RectImg_Curt.get_Cam().ptr<float>(0)[0] * BaseLineLength / DispImg_Rect.ptr<float>(y_idx)[x_idx];
                    float f = RectImg_Curt.get_Cam().ptr<float>(0)[0];
                    float Disparity = DispImg_Rect.ptr<float>(y_idx)[x_idx];
                    float Depth = DepthImg_Rect.ptr<float>(y_idx)[x_idx];
                    DepthImg_Rect.ptr<float>(y_idx)[x_idx] *= 1000; // Unit = mm
                }
            }

            cv::Mat DepthImg_Orig = cv::Mat::zeros(DispImg_Orig.size(), CV_32F);
            cv::remap(DepthImg_Rect, DepthImg_Orig, Orig2Rect_Curt_x, Orig2Rect_Curt_y, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT); // !!! Warning: Using interpolation may cause additional errors !!!
            //---Show Depth in pixel---

            float Depth = DepthImg_Orig.ptr<float>(166)[192];

            //---

            //---Show Depth Image---
            cv::Mat DepthImg_Rect_8U(DepthImg_Rect.size(), CV_8UC1);
            cv::normalize(DepthImg_Rect, DepthImg_Rect_8U, 0, 255, cv::NORM_MINMAX, CV_8UC1);
            cv::imwrite("E:\\Project_ARCHITECT\\Depth_Rect.png", DepthImg_Rect_8U);
            cv::Mat DepthImg_Orig_8U(DepthImg_Orig.size(), CV_8UC1);
            cv::normalize(DepthImg_Orig, DepthImg_Orig_8U, 0, 255, cv::NORM_MINMAX, CV_8UC1);
            cv::imwrite("E:\\Project_ARCHITECT\\Depth_Orig.png", DepthImg_Orig_8U);

            cv::Mat DepthImg_Orig_Sensor(cv::Size(m_OrigImgSize.x, m_OrigImgSize.y), CV_16UC1); // 2D Matrix(x*y) with (16-bit unsigned character & 1 Channel)
            memcpy(DepthImg_Orig_Sensor.data, _rDepthImage.data(), _rDepthImage.size() * sizeof(_rDepthImage[0]));
            cv::Mat DepthImg_Orig_Sensor_8U = cv::Mat(DepthImg_Orig_Sensor.size(), CV_8UC1);
            cv::normalize(DepthImg_Orig_Sensor, DepthImg_Orig_Sensor_8U, 0, 255, cv::NORM_MINMAX, CV_8UC1);
            cv::imwrite("E:\\Project_ARCHITECT\\Depth_Orig_Sensor.png", DepthImg_Orig_Sensor_8U);
            */

            //---Finish Processing -> Free last keyframe---
            m_idx_Keyf_Last = false;

            m_Cdt_Keyf_BaseLineL = 1000000000; //---Temp Setting: Only calculate once, because it takes too much time calculating in CPU...
        }
        
        // Optional for internal check
        CPluginInterface::ShowImg(_rRGBImage); // Showing image for visual checking -> Modify to control in editor.config
        
    }

    // -----------------------------------------------------------------------------

    Gfx::CTexturePtr CPluginInterface::GetLatestDepthImageGPU() const
    {
        return nullptr;
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnFrameGPU(Gfx::CTexturePtr _RGBImage, const glm::mat4& _Transform)
    {
        BASE_UNUSED(_RGBImage);
        BASE_UNUSED(_Transform);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::ShowImg(const std::vector<char>& Img_RGBA) const
    {
        cv::Mat CV_Img(cv::Size(m_OrigImgSize.x, m_OrigImgSize.y), CV_8UC4); // 2D Matrix(x*y) with (8-bit unsigned character) + (4 bands)
            // cv::Mat is built in BGR/BGRA in default.
            // cv::Mat is ImgH(Num of Row), ImgW(Num of Col)
        
        memcpy(CV_Img.data, Img_RGBA.data(), Img_RGBA.size());
        
        cv::cvtColor(CV_Img, CV_Img, cv::COLOR_BGRA2RGBA); // Transform to RGB before imshow & imwrite
        
        cv::imshow("Hello", CV_Img);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnStart()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin started!");

        //---Frame Resolution given by ARKit---
        m_FrameResolution = Core::CProgramParameters::GetInstance().Get("mr:stereo:frame_resolution", 0.5); // Full = 1; Half = 0.5;

        //---Keyframe Selection---
        m_Cdt_Keyf_MaxNum = Core::CProgramParameters::GetInstance().Get("mr:stereo:max_keyframe_number", 2);
        m_Cdt_Keyf_BaseLineL = Core::CProgramParameters::GetInstance().Get("mr:stereo:max_keyframe_baseline_length", 0.03);

        //---Keyframe Status for Calculation---
        m_idx_Keyf_Curt = false;
        m_idx_Keyf_Last = false;

        //---Rectification-----
        m_RectImgSize = Core::CProgramParameters::GetInstance().Get("mr:stereo:rectified_image_size", glm::ivec2(1280, 1040));

        //---OLD: For LibSGM---
        m_DisparityCount = Core::CProgramParameters::GetInstance().Get("mr:stereo:disparity_count", 128);

        m_pStereoMatcherCUDA = std::make_unique<sgm::StereoSGM>(m_RectImgSize.x, m_RectImgSize.y, m_DisparityCount, 8, 8, sgm::EXECUTE_INOUT_HOST2HOST);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin exited!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnPause()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin paused!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin resumed!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::EventHook()
    {
        ENGINE_CONSOLE_INFOV("Oh hello. An event has been raised!");
    }

} // namespace HW

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4& _Transform, const glm::mat4& _Intrinsics, const std::vector<uint16_t>& _rDepthImage)
{
    static_cast<Stereo::CPluginInterface&>(GetInstance()).OnFrameCPU(_rRGBImage, _Transform, _Intrinsics, _rDepthImage);
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void GetLatestDepthImageCPU(std::vector<char>& _rDepthImage)
{
    _rDepthImage = static_cast<Stereo::CPluginInterface&>(GetInstance()).GetLatestDepthImageCPU();
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void OnFrameGPU(Gfx::CTexturePtr _RGBImage, const glm::mat4& _Transform)
{
    static_cast<Stereo::CPluginInterface&>(GetInstance()).OnFrameGPU(_RGBImage, _Transform);
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void GetLatestDepthImageGPU(Gfx::CTexturePtr& _rDepthImage)
{
    _rDepthImage = static_cast<Stereo::CPluginInterface&>(GetInstance()).GetLatestDepthImageGPU();
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void SetIntrinsics(const glm::vec2& _rFocalLength, const glm::vec2& _rFocalPoint, const glm::ivec2& _rImageSize)
{
    static_cast<Stereo::CPluginInterface&>(GetInstance()).SetIntrinsics(_rFocalLength, _rFocalPoint, _rImageSize);
}