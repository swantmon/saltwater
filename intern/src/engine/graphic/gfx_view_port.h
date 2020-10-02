
#pragma once

#include "engine/engine_config.h"

#include "base/base_managed_pool.h"

namespace Gfx
{
    class ENGINE_API CViewPort : public Base::CManagedPoolItemBase
    {
        public:
            
            CViewPort();
            ~CViewPort();
            
        public:

            float GetWidth() const;
            float GetHeight() const;

            float GetMinDepth() const;
            float GetMaxDepth() const;

            float GetTopLeftX() const;
            float GetTopLeftY() const;

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
