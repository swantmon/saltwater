
#include "plugin/stereo/stereo_precompiled.h"

#include "engine/core/core_console.h"

#include "engine/engine.h"
#include "engine/graphic/gfx_texture.h"

#include "plugin/stereo/stereo_plugin_interface.h"

#include "opencv2/opencv.hpp" // After completing, remove #include openCV. => Do not apply OpenCV in Engine part.


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
        
        if (SeqImg_RGB.empty())
        {
            SeqImg_RGB.resize(1);

            SeqImg_RGB[0] = FutoGmtCV(_rRGBImage, m_ImageSize.x, m_ImageSize.y); // Image Data, Image Width (# of col), Image Height (# of row)
            SeqImg_RGB[0].set_Cam(m_Camera_mtx);
            SeqImg_RGB[0].set_Rot(glm::mat3(_Transform));
            SeqImg_RGB[0].set_Trans(-1 * glm::transpose(glm::mat3(_Transform)) * glm::vec3(_Transform[3]));
            SeqImg_RGB[0].set_P_mtx(m_Camera_mtx * glm::mat4x3(_Transform));
        }
        else if (SeqImg_RGB.size() < ImgMaxCal)
        {
            SeqImg_RGB.resize(SeqImg_RGB.size() + 1);
            int Seq_Idx = SeqImg_RGB.size() - 1; // Maybe can replace by iterator
            SeqImg_RGB[Seq_Idx] = FutoGmtCV(_rRGBImage, m_ImageSize.x, m_ImageSize.y);
            SeqImg_RGB[Seq_Idx].set_Cam(m_Camera_mtx);
            SeqImg_RGB[Seq_Idx].set_Rot(glm::mat3(_Transform));
            SeqImg_RGB[Seq_Idx].set_Trans(-1 * glm::transpose(glm::mat3(_Transform)) * glm::vec3(_Transform[3]));
            SeqImg_RGB[Seq_Idx].set_P_mtx(m_Camera_mtx * glm::mat4x3(_Transform));
        }
        else
        {
            // Start Stereo Matching

            for (std::vector<FutoGmtCV>::iterator iter = SeqImg_RGB.begin(); iter < SeqImg_RGB.end()-1; iter++) // end() returns the next position of the last element.
            {

                std::vector<FutoGmtCV>::iterator iterNext = iter + 1; // Next frame

                //---Check the Orientation of 2 Images---
                float r11_B = iter->get_P_mtx().at<float>(0, 0);
                float r22_B = iter->get_P_mtx().at<float>(1, 1);
                float r33_B = iter->get_P_mtx().at<float>(2, 2);
                float T1_B = iter->get_P_mtx().at<float>(0, 3);
                float T2_B = iter->get_P_mtx().at<float>(1, 3);
                float T3_B = iter->get_P_mtx().at<float>(2, 3);

                float r11_M = iterNext->get_P_mtx().at<float>(0, 0);
                float r22_M = iterNext->get_P_mtx().at<float>(1, 1);
                float r33_M = iterNext->get_P_mtx().at<float>(2, 2);
                float T1_M = iterNext->get_P_mtx().at<float>(0, 3);
                float T2_M = iterNext->get_P_mtx().at<float>(1, 3);
                float T3_M = iterNext->get_P_mtx().at<float>(2, 3);

                //---show Original Img for check---
                cv::imshow("Img_Base_Orig", iter->get_Img());
                cv::imshow("Img_Match_Orig", iterNext->get_Img());
                //---

                cv::Mat RectImg_Curt, RectImg_Next;

                cv::Mat F_mtx(3, 3, CV_16F);
                iter->cal_F_mtx(iterNext->get_P_mtx(), F_mtx);

                iter->cal_PolarRect(iterNext->get_Img(), F_mtx, RectImg_Curt, RectImg_Next);

                //---Show Rectified Img for check---
                cv::imshow("Img_Base_Rect", RectImg_Curt);
                cv::imshow("Img_Match_Rect", RectImg_Next);
                //---
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