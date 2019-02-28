
#include "plugin/stereo/stereo_precompiled.h"

#include "engine/core/core_console.h"

#include "engine/engine.h"
#include "engine/graphic/gfx_texture.h"

#include "plugin/stereo/stereo_plugin_interface.h"

#include "opencv2/opencv.hpp"


CORE_PLUGIN_INFO(Stereo::CPluginInterface, "Stereo Matching", "1.0", "This plugin takes RGB and transformation data and provides 2.5D depth maps")

namespace Stereo
{
    void CPluginInterface::SetIntrinsics(const glm::vec2& _rFocalLength, const glm::vec2& _rFocalPoint, const glm::ivec2& _rImageSize)
    {
        m_FocalLength = _rFocalLength;
        m_FocalPoint = _rFocalPoint;
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
        
        BASE_UNUSED(_rRGBImage);
        BASE_UNUSED(_Transform);
        
        if (SeqImg_RGB.empty())
        {
            SeqImg_RGB.resize(1);
            SeqImg_RGB[0] = Fu_FotoGmtCV(_rRGBImage);
            SeqImg_RGB[0].setPmtx(_Transform);
        }
        else if (SeqImg_RGB.size() < ImgMaxCal)
        {
            SeqImg_RGB.resize(SeqImg_RGB.size() + 1);
            SeqImg_RGB[SeqImg_RGB.size() - 1] = Fu_FotoGmtCV(_rRGBImage);
        }
        else
        {
            // Start Calculation; SfM + SGM
            
        }
        
        
        // Optional for internal check
        /*
        CPluginInterface::ShowImg(_rRGBImage); // Showing image for visual checking -> Modify to control in editor.config
        */
        
        
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
        cv::Mat CV_Img(cv::Size(m_ImageSize.x, m_ImageSize.y), CV_8UC4); // 2D Matrix(x*y) with (8-bit unsigned character) + (4 bands)
            // cv::Mat is built in BGR/BGRA in default.
        
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