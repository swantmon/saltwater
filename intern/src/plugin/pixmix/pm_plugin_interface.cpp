
#include "plugin/pixmix/pm_precompiled.h"


#include "base/base_include_glm.h"
#include "engine/core/core_console.h"
#include "engine/engine_config.h"
#include "plugin/pixmix/pm_plugin_interface.h"

#ifndef OCEAN_PIXMIX

#ifdef ENGINE_DEBUG_MODE

#pragma comment(lib, "opencv_world410d.lib")

#else

#pragma comment(lib, "opencv_world410.lib")

#endif

#include "plugin/pixmix/PixMix/PixMix.h"
#include "opencv2/opencv.hpp"

#else

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

#include <vector>

#ifndef OCEAN_PIXMIX
CORE_PLUGIN_INFO(PM::CPluginInterface, "PixMix", "1.0", "This plugin enables inpainting with PixMix (Open version).")
#else
CORE_PLUGIN_INFO(PM::CPluginInterface, "PixMix", "1.0", "This plugin enables inpainting with PixMix (Original version).")
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
        InpaintInternal(_Resolution, _SourceImage, _DestinationImage, false);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::InpaintWithMask(const glm::ivec2& _Resolution, const std::vector<glm::u8vec4>& _SourceImage, std::vector<glm::u8vec4>& _DestinationImage)
    {
        InpaintInternal(_Resolution, _SourceImage, _DestinationImage, true);
    }

    // -----------------------------------------------------------------------------

#ifndef OCEAN_PIXMIX
    void CPluginInterface::InpaintInternal(const glm::ivec2& _Resolution, const std::vector<glm::u8vec4>& _SourceImage, std::vector<glm::u8vec4>& _DestinationImage, bool _MaskInAlpha)
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
                cv::Vec3b color = Source3(r, c);

                if (color[0] == 255 && color[1] == 255 && color[2] == 255)
                {
                    Mask(r, c) = 0;
                    if (r + 2 < Source3.rows)
                    {
                        Mask(r + 2, c) = 0;
                    }

                    if (r - 2 >= 0)
                    {
                        Mask(r - 2, c) = 0;
                    }

                    if (c + 2 < Source3.cols)
                    {
                        Mask(r, c + 2) = 0;
                    }

                    if (c - 2 >= 0)
                    {
                        Mask(r, c - 2) = 0;
                    }
                }
            }
        }

#ifdef ENGINE_DEBUG_MODE
        cv::imshow("Input color image", Source3);
        cv::imshow("Input mask image", Mask);
#endif

        PixMix pm;
        pm.init(Source3, Mask);

        pm.execute(Dest3, 0.05f);

#ifdef ENGINE_DEBUG_MODE
        cv::imshow("Output color image", Dest3);
#endif

        cv::cvtColor(Dest3, Dest4, cv::COLOR_RGB2BGRA);

        std::memcpy(_DestinationImage.data(), Dest4.data, _DestinationImage.size() * sizeof(_DestinationImage[0]));
    }

#else

    // -----------------------------------------------------------------------------

    void CPluginInterface::InpaintInternal(const glm::ivec2& _Resolution, const std::vector<glm::u8vec4>& _SourceImage, std::vector<glm::u8vec4>& _DestinationImage, bool _MaskInAlpha)
    {
        assert(_Resolution.x > 0 && _Resolution.y > 0);
        assert(_SourceImage.size() == _DestinationImage.size());

        std::vector<glm::u8vec3> Image(_Resolution.x * _Resolution.y);
        std::vector<unsigned char> Mask(_Resolution.x * _Resolution.y);

        for (int i = 0; i < _Resolution.x * _Resolution.y; ++i)
        {
            glm::u8vec4 Pixel = _SourceImage[i];
            Image[i] = glm::u8vec3(Pixel.r, Pixel.g, Pixel.b);
            Mask[i] = 255 - Pixel.a;
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