
#pragma once

namespace Gfx
{
namespace App
{
    unsigned int RegisterWindow(void* _pWindow, int _VSync = 0);

    void ActivateWindow(unsigned int _WindowID);

    void OnResize(unsigned int _WindowID, unsigned int _Width, unsigned int _Height);

    void TakeScreenshot(unsigned int _WindowID, const char* _pPathToFile);
} // namespace App
} // namespace Gfx
