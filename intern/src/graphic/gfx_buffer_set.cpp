
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_buffer_set.h"

#include <assert.h>

namespace Gfx
{
    CBufferSet::CBufferSet()
        : m_NumberOfBuffers(0)
    {
        unsigned int IndexofBuffer;

        for (IndexofBuffer = 0; IndexofBuffer < s_MaxNumberOfBuffers; ++ IndexofBuffer)
        {
            m_Strides[IndexofBuffer] = 0;
        }
    }

    // -----------------------------------------------------------------------------

    CBufferSet::~CBufferSet()
    {
    }

    // -----------------------------------------------------------------------------

    unsigned int CBufferSet::GetNumberOfBuffers() const
    {
        return m_NumberOfBuffers;
    }

    // -----------------------------------------------------------------------------

    CBufferPtr CBufferSet::GetBuffer(unsigned int _Index)
    {
        assert(_Index < m_NumberOfBuffers);

        return m_BufferPtrs[_Index];
    }

    // -----------------------------------------------------------------------------

    bool CBufferSet::ContainsBuffer(CBufferPtr _BufferPtr) const
    {
        unsigned int IndexOfBuffer;

        assert(_BufferPtr != 0);

        for (IndexOfBuffer = 0; IndexOfBuffer < m_NumberOfBuffers; ++ IndexOfBuffer)
        {
            if (m_BufferPtrs[IndexOfBuffer] == _BufferPtr)
            {
                return true;
            }
        }

        return false;
    }

    // -----------------------------------------------------------------------------

    const unsigned int* CBufferSet::GetStrides() const
    {
        return m_Strides;
    }
} // namespace Gfx
