
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

        cv::imshow("Input color image", Source3);
        cv::imshow("Input mask image", Mask);
        //cv::waitKey(1);

        PixMix pm;
        pm.init(Source3, Mask);

        pm.execute(Dest3, 0.05f);

        cv::imshow("Output color image", Dest3);
        //cv::waitKey();

        cv::cvtColor(Dest3, Dest4, cv::COLOR_RGB2BGRA);

        std::memcpy(_DestinationImage.data(), Dest4.data, _DestinationImage.size());
    }
} // namespace HW

extern "C" CORE_PLUGIN_API_EXPORT void Inpaint(const glm::ivec2& _Resolution, const std::vector<char>& _SourceImage, std::vector<char>& _DestinationImage)
{
    static_cast<PM::CPluginInterface&>(GetInstance()).Inpaint(_Resolution, _SourceImage, _DestinationImage);
}