
#include "plugin/pixmix/pm_precompiled.h"


#include "base/base_include_glm.h"
#include "engine/core/core_console.h"
#include "engine/engine_config.h"
#include "plugin/pixmix/pm_plugin_interface.h"

#include <opencv2/opencv.hpp>

#include <vector>

#ifdef OCEAN_PIXMIX_ONLY
CORE_PLUGIN_INFO(PM::CPluginInterface, "PixMix", "1.0", "This plugin enables inpainting with PixMix (Ocean version).")
#elif defined OPEN_PIXMIX_ONLY
CORE_PLUGIN_INFO(PM::CPluginInterface, "PixMix", "1.0", "This plugin enables inpainting with PixMix (Open version).")
#elif defined PIXMIX_FALLBACK
CORE_PLUGIN_INFO(PM::CPluginInterface, "PixMix", "1.0", "This plugin enables inpainting with PixMix (Fallback version).")
#endif

#if defined OCEAN_PIXMIX_ONLY || defined PIXMIX_FALLBACK

    #ifdef ENGINE_DEBUG_MODE

        #pragma comment(lib, "OceanBaseD.lib")
        #pragma comment(lib, "OceanCVD.lib")
        #pragma comment(lib, "OceanCVDetectorD.lib")
        #pragma comment(lib, "OceanCVSegmentationD.lib")
        #pragma comment(lib, "OceanCVSynthesisD.lib")
        #pragma comment(lib, "OceanGeometryD.lib")
        #pragma comment(lib, "OceanIOD.lib")
        #pragma comment(lib, "OceanMathD.lib")
        #pragma comment(lib, "OceanMediaD.lib")
        #pragma comment(lib, "OceanNetworkD.lib")
        #pragma comment(lib, "OceanPlatformWinD.lib")

    #else

        #pragma comment(lib, "OceanBase.lib")
        #pragma comment(lib, "OceanCV.lib")
        #pragma comment(lib, "OceanCVDetector.lib")
        #pragma comment(lib, "OceanCVSegmentation.lib")
        #pragma comment(lib, "OceanCVSynthesis.lib")
        #pragma comment(lib, "OceanGeometry.lib")
        #pragma comment(lib, "OceanIO.lib")
        #pragma comment(lib, "OceanMath.lib")
        #pragma comment(lib, "OceanMedia.lib")
        #pragma comment(lib, "OceanNetwork.lib")
        #pragma comment(lib, "OceanPlatformWin.lib")

    #endif

    #pragma warning(push)
    #pragma warning(disable : 4100 4245 5033)

    #define _WINDOWS
    #undef _DLL
    #include "ocean/cv/synthesis/SynthesisOneFramePixel.h"

    #pragma warning(pop)

#endif

#if defined OPEN_PIXMIX_ONLY || defined PIXMIX_FALLBACK
    #include "plugin/pixmix/PixMix/PixMix.h"
    #include "opencv2/opencv.hpp"

    #ifdef ENGINE_DEBUG_MODE
    #pragma comment(lib, "opencv_world410d.lib")
    #else
    #pragma comment(lib, "opencv_world410.lib")
    #endif

#endif


namespace PM
{
    void CPluginInterface::OnStart()
    {

    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnPause()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::EventHook()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Inpaint(const glm::ivec2& _Resolution, const std::vector<glm::u8vec4>& _SourceImage, std::vector<glm::u8vec4>& _DestinationImage)
    {
        int PixelCount = 0;
        for (const auto& Pixel : _SourceImage)
        {
            if (!(Pixel[0] == 0 && Pixel[1] == 0 && Pixel[2] == 0))
            {
                ++PixelCount;
            }
        }

        if (PixelCount < 50)
        {
            ENGINE_CONSOLE_DEBUG("Image is pure black");
            std::copy(_SourceImage.begin(), _SourceImage.end(), _DestinationImage.begin());
            return;
        }

#ifdef OCEAN_PIXMIX_ONLY
        InpaintWithOcean(_Resolution, _SourceImage, _DestinationImage, false);
#elif defined OPEN_PIXMIX_ONLY
        InpaintWithOpen(_Resolution, _SourceImage, _DestinationImage, false);
#elif defined PIXMIX_FALLBACK
        try
        {
            InpaintWithOcean(_Resolution, _SourceImage, _DestinationImage, false);
            ENGINE_CONSOLE_DEBUG("Inpainted with Ocean Version");
        }
        catch (...)
        {
            InpaintWithOpen(_Resolution, _SourceImage, _DestinationImage, false);
            ENGINE_CONSOLE_DEBUG("Inpainted with Open Version");
        }
#endif
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::InpaintWithMask(const glm::ivec2& _Resolution, const std::vector<glm::u8vec4>& _SourceImage, std::vector<glm::u8vec4>& _DestinationImage)
    {
        int PixelCount = 0;
        cv::Vec3f Sum(0.0f, 0.0f, 0.0f);
        for (const auto& Pixel : _SourceImage)
        {
            if (!(Pixel[0] == 0 && Pixel[1] == 0 && Pixel[2] == 0))
            {
                ++PixelCount;
                Sum[0] += Pixel[0];
                Sum[1] += Pixel[1];
                Sum[2] += Pixel[2];
            }
        }

        if (PixelCount == 0)
        {
            ENGINE_CONSOLE_DEBUG("Image is pure black");
            std::copy(_SourceImage.begin(), _SourceImage.end(), _DestinationImage.begin());
            return;
        }

        if (PixelCount < 50)
        {
            ENGINE_CONSOLE_DEBUG("Image is almost black");

            auto Average = Sum / PixelCount;
            for (int i = 0; i < _DestinationImage.size(); ++ i)
            {
                _DestinationImage[i] = glm::u8vec4(Average[0], Average[1], Average[2], 1.0f);
            }

            cv::Mat Source(cv::Size(_Resolution.x, _Resolution.y), CV_8UC4);
            cv::Mat Dest(cv::Size(_Resolution.x, _Resolution.y), CV_8UC4);

            std::memcpy(Source.data, _SourceImage.data(), _SourceImage.size() * sizeof(_SourceImage[0]));
            std::memcpy(Dest.data, _DestinationImage.data(), _DestinationImage.size() * sizeof(_DestinationImage[0]));

//             cv::imwrite("Source.png", Source);
//             cv::imwrite("Dest.png", Dest);

            return;
        }

#ifdef OCEAN_PIXMIX_ONLY
        InpaintWithOcean(_Resolution, _SourceImage, _DestinationImage, true);
#elif defined OPEN_PIXMIX_ONLY
        InpaintWithOpen(_Resolution, _SourceImage, _DestinationImage, true);
#elif defined PIXMIX_FALLBACK
        try
        {
            InpaintWithOcean(_Resolution, _SourceImage, _DestinationImage, true);
            ENGINE_CONSOLE_DEBUG("Inpainted with Ocean Version");
        }
        catch (...)
        {
            InpaintWithOpen(_Resolution, _SourceImage, _DestinationImage, true);
            ENGINE_CONSOLE_DEBUG("Inpainted with Open Version");
        }
#endif
    }

    // -----------------------------------------------------------------------------

#if defined OPEN_PIXMIX_ONLY || defined PIXMIX_FALLBACK

    void CPluginInterface::InpaintWithOpen(const glm::ivec2& _Resolution, const std::vector<glm::u8vec4>& _SourceImage, std::vector<glm::u8vec4>& _DestinationImage, bool _MaskInAlpha)
    {
        BASE_UNUSED(_MaskInAlpha);

        assert(_Resolution.x > 0 && _Resolution.y > 0);
        assert(_SourceImage.size() == _DestinationImage.size());

        auto Resolution = glm::ivec2(_Resolution.y, _Resolution.x);

        cv::Mat_<cv::Vec4b> Source4(Resolution.x, Resolution.y);
        cv::Mat_<cv::Vec4b> Dest4(Resolution.x, Resolution.y);

        std::memcpy(Source4.data, _SourceImage.data(), _SourceImage.size() * sizeof(_SourceImage[0]));
        std::memcpy(Dest4.data, _DestinationImage.data(), _DestinationImage.size() * sizeof(_DestinationImage[0]));

        cv::Mat_<cv::Vec3b> Source3(Resolution.x, Resolution.y);
        cv::Mat_<cv::Vec3b> Dest3(Resolution.x, Resolution.y);

        cv::cvtColor(Source4, Source3, cv::COLOR_BGRA2RGB);

        uchar NonMaskValue = 255;
        cv::Mat_<uchar> Mask(Source3.rows, Source3.cols, NonMaskValue);

        for (int r = 0; r < Source3.rows; ++r)
        {
            for (int c = 0; c < Source3.cols; ++c)
            {
                cv::Vec4b color = Source4(r, c);

                if (_MaskInAlpha ? (color[3] == 0) : (color[0] == 255 && color[1] == 255 && color[2] == 255))
                {
                    Mask(r, c) = 0;
                }
            }
        }

//         cv::imwrite("Source4.png", Source4);
//         cv::imwrite("Mask.png", Mask);
// 
// #ifdef ENGINE_DEBUG_MODE
//         cv::imshow("Input color image", Source3);
//         cv::imshow("Input mask image", Mask);
// #endif

        PixMix pm;
        pm.init(Source3, Mask);

        pm.execute(Dest3, 0.05f);

// #ifdef ENGINE_DEBUG_MODE
//         cv::imshow("Output color image", Dest3);
// #endif
// 
//         cv::imwrite("Output.png", Dest3);

        cv::cvtColor(Dest3, Dest4, cv::COLOR_RGB2BGRA);

        std::memcpy(_DestinationImage.data(), Dest4.data, _DestinationImage.size() * sizeof(_DestinationImage[0]));
    }

#endif

    // -----------------------------------------------------------------------------

#if defined OCEAN_PIXMIX_ONLY || defined PIXMIX_FALLBACK

    void CPluginInterface::InpaintWithOcean(const glm::ivec2& _Resolution, const std::vector<glm::u8vec4>& _SourceImage, std::vector<glm::u8vec4>& _DestinationImage, bool _MaskInAlpha)
    {
        assert(_Resolution.x > 0 && _Resolution.y > 0);
        assert(_SourceImage.size() == _DestinationImage.size());

        std::vector<glm::u8vec3> Image(_Resolution.x * _Resolution.y);
        std::vector<unsigned char> Mask(_Resolution.x * _Resolution.y);

        if (_MaskInAlpha)
        {
            for (int i = 0; i < _Resolution.x * _Resolution.y; ++i)
            {
                glm::u8vec4 Pixel = _SourceImage[i];
                Image[i] = glm::u8vec3(Pixel.r, Pixel.g, Pixel.b);
                Mask[i] = Pixel.a < 255 ? 0x00 : 0xFF;
            }
        }
        else
        {
            for (int i = 0; i < _Resolution.x * _Resolution.y; ++i)
            {
                glm::u8vec4 Pixel = _SourceImage[i];
                Image[i] = glm::u8vec3(Pixel.r, Pixel.g, Pixel.b);
                Mask[i] = (Pixel.r == 255 && Pixel.g == 255 && Pixel.b == 255) ? 0x00 : 0xFF;
            }
        }

        Ocean::CV::Synthesis::SynthesisOneFramePixel PixMix;
        Ocean::RandomGenerator Generator;

        Ocean::FrameType InputType(_Resolution.x, _Resolution.y, Ocean::FrameType::FORMAT_RGB24, Ocean::FrameType::PixelOrigin::ORIGIN_UPPER_LEFT);
        Ocean::Frame Input(InputType, &Image[0].r, true);

        Ocean::FrameType MaskType(_Resolution.x, _Resolution.y, Ocean::FrameType::FORMAT_Y8, Ocean::FrameType::PixelOrigin::ORIGIN_UPPER_LEFT);
        Ocean::Frame MaskF(MaskType, Mask.data(), true);

        PixMix.initialize(Input, MaskF);

        PixMix.applyInpainting(Generator, Ocean::CV::Synthesis::SynthesisOneFramePixel::IT_PATCH_REGION_2);

        PixMix.createInpaintingResult(Input);

        _DestinationImage.resize(_Resolution.x * _Resolution.y);

        const auto pOutputPixels = reinterpret_cast<glm::u8vec3*>(Input.data());

        for (int i = 0; i < _Resolution.x * _Resolution.y; ++i)
        {
            _DestinationImage[i] = glm::u8vec4(pOutputPixels[i], 255);
        }
    }

#endif
} // namespace PM

extern "C" CORE_PLUGIN_API_EXPORT void Inpaint(const glm::ivec2 & _Resolution, const std::vector<glm::u8vec4> & _SourceImage, std::vector<glm::u8vec4> & _DestinationImage)
{
    static_cast<PM::CPluginInterface&>(GetInstance()).Inpaint(_Resolution, _SourceImage, _DestinationImage);
}

extern "C" CORE_PLUGIN_API_EXPORT void InpaintWithMask(const glm::ivec2 & _Resolution, const std::vector<glm::u8vec4> & _SourceImage, std::vector<glm::u8vec4> & _DestinationImage)
{
    static_cast<PM::CPluginInterface&>(GetInstance()).InpaintWithMask(_Resolution, _SourceImage, _DestinationImage);
}