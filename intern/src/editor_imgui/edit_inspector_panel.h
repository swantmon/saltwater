
#pragma once

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

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

        void Render();
    };
} // namespace GUI
} // namespace Edit