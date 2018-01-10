
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
    class SGraphicsInfo
    {
    public:

        enum EGraphicAPI
        {
            OpenGL,
            OpenGLES,
        };

        EGraphicAPI m_GraphicsAPI;
        int         m_MajorVersion;
        int         m_MinorVersion;
    };
} // namespace Gfx

namespace Gfx
{
namespace Main
{
    void OnStart();
    void OnExit();

    void RegisterResizeHandler(CResizeDelegate _NewDelgate);

    unsigned int RegisterWindow(void* _pWindow, unsigned int _VSync = 1);

    unsigned int GetNumberOfWindows();

    void ActivateWindow(unsigned int _WindowID);

    const Base::Int2& GetActiveWindowSize();
    const Base::Int2& GetWindowSize(unsigned int _WindowID);

    void OnResize(unsigned int _WindowID, unsigned int _Width, unsigned int _Height);

    void TakeScreenshot(unsigned int _WindowID, const char* _pPathToFile);

    const SGraphicsInfo& GetGraphicsAPI();
    bool IsExtensionAvailable(const std::string& _Name);

    void BeginFrame();
    void EndFrame();
    
    void CreatePerFrameConstantBuffers();
    void DestroyPerFrameConstantBuffers();
    void UploadPerFrameConstantBuffers();
    
    CBufferPtr GetPerFrameConstantBuffer();
} // namespace Main
} // namespace Gfx