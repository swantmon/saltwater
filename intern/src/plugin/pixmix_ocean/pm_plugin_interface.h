
#pragma once

#include "engine/core/core_plugin_manager.h"

namespace PM
{
    class CPluginInterface : public Core::IPlugin
    {
    public:

        void OnStart() override;
        void OnExit() override;
        void OnPause() override;
        void OnResume() override;
        void Update() override;

    public:

        void EventHook();
        void Inpaint(const glm::ivec2& _Resolution, const std::vector<glm::u8vec4>& _SourceImage, std::vector<glm::u8vec4>& _DestinationImage);
    };
} // namespace PM