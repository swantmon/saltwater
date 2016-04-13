
#pragma once

#include "base/base_managed_pool.h"

#include "graphic/gfx_buffer.h"

namespace Gfx
{
    class CBufferSet : public Base::CManagedPoolItemBase
    {
    public:

        static const unsigned int s_MaxNumberOfBuffers = 16;

    public:

        unsigned int GetNumberOfBuffers() const;
        
        CBufferPtr GetBuffer(unsigned int _Index);

        bool ContainsBuffer(CBufferPtr _BufferPtr) const;

    public:

        const unsigned int* GetStrides() const;

    protected:

        unsigned int  m_NumberOfBuffers;
        CBufferPtr    m_BufferPtrs[s_MaxNumberOfBuffers];
        unsigned int  m_Strides[s_MaxNumberOfBuffers];

    protected:

        CBufferSet();
       ~CBufferSet();
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CBufferSet> CBufferSetPtr;
} // namespace Gfx
