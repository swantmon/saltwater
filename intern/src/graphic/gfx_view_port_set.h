
#pragma once

#include "base/base_managed_pool.h"

#include "graphic/gfx_export.h"
#include "graphic/gfx_view_port.h"

namespace Gfx
{
    class CViewPortSet : public Base::CManagedPoolItemBase
    {
        public:

            static const unsigned int s_MaxNumberOfViewPorts = 16;

        public:

            GFX_API unsigned int GetNumberOfViewPorts() const;

        public:

            GFX_API CViewPortPtr* GetViewPorts();
            GFX_API const CViewPortPtr* GetViewPorts() const;

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
