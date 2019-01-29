
#pragma once

#include "base/base_singleton.h"

#include "editor/edit_panel_interface.h"

#include <vector>

namespace Edit
{
namespace GUI
{
    class CInfosPanel : public IPanel
    {
        BASE_SINGLETON_FUNC(CInfosPanel)

    public:

        CInfosPanel();
       ~CInfosPanel();

    public:

        void Render();

        const char* GetName();

    private:

        std::vector<float> m_FrameTimings;
    };
} // namespace GUI
} // namespace Edit