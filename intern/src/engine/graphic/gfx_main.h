
#pragma once

#include "engine/engine_config.h"

#include "base/base_include_glm.h"

#include "engine/graphic/gfx_buffer.h"
#include "engine/graphic/gfx_graphics_info.h"

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
    ENGINE_API void OnStart();
    ENGINE_API void OnExit();

    ENGINE_API void RegisterResizeHandler(CResizeDelegate _NewDelgate);

    ENGINE_API unsigned int RegisterWindow(void* _pWindow, int _VSync = 1);

    ENGINE_API unsigned int GetNumberOfWindows();

    ENGINE_API void ActivateWindow(unsigned int _WindowID);

    ENGINE_API const glm::ivec2& GetActiveWindowSize();
    ENGINE_API const glm::ivec2& GetWindowSize(unsigned int _WindowID);

    ENGINE_API const glm::ivec2& GetActiveNativeWindowSize();
    ENGINE_API const glm::ivec2& GetNativeWindowSize(unsigned int _WindowID);

    ENGINE_API void OnResize(unsigned int _WindowID, unsigned int _Width, unsigned int _Height);

    ENGINE_API void TakeScreenshot(unsigned int _WindowID, const char* _pPathToFile);

    ENGINE_API const CGraphicsInfo& GetGraphicsAPI();
    ENGINE_API bool IsExtensionAvailable(const std::string& _Name);

    ENGINE_API void BeginFrame();
    ENGINE_API void EndFrame();
    
    ENGINE_API void CreatePerFrameConstantBuffers();
    ENGINE_API void DestroyPerFrameConstantBuffers();
    ENGINE_API void UploadPerFrameConstantBuffers();
    
    ENGINE_API CBufferPtr GetPerFrameConstantBuffer();
} // namespace Main
} // namespace Gfx