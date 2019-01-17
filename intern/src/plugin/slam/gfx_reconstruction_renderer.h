#pragma once

#include "mr_scalable_slam_reconstructor.h"

namespace Gfx
{
namespace ReconstructionRenderer
{
    void OnStart();
    void OnExit();
    
    void OnSetupShader();
    void OnSetupKernels();
    void OnSetupRenderTargets();
    void OnSetupStates();
    void OnSetupTextures();
    void OnSetupBuffers();
    void OnSetupModels();
    void OnSetupResources();
    void OnSetupEnd();
    
    void OnReload();
    void OnNewMap();
    void OnUnloadMap();
    
    void Update();
    void Render();
    void RenderForward();
    void ChangeCamera(bool _IsTrackingCamera);

    void SetReconstructor(MR::CScalableSLAMReconstructor& _rReconstructor);

    glm::vec3 Pick(const glm::ivec2& _rCursor);

    void SetSelectionBox(const glm::vec3& _rAnchor0, const glm::vec3& _rAnchor1, float _Height, int _State);

    void AddPositionToSelection(const glm::vec3& _rWSPosition);
    void ResetSelection();
} // namespace ReconstructionRenderer
} // namespace Gfx
