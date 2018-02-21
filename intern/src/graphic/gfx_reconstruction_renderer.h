//
//  gfx_reconstruction_renderer.h
//  graphic
//
//  Created by Tobias Schwandt on 12/12/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "mr/mr_slam_reconstructor.h"

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
    void Render(int _Pass);
    void PauseIntegration(bool _Paused);
    void PauseTracking(bool _Paused);
    void ChangeCamera(bool _IsTrackingCamera);
} // namespace ReconstructionRenderer
} // namespace Gfx
