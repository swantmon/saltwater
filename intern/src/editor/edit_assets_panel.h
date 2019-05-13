
#pragma once

#include "base/base_singleton.h"

#include "editor/edit_panel_interface.h"

#include "engine/data/data_entity.h"

#include <vector>
#include <map>

namespace Edit
{
namespace GUI
{
    class CAssetsPanel : public IPanel
    {
        BASE_SINGLETON_FUNC(CAssetsPanel)

    public:

        CAssetsPanel();
       ~CAssetsPanel();

    public:

        void Render() override;

        const char* GetName() override;

    };
} // namespace GUI
} // namespace Edit