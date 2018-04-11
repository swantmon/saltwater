//
//  gfx_particle_renderer.h
//  graphic
//
//  Created by Tobias Schwandt on 28/01/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Gfx
{
namespace ParticleRenderer
{
    void OnStart();
    void OnExit();
    
    void OnSetupShader();
    void OnSetupKernels();
    void OnSetupRenderTargets();
    void OnSetupStates();
    void OnSetupTextures();
    void OnSetupBuffers();
    void OnSetupResources();
    void OnSetupModels();
    void OnSetupEnd();
    
    void OnReload();
    void OnNewMap();
    void OnUnloadMap();
    
    void Update();
    void Render();
} // namespace ParticleRenderer
} // namespace Gfx
