//
//  gfx_sky_renderer.h
//  graphic
//
//  Created by Tobias Schwandt on 02/03/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "graphic/gfx_texture_set.h"

namespace Gfx
{
namespace SkyRenderer
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
} // namespace SkyRenderer
} // namespace Gfx
