
#pragma once

#include "engine/engine_config.h"

#include "base/base_managed_pool.h"

namespace Gfx
{
    class CViewPort : public Base::CManagedPoolItemBase
    {
        public:
            
            CViewPort();
            ~CViewPort();
            
        public:

            ENGINE_API float GetWidth() const;
            ENGINE_API float GetHeight() const;

            ENGINE_API float GetMinDepth() const;
            ENGINE_API float GetMaxDepth() const;

            ENGINE_API float GetTopLeftX() const;
            ENGINE_API float GetTopLeftY() const;

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
