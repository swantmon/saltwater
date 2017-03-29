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
    void OnReconstructionUpdate(const MR::CSLAMReconstructor::SReconstructionSettings& _Settings);
    void OnNewMap();
    void OnUnloadMap();
    
    void Update();
    void Render();
} // namespace ReconstructionRenderer
} // namespace Gfx
