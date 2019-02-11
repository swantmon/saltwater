
#pragma once

#include "engine/core/core_plugin_manager.h"

#include "engine/engine.h"

namespace MR
{
    class CPluginInterface : public Core::IPlugin
    {
    public:

        void OnStart() override;
        void OnExit() override;
        void OnPause() override;
        void OnResume() override;
        void Update() override;

    private:

        void Gfx_OnUpdate();
        Engine::CEventDelegates::HandleType m_EventDelegateHandle;
    };
} // namespace MR