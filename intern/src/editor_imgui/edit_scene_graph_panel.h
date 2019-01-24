
#pragma once

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/data/data_entity.h"

namespace Edit
{
namespace GUI
{
    class CSceneGraphPanel : public Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CSceneGraphPanel)

    public:

        CSceneGraphPanel();
       ~CSceneGraphPanel();

    public:

        void Render();
    };
} // namespace GUI
} // namespace Edit