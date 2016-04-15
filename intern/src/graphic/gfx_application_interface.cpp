
#include "graphic/gfx_application_interface.h"
#include "graphic/gfx_main.h"

namespace Gfx
{
namespace App
{
    void ActivateWindow(unsigned int _WindowID)
    {
        Gfx::Main::ActivateWindow(_WindowID);
    }

    // -----------------------------------------------------------------------------

    unsigned int RegisterWindow(void* _pWindow)
    {
        assert(_pWindow != 0);

        return Gfx::Main::RegisterWindow(_pWindow);
    }
} // namespace App
} // namespace Gfx