//
//  gfx_post_effect_after_tonemapping_renderer.h
//  graphic
//
//  Created by Tobias Schwandt on 12/12/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Gfx
{
namespace PostFX
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
} // namespace PostFX
} // namespace Gfx
