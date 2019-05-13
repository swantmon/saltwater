
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

        void Render() override;

        const char* GetName() override;

    private:

        Dt::CEntity* m_pEntity;
    };
} // namespace GUI
} // namespace Edit