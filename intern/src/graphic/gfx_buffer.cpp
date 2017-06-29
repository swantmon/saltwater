
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_buffer.h"

namespace Gfx
{
    CBuffer::CBuffer()
        : m_pStorage(nullptr)
    {
    }

    // -----------------------------------------------------------------------------

    CBuffer::~CBuffer()
    {
    }

    // -----------------------------------------------------------------------------
    
    CBuffer::EUsage CBuffer::GetUsage() const
    {
        return static_cast<EUsage>(m_Info.m_Usage);
    }
    
    // -----------------------------------------------------------------------------
    
    CBuffer::EBinding CBuffer::GetBinding() const
    {
        return static_cast<EBinding>(m_Info.m_Binding);
    }
    
    // -----------------------------------------------------------------------------
    
    unsigned int CBuffer::GetNumberOfBytes() const
    {
        return m_NumberOfBytes;
    }
    
    // -----------------------------------------------------------------------------
    
    unsigned int CBuffer::GetAccess() const
    {
        return m_Info.m_Access;
    }
    
    // -----------------------------------------------------------------------------
    
    unsigned int CBuffer::GetStride() const
    {
        return m_Info.m_Stride;
    }
    
    // -----------------------------------------------------------------------------
    
    void* CBuffer::GetBytes()
    {
        return m_pBytes;
    }
    
    // -----------------------------------------------------------------------------
    
    const void* CBuffer::GetBytes() const
    {
        return m_pBytes;
    }

    // -----------------------------------------------------------------------------

    void* CBuffer::GetStorage()
    {
        assert(m_pStorage != nullptr);
        return m_pStorage;
    }
} // namespace Gfx