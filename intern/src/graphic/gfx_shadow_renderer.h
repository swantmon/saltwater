//
//  gfx_shadow_renderer.h
//  graphic
//
//  Created by Tobias Schwandt on 26/05/15.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Gfx
{
namespace ShadowRenderer
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
} // namespace ShadowRenderer
} // namespace Gfx