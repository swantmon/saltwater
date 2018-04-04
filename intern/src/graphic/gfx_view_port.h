
#pragma once

#include "base/base_managed_pool.h"

#include "graphic/gfx_export.h"

namespace Gfx
{
    class CViewPort : public Base::CManagedPoolItemBase
    {
        public:
            
            CViewPort();
            ~CViewPort();
            
        public:

            GFX_API float GetWidth() const;
            GFX_API float GetHeight() const;

            GFX_API float GetMinDepth() const;
            GFX_API float GetMaxDepth() const;

            GFX_API float GetTopLeftX() const;
            GFX_API float GetTopLeftY() const;

        public:

            struct SPort
            {
                float m_TopLeftX;
                float m_TopLeftY;
                float m_Width;
                float m_Height;
                float m_MinDepth;
                float m_MaxDepth;
            };
        
        protected:

            SPort m_Port;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CViewPort> CViewPortPtr;
} // namespace Gfx
