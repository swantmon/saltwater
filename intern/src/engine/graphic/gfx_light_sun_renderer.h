//
//  gfx_light_sun_renderer.h
//  graphic
//
//  Created by Tobias Schwandt on 24/03/16.
//  Copyright (c) 2016 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Gfx
{
namespace LightSunRenderer
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
} // namespace LightSunRenderer
} // namespace Gfx
