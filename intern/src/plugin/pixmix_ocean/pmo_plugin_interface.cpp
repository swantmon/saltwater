
#include "plugin/pixmix_ocean/pmo_precompiled.h"

#include "base/base_include_glm.h"
#include "engine/core/core_console.h"
#include "engine/engine.h"
#include "plugin/pixmix_ocean/pmo_plugin_interface.h"

#include <vector>

#if _DEBUG
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

CORE_PLUGIN_INFO(PMO::CPluginInterface, "PixMixOcean", "1.0", "This plugin enables inpainting with PixMix (Original version).")

namespace PMO
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
        assert(_Resolution.x > 0 && _Resolution.y > 0);
        assert(_SourceImage.size() == _DestinationImage.size());

		std::vector<glm::u8vec3> Image(_Resolution.x * _Resolution.y);
		std::vector<unsigned char> Mask(_Resolution.x * _Resolution.y);

		for (int i = 0; i < _Resolution.x * _Resolution.y; ++ i)
		{
			glm::u8vec4 Pixel = _SourceImage[i];
			Image[i] = glm::u8vec3(Pixel.r, Pixel.g, Pixel.b);
			Mask[i] = Pixel.r == 255 && Pixel.g == 255 && Pixel.b == 255 ? 0x00 : 0xFF;
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

		for (int i = 0; i < _Resolution.x * _Resolution.y; ++ i)
		{
			_DestinationImage[i] = glm::u8vec4(pOutputPixels[i], 255);
		}
    }
} // namespace PMO

extern "C" CORE_PLUGIN_API_EXPORT void Inpaint(const glm::ivec2& _Resolution, const std::vector<glm::u8vec4>& _SourceImage, std::vector<glm::u8vec4>& _DestinationImage)
{
    static_cast<PMO::CPluginInterface&>(GetInstance()).Inpaint(_Resolution, _SourceImage, _DestinationImage);
}