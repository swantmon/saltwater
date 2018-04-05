
#include "engine/engine_precompiled.h"

#include "graphic/gfx_application_interface.h"
#include "graphic/gfx_main.h"

namespace Gfx
{
namespace App
{
    unsigned int RegisterWindow(void* _pWindow, int _VSync)
    {
        assert(_pWindow != 0);

        return Gfx::Main::RegisterWindow(_pWindow, _VSync);
    }

    // -----------------------------------------------------------------------------

    void ActivateWindow(unsigned int _WindowID)
    {
        Gfx::Main::ActivateWindow(_WindowID);
    }

    // -----------------------------------------------------------------------------

    void OnResize(unsigned int _WindowID, unsigned int _Width, unsigned int _Height)
    {
        Gfx::Main::OnResize(_WindowID, _Width, _Height);
    }

    // -----------------------------------------------------------------------------

    void TakeScreenshot(unsigned int _WindowID, const char* _pPathToFile)
    {
        Gfx::Main::TakeScreenshot(_WindowID, _pPathToFile);
    }
} // namespace App
} // namespace Gfx