
#pragma once

#include "base/base_singleton.h"

#include "editor/edit_panel_interface.h"

#include <vector>

namespace Edit
{
namespace GUI
{
    class CConsolePanel : public IPanel
    {
        BASE_SINGLETON_FUNC(CConsolePanel)

    public:

        CConsolePanel();
       ~CConsolePanel();

    public:

        void Render();

        const char* GetName();
    };
} // namespace GUI
} // namespace Edit