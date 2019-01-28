
#pragma once

#include "base/base_singleton.h"

#include "editor/edit_panel_interface.h"

#include "engine/data/data_entity.h"

namespace Edit
{
namespace GUI
{
    class CInspectorPanel : public IPanel
    {
        BASE_SINGLETON_FUNC(CInspectorPanel)

    public:

        CInspectorPanel();
       ~CInspectorPanel();

    public:

        void InspectEntity(Dt::CEntity::BID _ID);

    public:

        void Render();

        const char* GetName();

    private:

        Dt::CEntity* m_pEntity;
        std::string m_SearchString;
    };
} // namespace GUI
} // namespace Edit