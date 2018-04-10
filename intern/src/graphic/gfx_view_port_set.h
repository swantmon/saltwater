
#pragma once

#include "engine/engine_config.h"

#include "base/base_managed_pool.h"

#include "graphic/gfx_view_port.h"

namespace Gfx
{
    class CViewPortSet : public Base::CManagedPoolItemBase
    {
        public:

            static const unsigned int s_MaxNumberOfViewPorts = 16;

        public:

            ENGINE_API unsigned int GetNumberOfViewPorts() const;

        public:

            ENGINE_API CViewPortPtr* GetViewPorts();
            ENGINE_API const CViewPortPtr* GetViewPorts() const;

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
