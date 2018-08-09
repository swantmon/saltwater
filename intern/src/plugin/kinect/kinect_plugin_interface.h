
#pragma once

#include "base/base_include_glm.h"

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

        bool GetDepthBuffer(unsigned short* pBuffer);
        void GetColorBuffer(char* pBuffer);

        void GetIntrinsics(glm::vec2& FocalLength, glm::vec2& FocalPoint, glm::ivec2& Size);

    private:

        std::unique_ptr<MR::CKinectControl> m_pControl;
    };
} // namespace HW