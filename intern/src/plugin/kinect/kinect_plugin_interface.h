
#pragma once

#include "engine/core/core_plugin_manager.h"

#include "plugin/kinect/kinect_control.h"

#include <memory>

namespace HW
{
    class CPluginInterface : public Core::IPlugin
    {
    public:

        void OnStart() override;
        void OnExit() override;
        void OnPause() override;
        void OnResume() override;
        void Update() override;

        void GetDepthBuffer(unsigned short* pBuffer);
        void GetColorBuffer(char* pBuffer);

    private:

        std::unique_ptr<MR::CKinectControl> m_pControl;
    };
} // namespace HW