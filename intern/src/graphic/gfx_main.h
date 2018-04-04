
#pragma once

#include "base/base_include_glm.h"

#include "graphic/gfx_buffer.h"
#include "graphic/gfx_export.h"
#include "graphic/gfx_graphics_info.h"

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
    GFX_API void OnStart();
    GFX_API void OnExit();

    GFX_API void RegisterResizeHandler(CResizeDelegate _NewDelgate);

    GFX_API unsigned int RegisterWindow(void* _pWindow, int _VSync = 1);

    GFX_API unsigned int GetNumberOfWindows();

    GFX_API void ActivateWindow(unsigned int _WindowID);

    GFX_API const glm::ivec2& GetActiveWindowSize();
    GFX_API const glm::ivec2& GetWindowSize(unsigned int _WindowID);

    GFX_API const glm::ivec2& GetActiveNativeWindowSize();
    GFX_API const glm::ivec2& GetNativeWindowSize(unsigned int _WindowID);

    GFX_API void OnResize(unsigned int _WindowID, unsigned int _Width, unsigned int _Height);

    GFX_API void TakeScreenshot(unsigned int _WindowID, const char* _pPathToFile);

    GFX_API const CGraphicsInfo& GetGraphicsAPI();
    GFX_API bool IsExtensionAvailable(const std::string& _Name);

    GFX_API void BeginFrame();
    GFX_API void EndFrame();
    
    GFX_API void CreatePerFrameConstantBuffers();
    GFX_API void DestroyPerFrameConstantBuffers();
    GFX_API void UploadPerFrameConstantBuffers();
    
    GFX_API CBufferPtr GetPerFrameConstantBuffer();
} // namespace Main
} // namespace Gfx