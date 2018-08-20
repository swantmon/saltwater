
#pragma once

#include "engine/core/core_plugin_manager.h"

#include "plugin/slam/mr_scalable_slam_reconstructor.h"

namespace SLAM
{
    class CPluginInterface : public Core::IPlugin
    {
    public:

        void InitializeReconstructor();
        void TerminateReconstructor();
        void OnNewFrame(Gfx::CTexturePtr, const char* pColorBuffer, const glm::mat4* pTransform);
        void SetImageSizesAndIntrinsicData(glm::vec4 _ImageSizes, glm::vec4 _Intrinsics);
        void ResetReconstruction();

    public:

        void OnStart() override;
        void OnExit() override;
        void OnPause() override;
        void OnResume() override;
        void Update() override;

    private:


    };
} // namespace HW