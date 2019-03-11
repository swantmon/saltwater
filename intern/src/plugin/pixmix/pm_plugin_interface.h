
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

    private:
    };
} // namespace PM