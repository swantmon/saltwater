
#pragma once

#include "base/base_singleton.h"

#include "editor_imgui/edit_panel_interface.h"

#include "engine/data/data_entity.h"

#include <vector>
#include <map>

namespace Edit
{
namespace GUI
{
    class CSceneGraphPanel : public IPanel
    {
        BASE_SINGLETON_FUNC(CSceneGraphPanel)

    public:

        CSceneGraphPanel();
       ~CSceneGraphPanel();

    public:

        void Render();

        const char* GetName();

    private:

        struct SItemState
        {
            int Depth;
            Dt::CEntity* pEntity;
        };

    private:

        std::vector<SItemState> m_ItemState;
        std::map<Dt::CEntity::BID, bool> m_SelectionState;

    };
} // namespace GUI
} // namespace Edit