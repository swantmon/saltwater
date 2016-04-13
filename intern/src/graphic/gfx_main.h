
//
//  gfx_main.h
//  gfx
//
//  Created by Tobias Schwandt on 18/09/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_vector2.h"

#include "graphic/gfx_buffer.h"

#include <functional>

namespace Gfx
{
namespace Main
{
    typedef std::function<void(int, int)> CResizeDelegate;
} // namespace Main
} // namespace Gfx

#define GFX_BIND_RESIZE_METHOD(_Method) std::bind(_Method, this, std::placeholders::_1, std::placeholders::_2)

namespace Gfx
{
namespace Main
{
    void OnStart(int _Width, int _Height);
    void OnExit();
    
    void OnResize(int _Width, int _Height);
    
    Base::Int2 GetScreenSize();
    
    void RegisterResizeHandler(CResizeDelegate _NewDelgate);
    
    void BeginFrame();
    void EndFrame();
    
    unsigned int GetFrame();
    
    void CreatePerFrameConstantBuffers();
    void DestroyPerFrameConstantBuffers();
    void UploadPerFrameConstantBuffers();
    
    CBufferPtr GetPerFrameConstantBufferVS();
    CBufferPtr GetPerFrameConstantBufferHS();
    CBufferPtr GetPerFrameConstantBufferDS();
    CBufferPtr GetPerFrameConstantBufferGS();
    CBufferPtr GetPerFrameConstantBufferPS();
} // namespace Main
} // namespace Gfx