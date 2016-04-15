
#include "graphic/gfx_application_interface.h"
#include "graphic/gfx_main.h"

namespace Gfx
{
namespace App
{
    unsigned int RegisterWindow(void* _pWindow)
    {
        assert(_pWindow != 0);

        return Gfx::Main::RegisterWindow(_pWindow);
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
} // namespace App
} // namespace Gfx