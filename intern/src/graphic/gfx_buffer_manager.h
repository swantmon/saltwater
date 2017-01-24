
#pragma once

#include "base/base_typedef.h"

#include "graphic/gfx_buffer.h"
#include "graphic/gfx_buffer_set.h"
#include "graphic/gfx_data_behavior.h"

#include <typeinfo>
#include <typeindex>

namespace Gfx
{
    struct SBufferDescriptor
    {
        unsigned int          m_Stride;
        CBuffer::EUsage       m_Usage;
        CBuffer::EBinding     m_Binding;
        CBuffer::EAccess      m_Access;
        unsigned int          m_NumberOfBytes;
        void*                 m_pBytes;
        const std::type_info* m_pClassKey;
    };
} // namespace Gfx

namespace Gfx
{
namespace BufferManager
{
    void OnStart();
    void OnExit();

    CBufferPtr CreateBuffer(const SBufferDescriptor& _rDescriptor, SDataBehavior::Enum _Behavior = SDataBehavior::LeftAlone);
    CBufferPtr GetBuffer(std::type_index _ClassKey);

    CBufferSetPtr CreateBufferSet(CBufferPtr _Buffer1Ptr);
    CBufferSetPtr CreateBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr);
    CBufferSetPtr CreateBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr, CBufferPtr _Buffer3Ptr);
    CBufferSetPtr CreateBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr, CBufferPtr _Buffer3Ptr, CBufferPtr _Buffer4Ptr);
    CBufferSetPtr CreateBufferSet(CBufferPtr* _pBufferPtrs, unsigned int _NumberOfBuffers);
    
    CBufferSetPtr CreateVertexBufferSet(CBufferPtr _Buffer1Ptr);
    CBufferSetPtr CreateVertexBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr);
    CBufferSetPtr CreateVertexBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr, CBufferPtr _Buffer3Ptr);
    CBufferSetPtr CreateVertexBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr, CBufferPtr _Buffer3Ptr, CBufferPtr _Buffer4Ptr);
    CBufferSetPtr CreateVertexBufferSet(CBufferPtr* _pBufferPtrs, unsigned int _NumberOfBuffers);
    
    void CopyBufferToBuffer(CBufferPtr _TargetBufferPtr, CBufferPtr _SourceBufferPtr);

    void* MapVertexBuffer(CBufferPtr _BufferPtr, CBuffer::EMap _Map);
    void UnmapVertexBuffer(CBufferPtr _BufferPtr);

    void* MapIndexBuffer(CBufferPtr _BufferPtr, CBuffer::EMap _Map);
    void UnmapIndexBuffer(CBufferPtr _BufferPtr);

    void* MapConstantBuffer(CBufferPtr _BufferPtr, CBuffer::EMap _Map);
    void UnmapConstantBuffer(CBufferPtr _BufferPtr);

    void UploadVertexBufferData(CBufferPtr _BufferPtr, const void* _pData);
    void UploadConstantBufferData(CBufferPtr _BufferPtr, const void* _pData);
} // namespace BufferManager
} // namespace Gfx