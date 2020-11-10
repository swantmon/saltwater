
#pragma once

#include "engine/core/core_plugin_manager.h"

#include "plugin/slam/mr_slam_control_arkit.h"

#include "engine/script/script_slam.h"

namespace SLAM
{
    class CPluginInterface : public Core::IPlugin
    {
    public:

        void OnStart() override;
        void OnExit() override;
        void OnPause() override;
        void OnResume() override;
        void Update() override;

        void OnInput(const Base::CInputEvent& _rEvent);

        void UpdateScriptSettings(const Scpt::CSLAMScript::SScriptSettings& _rSettings);

        void CPluginInterface::ReadScene(CSceneReader& _rCodec);
        void CPluginInterface::WriteScene(CSceneWriter& _rCodec);

    private:

        MR::CSLAMControlARKit m_SLAMControl;

        Engine::CEventDelegates::HandleType m_UpdateDelegate;
        Engine::CEventDelegates::HandleType m_RenderGBufferDelegate;
        Engine::CEventDelegates::HandleType m_RenderForwardDelegate;
    };
} // namespace HW