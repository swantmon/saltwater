//
//  gfx_selection_renderer.h
//  graphic
//
//  Created by Tobias Schwandt on 17/11/16.
//  Copyright (c) 2016 TU Ilmenau. All rights reserved.
//

#pragma once

namespace Gfx
{
namespace SelectionRenderer
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
    
    void Update();
    void Render();

    void SelectEntity(unsigned int _EntityID);
    void UnselectEntity();
} // namespace SelectionRenderer
} // namespace Gfx
