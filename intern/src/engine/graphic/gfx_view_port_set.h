
#pragma once

#include "engine/engine_config.h"

#include "base/base_managed_pool.h"

#include "engine/graphic/gfx_view_port.h"

namespace Gfx
{
    class ENGINE_API CViewPortSet : public Base::CManagedPoolItemBase
    {
        public:

            static const unsigned int s_MaxNumberOfViewPorts = 16;

        public:

            unsigned int GetNumberOfViewPorts() const;

        public:

            CViewPortPtr* GetViewPorts();
            const CViewPortPtr* GetViewPorts() const;

        protected:

            unsigned int     m_NumberOfViewPorts;
            CViewPortPtr     m_ViewPortPtrs[s_MaxNumberOfViewPorts];
            CViewPort::SPort m_ViewPorts[s_MaxNumberOfViewPorts];

        protected:

            CViewPortSet();
           ~CViewPortSet();
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CViewPortSet> CViewPortSetPtr;
} // namespace Gfx
