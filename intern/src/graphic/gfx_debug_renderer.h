//
//  gfx_debug_renderer.h
//  graphic
//
//  Created by Tobias Schwandt on 03/12/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_aabb2.h"
#include "base/base_vector2.h"
#include "base/base_vector3.h"

#include "graphic/gfx_camera.h"
#include "graphic/gfx_texture_base.h"

#include <string>

namespace Gfx
{
namespace DebugRenderer
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

    void DrawCamera(Gfx::CCameraPtr _CameraPtr);
    void DrawGizmo(bool _Flag);
    void DrawTexture(Gfx::CTextureBasePtr _TexturePtr, const Base::AABB2Float& _rScreenRegion);
    void DrawText(const std::string& _rText, const Base::Float2& _rScreenPosition, const Base::Float4& _rColor, unsigned int _TextSize);
} // namespace DebugRenderer
} // namespace Gfx
