
#pragma once

#include "engine/engine_config.h"

namespace Gfx
{
namespace App
{
    ENGINE_API unsigned int RegisterWindow(void* _pWindow, int _VSync = 0);

    ENGINE_API void ActivateWindow(unsigned int _WindowID);

    ENGINE_API void OnResize(unsigned int _WindowID, unsigned int _Width, unsigned int _Height);

    ENGINE_API void TakeScreenshot(unsigned int _WindowID, const char* _pPathToFile);
} // namespace App
} // namespace Gfx
