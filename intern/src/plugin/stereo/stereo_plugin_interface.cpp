
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
    }

    // -----------------------------------------------------------------------------

    std::vector<char> CPluginInterface::GetLatestDepthImageCPU() const
    {
        return std::vector<char>();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4& _Transform)
    {
        
        // BASE_UNUSED(_rRGBImage); // For variables which has not been used yet.
        // BASE_UNUSED(_Transform); // For variables which has not been used yet.

        //---Transform Image & Orientations to OpenCV format---
        cv::Mat Img_cv(cv::Size(m_ImageSize.x, m_ImageSize.y), CV_8UC4); // 2D Matrix(x*y) with (8-bit unsigned character) + (4 bands)
        memcpy(Img_cv.data, _rRGBImage.data(), _rRGBImage.size());
        cv::cvtColor(Img_cv, Img_cv, cv::COLOR_BGRA2RGBA); // Transform image from BGRA (default color mode in OpenCV) to RGBA

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

        //------
        
        if (SeqImg.empty())
        {
            SeqImg.resize(1);
            SeqImg[0] = FutoGmtCV(Img_cv, K_cv, R_cv, T_cv); 
        }
        else if (SeqImg.size() < ImgMaxCal)
        {
            SeqImg.resize(SeqImg.size() + 1);
            int Seq_Idx = SeqImg.size() - 1; 
            SeqImg[Seq_Idx] = FutoGmtCV(Img_cv, K_cv, R_cv, T_cv);
        }
        else
        {
            static bool T = true;
            if (T) // During the testing, Only finishing one Image Pair and then close the program.
            {
                for (std::vector<FutoGmtCV>::iterator iter = SeqImg.begin(); iter < SeqImg.end() - 1; iter++) // end() returns the next position of the last element.
                {
                    //---Epipolarization---

                    std::vector<FutoGmtCV>::iterator iterNext = iter + 1; // Next frame

                    //---show Original Img for check---
                    /*
                    cv::imshow("Img_Base_Orig", iter->get_Img());
                    cv::imshow("Img_Match_Orig", iterNext->get_Img());
                    */
                    //------

                    //---Compute Fundamental Matrix---
                    cv::Mat F_mtx(3, 3, CV_32F);
                    iter->cal_F_mtx(iterNext->get_P_mtx(), F_mtx);
                    //------

                    //---Generate Rectified Images---
                    cv::Mat RectImg_Curt, RectImg_Next;

                    //iter->cal_PolarRect(RectImg_Curt, RectImg_Next, iterNext->get_Img(), F_mtx); //Applied Polar Rectification
                   // iter->imp_PlanarRect(RectImg_Curt, RectImg_Next, *iterNext); //Applied Polar Rectification
                    RectImg_Curt = iter->get_Img();
                    RectImg_Next = iterNext->get_Img();
                    //------

                    //---Show Rectified Img for check---
                    
                    cv::imwrite("C:\\saltwater\\intern\\src\\plugin\\stereo\\Img_Base_Rect.tif", RectImg_Curt);
                    cv::imwrite("C:\\saltwater\\intern\\src\\plugin\\stereo\\Img_Match_Rect.tif", RectImg_Next);
                    
                    //---

                    //---Stereo Matching---
                    cv::Mat DispImg_Curt;
                    cv::Mat RectImg_Curt_Gray, RectImg_Next_Gray;
                    cv::cvtColor(RectImg_Curt, RectImg_Curt_Gray, cv::COLOR_RGBA2GRAY);
                    cv::cvtColor(RectImg_Next, RectImg_Next_Gray, cv::COLOR_RGBA2GRAY);
                    cv::imshow("RectImg_Curt_Gray", RectImg_Curt_Gray);
                    cv::imshow("RectImg_Next_Gray", RectImg_Next_Gray);
                    iter->imp_cvSGBM(DispImg_Curt, RectImg_Curt_Gray, RectImg_Next_Gray);
                    //---test SGBM in OpenCV---
                    /*
                    cv::Mat TestImgL = cv::imread("C:\\saltwater\\intern\\src\\plugin\\stereo\\Test\\im2.png");
                    cv::Mat TestImgR = cv::imread("C:\\saltwater\\intern\\src\\plugin\\stereo\\Test\\im6.png");
                    cv::imshow("TestImgL", TestImgL);
                    cv::imshow("TestImgR", TestImgR);
                    iter->imp_cvSGBM(DispImg_Curt, TestImgL, TestImgR);
                    */
                    //---
                    DispImg_Curt.convertTo(DispImg_Curt, CV_32F, 1.0 / 16); // Disparity Image is in 16-bit -> Divide by 16 to get real Disparity.

                    //---for imshow---
                    cv::Mat DispImg_Curt_8U(DispImg_Curt.size(), CV_8UC1);
                    cv::normalize(DispImg_Curt, DispImg_Curt_8U, 0, 255, cv::NORM_MINMAX, CV_8UC1);
                    cv::imshow("Img_Disp", DispImg_Curt_8U);
                    cv::imwrite("C:\\saltwater\\intern\\src\\plugin\\stereo\\Disp.png", DispImg_Curt_8U);

                    //---
                    T = false;
                }
            }
            

            //---Reform SeqImg---
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