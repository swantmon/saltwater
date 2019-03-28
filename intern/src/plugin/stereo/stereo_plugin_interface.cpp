
#include "plugin/stereo/stereo_precompiled.h"

#include "engine/core/core_console.h"
#include "engine/engine.h"
#include "engine/graphic/gfx_texture.h"

#include "plugin/stereo/stereo_plugin_interface.h"

#include "base/base_include_glm.h" // Some warnings appears when directly #include glm 
#include "opencv2/opencv.hpp" 


CORE_PLUGIN_INFO(Stereo::CPluginInterface, "Stereo Matching", "1.0", "This plugin takes RGB and transformation data and provides 2.5D depth maps")

namespace Stereo
{
    void CPluginInterface::SetIntrinsics(const glm::vec2& _rFocalLength, const glm::vec2& _rFocalPoint, const glm::ivec2& _rImageSize)
    {
        m_Camera_mtx = glm::mat3(glm::vec3(_rFocalLength.x, 0, 0), glm::vec3(0, _rFocalLength.y, 0), glm::vec3(_rFocalPoint.x, _rFocalPoint.y, 1)); // Unit is pixel
        m_ImageSize = _rImageSize;
        m_DisCoeff = glm::vec4(0.1325, -0.2666, -0.0030, 0.0020);
    }

    // -----------------------------------------------------------------------------

    std::vector<char> CPluginInterface::GetLatestDepthImageCPU() const
    {
        return std::vector<char>();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4& _Transform)
    {
        /*
        // If Valuables have not been used, use "BASE_UNUSED" to avoid warnings
        BASE_UNUSED(_rRGBImage); // For variables which has not been used yet.
        BASE_UNUSED(_Transform); // For variables which has not been used yet.
        */

        //---Transform Image & Orientations to OpenCV format---
        glm::mat4x3 P_glm = m_Camera_mtx * glm::mat4x3(_Transform);
        cv::Mat P_cv(3, 4, CV_32F);
        glm2cv(&P_cv, glm::transpose(P_glm));

        cv::Mat K_cv(3, 3, CV_32F);
        glm2cv(&K_cv, glm::transpose(m_Camera_mtx));

        glm::mat3 R_glm = glm::mat3(_Transform);
        cv::Mat R_cv(3, 3, CV_32F);
        glm2cv(&R_cv, glm::transpose(R_glm));

        glm::vec3 T_glm = glm::vec3(_Transform[3]);
        cv::Mat T_cv(3, 1, CV_32F);
        glm2cv(&T_cv, T_glm);

        cv::Mat Img_dist_cv(cv::Size(m_ImageSize.x, m_ImageSize.y), CV_8UC4); // 2D Matrix(x*y) with (8-bit unsigned character) + (4 bands)
        memcpy(Img_dist_cv.data, _rRGBImage.data(), _rRGBImage.size());
        cv::cvtColor(Img_dist_cv, Img_dist_cv, cv::COLOR_BGRA2RGBA); // Transform image from BGRA (default color mode in OpenCV) to RGBA

        cv::Mat Img_Undist_cv, DistCoeff_cv(4, 1, CV_32F);
        glm2cv(&DistCoeff_cv, m_DisCoeff);

        cv::undistort(Img_dist_cv, Img_Undist_cv, K_cv, DistCoeff_cv);

        //---Select Keyframe for Computation---
        if (Keyframes.empty())
        {
            Keyframes.resize(1);
            Keyframes[0] = FutoGmtCV(Img_Undist_cv, K_cv, R_cv, T_cv);
        }
        else if (Keyframes.size() < Cdt_Keyf_MaxNum)
        {
            //---Keyframe Selection: Baseline Condition---
            cv::Mat BaseLine = Keyframes.back().get_Trans() - T_cv;
            float BaseLineLength = cv::norm(BaseLine, cv::NORM_L2);
            if (BaseLineLength >= Cnd_Keyf_BaseLine)
            {
                Keyframes.resize(Keyframes.size() + 1); // Apply resize for memory allocation.
                Keyframes.back() = FutoGmtCV(Img_Undist_cv, K_cv, R_cv, T_cv); // Push_back & Pull_back are only applied to add/remove element -> Applying push/pull with memory allocation has bad efficiency
            }
        }
        else
        {

            static bool T = true;
            if (T) // During the testing, Only finishing one Image Pair and then close the program.
            {
                for (std::vector<FutoGmtCV>::iterator iter = Keyframes.begin(); iter < Keyframes.end() - 1; iter++) // end() returns the next position of the last element.
                {
                    std::vector<FutoGmtCV>::iterator iterNext = iter + 1; // Next frame
                    //---show Original Img for check---
                    cv::imshow("Img_Base_Orig", iter->get_Img());
                    cv::imshow("Img_Match_Orig", iterNext->get_Img());
                    //------

                    //---Compute Fundamental Matrix---
                    cv::Mat F_mtx(3, 3, CV_32F);
                    iter->cal_F_mtx(iterNext->get_P_mtx(), F_mtx);
                    //------

                    //---Generate Rectified Images---
                    cv::Mat RectImg_Curt, RectImg_Next;
                    cv::Mat TableB_x_Orig2Rect, TableB_y_Orig2Rect, TableM_x_Orig2Rect, TableM_y_Orig2Rect;

                    //iter->cal_PolarRect(RectImg_Curt, RectImg_Next, iterNext->get_Img(), F_mtx); //Applied Polar Rectification
                    //iter->imp_PlanarRect(RectImg_Curt, RectImg_Next, TableB_x_Orig2Rect, TableB_y_Orig2Rect, TableM_x_Orig2Rect, TableM_y_Orig2Rect, *iterNext); //Applied Polar Rectification
                    iter->imp_Rect_OpenCV(RectImg_Curt, RectImg_Next, TableB_x_Orig2Rect, TableB_y_Orig2Rect, TableM_x_Orig2Rect, TableM_y_Orig2Rect, *iterNext);

                    cv::Mat RectImg_Curt_Gray, RectImg_Next_Gray;
                    cv::cvtColor(RectImg_Curt, RectImg_Curt_Gray, cv::COLOR_RGBA2GRAY);
                    cv::cvtColor(RectImg_Next, RectImg_Next_Gray, cv::COLOR_RGBA2GRAY);
                    //---Show Rectified Img for check---
                    //char pixValue = RectImg_Curt_Gray.at<char>(113, 78);
                    cv::imwrite("C:\\saltwater\\intern\\src\\plugin\\stereo\\RectImg_Curt_Gray.png", RectImg_Curt_Gray);
                    cv::imwrite("C:\\saltwater\\intern\\src\\plugin\\stereo\\RectImg_Next_Gray.png", RectImg_Next_Gray);

                    //---Stereo Matching---
                    cv::Mat DispImg_Curt_Rect, DispImg_Curt_Orig;
                    iter->imp_cvSGBM(DispImg_Curt_Rect, RectImg_Curt_Gray, RectImg_Next_Gray);
                    //---
                    //---test SGBM in OpenCV---
                    /*
                    cv::Mat TestImgL = cv::imread("C:\\saltwater\\intern\\src\\plugin\\stereo\\Test\\im2.png");
                    cv::Mat TestImgR = cv::imread("C:\\saltwater\\intern\\src\\plugin\\stereo\\Test\\im6.png");
                    cv::imshow("TestImgL", TestImgL);
                    cv::imshow("TestImgR", TestImgR);
                    iter->imp_cvSGBM(DispImg_Curt, TestImgL, TestImgR);
                    */

                    DispImg_Curt_Rect.convertTo(DispImg_Curt_Rect, CV_32F, 1.0 / 16); // Disparity Image is in 16-bit -> Divide by 16 to get real Disparity.
                    DispImg_Curt_Orig = cv::Mat(iter->get_Img().size(), CV_32F);
                    cv::remap(DispImg_Curt_Rect, DispImg_Curt_Orig, TableB_x_Orig2Rect, TableB_y_Orig2Rect, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);

                    //---for imshow--- => Modify latter: Transform Disparity in Originals rather than in Rectified.
                    cv::Mat DispImg_Curt_Rect_8U(DispImg_Curt_Rect.size(), CV_8UC1);
                    cv::normalize(DispImg_Curt_Rect, DispImg_Curt_Rect_8U, 0, 255, cv::NORM_MINMAX, CV_8UC1);
                    cv::imwrite("C:\\saltwater\\intern\\src\\plugin\\stereo\\Disp_Rect.png", DispImg_Curt_Rect_8U);

                    cv::Mat DispImg_Curt_Orig_8U(DispImg_Curt_Orig.size(), CV_8UC1);
                    cv::remap(DispImg_Curt_Rect_8U, DispImg_Curt_Orig_8U, TableB_x_Orig2Rect, TableB_y_Orig2Rect, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);
                    cv::imwrite("C:\\saltwater\\intern\\src\\plugin\\stereo\\Disp_Orig.png", DispImg_Curt_Orig_8U);

                    //---Transform Disparity into Depth: Using Parallax Equation---


                    //---Free the First frame, shift rest of frames, Add new frame---


                    //---
                    T = false;
                }
            }
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

    void CPluginInterface::glm2cv(cv::Mat* cvmat, const glm::mat3& glmmat)
    {
        memcpy(cvmat->data, glm::value_ptr(glmmat), 9 * sizeof(float));
    }

    void CPluginInterface::glm2cv(cv::Mat* cvmat, const glm::vec3& glmmat)
    {
        memcpy(cvmat->data, glm::value_ptr(glmmat), 3 * sizeof(float));
    }

    void CPluginInterface::glm2cv(cv::Mat* cvmat, const glm::vec4& glmmat)
    {
        memcpy(cvmat->data, glm::value_ptr(glmmat), 4 * sizeof(float));
    }

    void CPluginInterface::glm2cv(cv::Mat* cvmat, const glm::mat3x4& glmmat)
    {
        memcpy(cvmat->data, glm::value_ptr(glmmat), 12 * sizeof(float));
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::ShowImg(const std::vector<char>& Img_RGBA) const
    {
        cv::Mat CV_Img(cv::Size(m_ImageSize.x, m_ImageSize.y), CV_8UC4); // 2D Matrix(x*y) with (8-bit unsigned character) + (4 bands)
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

extern "C" CORE_PLUGIN_API_EXPORT void OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4& _Transform)
{
    static_cast<Stereo::CPluginInterface&>(GetInstance()).OnFrameCPU(_rRGBImage, _Transform);
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