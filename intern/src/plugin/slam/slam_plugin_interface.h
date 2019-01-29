
#pragma once

#include "engine/core/core_plugin_manager.h"

#include "plugin/slam/mr_scalable_slam_reconstructor.h"

#include "plugin/slam/mr_slam_control.h"

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

        void SetActivateSelection(bool _Flag);

    private:

        MR::CSLAMControl m_SLAMControl;
    };
} // namespace HW