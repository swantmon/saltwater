
#pragma once

#include "base/base_singleton.h"

#include "editor/edit_panel_interface.h"

#include <vector>

namespace Edit
{
namespace GUI
{
    class CToolbarPanel : public IPanel
    {
        BASE_SINGLETON_FUNC(CToolbarPanel)

    public:

        CToolbarPanel();
       ~CToolbarPanel();

    public:

        void Render() override;

        const char* GetName() override;

    private:

        std::vector<float> m_FrameTimings;
    };
} // namespace GUI
} // namespace Edit