#pragma once

#include "mr_slam_reconstructor.h"

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
    void RenderHitProxy();
    void ChangeCamera(bool _IsTrackingCamera);

    void SetReconstructor(MR::CScalableSLAMReconstructor& _rReconstructor);

    glm::vec3 Pick(const glm::ivec2& _rCursor);

    void UpdateSelectionBox();

    void AddPositionToSelection(const glm::vec3& _rWSPosition);
    void ResetSelection();
    void SetInpaintedPlane(Gfx::CTexturePtr _Texture, const Base::AABB3Float& _rAABB);

    const Base::AABB3Float& GetSelectionBox();
} // namespace ReconstructionRenderer
} // namespace Gfx
