
#include "plugin/pixmix/pm_precompiled.h"

#include "base/base_include_glm.h"
#include "engine/core/core_console.h"
#include "engine/engine.h"
#include "plugin/pixmix/pm_plugin_interface.h"
#include "plugin/pixmix/PixMix/PixMix.h"

#include "opencv2/opencv.hpp"

#include <vector>

CORE_PLUGIN_INFO(PM::CPluginInterface, "PixMix", "1.0", "This plugin enables inpainting with PixMix.")

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

    void CPluginInterface::Inpaint(const glm::ivec2& _Resolution, const std::vector<char>& _SourceImage, std::vector<char>& _DestinationImage)
    {
        assert(_Resolution.x > 0 && _Resolution.y > 0);
        assert(_SourceImage.size() == _DestinationImage.size());

        cv::Mat_<cv::Vec4b> Source4(_Resolution.x, _Resolution.y);
        cv::Mat_<cv::Vec4b> Dest4(_Resolution.x, _Resolution.y);

        std::memcpy(Source4.data, _SourceImage.data(), _SourceImage.size());
        std::memcpy(Dest4.data, _DestinationImage.data(), _DestinationImage.size());
                
        cv::Mat_<cv::Vec3b> Source3(_Resolution.x, _Resolution.y);
        cv::Mat_<cv::Vec3b> Dest3(_Resolution.x, _Resolution.y);

        cv::cvtColor(Source4, Source3, CV_RGBA2RGB);
        cv::cvtColor(Dest4, Dest3, CV_RGBA2RGB);

        cv::Mat_<uchar> Mask;

        Util::createMask(Source3, cv::Scalar(255, 255, 255), Mask);

        cv::imshow("Input color image", Source3);
        cv::imshow("Input mask image", Mask);
        cv::waitKey(1);

        PixMix pm;
        pm.init(Source3, Mask);

        pm.execute(Dest3, 0.05f);

        cv::imshow("Output color image", Dest3);
        cv::waitKey();
    }
} // namespace HW

extern "C" CORE_PLUGIN_API_EXPORT void Inpaint(const glm::ivec2& _Resolution, const std::vector<char>& _SourceImage, std::vector<char>& _DestinationImage)
{
    static_cast<PM::CPluginInterface&>(GetInstance()).Inpaint(_Resolution, _SourceImage, _DestinationImage);
}