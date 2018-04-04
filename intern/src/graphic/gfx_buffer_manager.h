
#pragma once

#include "base/base_typedef.h"

#include "graphic/gfx_buffer.h"
#include "graphic/gfx_buffer_set.h"
#include "graphic/gfx_data_behavior.h"
#include "graphic/gfx_export.h"

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
    GFX_API void OnStart();
    GFX_API void OnExit();

    GFX_API CBufferPtr CreateBuffer(const SBufferDescriptor& _rDescriptor, SDataBehavior::Enum _Behavior = SDataBehavior::LeftAlone);
    GFX_API CBufferPtr GetBuffer(std::type_index _ClassKey);

    GFX_API CBufferSetPtr CreateBufferSet(CBufferPtr _Buffer1Ptr);
    GFX_API CBufferSetPtr CreateBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr);
    GFX_API CBufferSetPtr CreateBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr, CBufferPtr _Buffer3Ptr);
    GFX_API CBufferSetPtr CreateBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr, CBufferPtr _Buffer3Ptr, CBufferPtr _Buffer4Ptr);
    GFX_API CBufferSetPtr CreateBufferSet(CBufferPtr* _pBufferPtrs, unsigned int _NumberOfBuffers);
    
    GFX_API void CopyBufferToBuffer(CBufferPtr _TargetBufferPtr, CBufferPtr _SourceBufferPtr);
    GFX_API void CopyBufferToBuffer(CBufferPtr _TargetBufferPtr, CBufferPtr _SourceBufferPtr, unsigned int _ReadOffset, unsigned int _WriteOffset, unsigned int _Range);
    
    GFX_API void* MapBuffer(CBufferPtr _BufferPtr, CBuffer::EMap _Map);
    GFX_API void* MapBufferRange(CBufferPtr _BufferPtr, CBuffer::EMap _Map, unsigned int _Offset, unsigned int _Range);
    GFX_API void UnmapBuffer(CBufferPtr _BufferPtr);
    
    GFX_API void UploadBufferData(CBufferPtr _BufferPtr, const void* _pData);
    GFX_API void UploadBufferData(CBufferPtr _BufferPtr, const void* _pData, unsigned int _Offset, unsigned int _Range);

    GFX_API void SetBufferLabel(CBufferPtr _BufferPtr, const char* _pLabel);
} // namespace BufferManager
} // namespace Gfx