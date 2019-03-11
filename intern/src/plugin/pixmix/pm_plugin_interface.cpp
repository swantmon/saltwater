
#include "plugin/pixmix/pm_precompiled.h"

#include "base/base_include_glm.h"

#include "engine/core/core_console.h"

#include "engine/engine.h"

#include "plugin/pixmix/pm_plugin_interface.h"

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
        std::memcpy(_DestinationImage.data(), _SourceImage.data(), _DestinationImage.size());
    }
} // namespace HW

extern "C" CORE_PLUGIN_API_EXPORT void Inpaint(const glm::ivec2& _Resolution, const std::vector<char>& _SourceImage, std::vector<char>& _DestinationImage)
{
    static_cast<PM::CPluginInterface&>(GetInstance()).Inpaint(_Resolution, _SourceImage, _DestinationImage);
}