#pragma once

#include "plugin/slam/mr_slam_reconstructor.h"

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
} // namespace ReconstructionRenderer
} // namespace Gfx
