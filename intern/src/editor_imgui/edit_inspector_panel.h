
#pragma once

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/data/data_entity.h"

namespace Edit
{
namespace GUI
{
    class CInspectorPanel : public Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CInspectorPanel)

    public:

        CInspectorPanel();
       ~CInspectorPanel();

    public:

        void InspectEntity(int _ID);

    public:

        void Render();

    private:

        Dt::CEntity* m_pEntity;
    };
} // namespace GUI
} // namespace Edit