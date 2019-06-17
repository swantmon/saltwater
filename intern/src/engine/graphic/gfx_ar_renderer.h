//
//  gfx_ar_renderer.h
//  graphic
//
//  Created by Tobias Schwandt on 18/11/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Gfx
{
namespace ARRenderer
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
    void RenderHitProxy();
} // namespace ARRenderer
} // namespace Gfx
