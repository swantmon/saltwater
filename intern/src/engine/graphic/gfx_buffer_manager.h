
#pragma once

#include "engine/engine_config.h"

#include "base/base_typedef.h"

#include "engine/graphic/gfx_buffer.h"
#include "engine/graphic/gfx_buffer_set.h"
#include "engine/graphic/gfx_data_behavior.h"

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
        unsigned long long    m_NumberOfBytes;
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

    ENGINE_API CBufferPtr CreateBuffer(const SBufferDescriptor& _rDescriptor, SDataBehavior::Enum _Behavior = SDataBehavior::LeftAlone);
    ENGINE_API CBufferPtr GetBuffer(std::type_index _ClassKey);

    ENGINE_API CBufferSetPtr CreateBufferSet(CBufferPtr _Buffer1Ptr);
    ENGINE_API CBufferSetPtr CreateBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr);
    ENGINE_API CBufferSetPtr CreateBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr, CBufferPtr _Buffer3Ptr);
    ENGINE_API CBufferSetPtr CreateBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr, CBufferPtr _Buffer3Ptr, CBufferPtr _Buffer4Ptr);
    ENGINE_API CBufferSetPtr CreateBufferSet(CBufferPtr* _pBufferPtrs, unsigned int _NumberOfBuffers);
    
    ENGINE_API void CopyBufferToBuffer(CBufferPtr _TargetBufferPtr, CBufferPtr _SourceBufferPtr);
    ENGINE_API void CopyBufferToBuffer(CBufferPtr _TargetBufferPtr, CBufferPtr _SourceBufferPtr, unsigned int _ReadOffset, unsigned int _WriteOffset, unsigned int _Range);
    
    ENGINE_API void* MapBuffer(CBufferPtr _BufferPtr, CBuffer::EMap _Map);
    ENGINE_API void* MapBufferRange(CBufferPtr _BufferPtr, CBuffer::EMap _Map, unsigned int _Offset, unsigned int _Range);
    ENGINE_API void UnmapBuffer(CBufferPtr _BufferPtr);
    
    ENGINE_API void UploadBufferData(CBufferPtr _BufferPtr, const void* _pData);
    ENGINE_API void UploadBufferData(CBufferPtr _BufferPtr, const void* _pData, unsigned int _Offset, unsigned int _Range);

    ENGINE_API void SetBufferLabel(CBufferPtr _BufferPtr, const char* _pLabel);
} // namespace BufferManager
} // namespace Gfx