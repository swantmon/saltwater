//
//  gfx_fog_renderer.h
//  graphic
//
//  Created by Tobias Schwandt on 20/07/16.
//  Copyright (c) 2016 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Gfx
{
namespace FogRenderer
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
} // namespace FogRenderer
} // namespace Gfx
