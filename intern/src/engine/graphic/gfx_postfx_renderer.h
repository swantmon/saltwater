//
//  gfx_postfx_renderer.h
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
    ENGINE_API void OnStart();
    ENGINE_API void OnExit();
    
    ENGINE_API void OnSetupShader();
    ENGINE_API void OnSetupKernels();
    ENGINE_API void OnSetupRenderTargets();
    ENGINE_API void OnSetupStates();
    ENGINE_API void OnSetupTextures();
    ENGINE_API void OnSetupBuffers();
    ENGINE_API void OnSetupModels();
    ENGINE_API void OnSetupResources();
    ENGINE_API void OnSetupEnd();
    
    ENGINE_API void OnReload();
    ENGINE_API void OnNewMap();
    ENGINE_API void OnUnloadMap();
    
    ENGINE_API void Update();
    ENGINE_API void Render();

    ENGINE_API void DebugAlbedo(bool _Flag);
} // namespace PostFX
} // namespace Gfx
