//
//  gfx_reflection_renderer.h
//  graphic
//
//  Created by Tobias Schwandt on 04/06/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Gfx
{
namespace ReflectionRenderer
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

    CTextureBasePtr GetBRDF();
} // namespace ReflectionRenderer
} // namespace Gfx