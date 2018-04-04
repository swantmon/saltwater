
#pragma once

#include "graphic/gfx_export.h"

namespace Gfx
{
namespace App
{
    GFX_API unsigned int RegisterWindow(void* _pWindow, int _VSync = 0);

    GFX_API void ActivateWindow(unsigned int _WindowID);

    GFX_API void OnResize(unsigned int _WindowID, unsigned int _Width, unsigned int _Height);

    GFX_API void TakeScreenshot(unsigned int _WindowID, const char* _pPathToFile);
} // namespace App
} // namespace Gfx
