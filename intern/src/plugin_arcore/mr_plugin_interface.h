
#pragma once

#include "core/core_plugin_manager.h"

namespace MR
{
    class CPluginInterface : public Core::IPlugin
    {
    public:

        void OnStart() override;
        void OnExit() override;
        void Update() override;
    };
} // namespace MR