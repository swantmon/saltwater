//
//  gfx_light_indirect_renderer.h
//  graphic
//
//  Created by Tobias Schwandt on 02/12/16.
//  Copyright (c) 2016 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Gfx
{
namespace LightIndirectRenderer
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
} // namespace LightIndirectRenderer
} // namespace Gfx
