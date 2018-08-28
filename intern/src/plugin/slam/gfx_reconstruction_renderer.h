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
    void OnReconstructionUpdate(const MR::SReconstructionSettings& _Settings);
    void OnNewMap();
    void OnUnloadMap();
    
    void Update();
    void Render();
    void PauseIntegration(bool _Paused);
    void PauseTracking(bool _Paused);
    void ChangeCamera(bool _IsTrackingCamera);
    float GetReconstructionSize();

    MR::CScalableSLAMReconstructor& GetReconstructor();
    glm::vec3 Pick(const glm::ivec2& _rCursor);
} // namespace ReconstructionRenderer
} // namespace Gfx
