
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
        void OnNewFrame(Gfx::CTexturePtr, Gfx::CTexturePtr ColorBuffer, const glm::mat4* pTransform);
        void SetImageSizesAndIntrinsicData(glm::vec4 _ImageSizes, glm::vec4 _Intrinsics);
        void ResetReconstruction();
        glm::mat4 GetPoseMatrix();
        glm::vec3 Pick(const glm::ivec2 _rCursor);
        void SetSelectionBox(const glm::vec3& _rAnchor0, const glm::vec3& _rAnchor1, float _Height, int _State);
        bool IsCapturingColor();

    public:

        void OnStart() override;
        void OnExit() override;
        void OnPause() override;
        void OnResume() override;
        void Update() override;

    private:


    };
} // namespace HW