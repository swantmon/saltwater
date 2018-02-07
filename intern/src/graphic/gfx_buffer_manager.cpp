
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_managed_pool.h"
#include "base/base_memory.h"
#include "base/base_typedef.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_native_buffer.h"
#include "graphic/gfx_native_buffer_set.h"
#include "graphic/gfx_native_types.h"

#include <assert.h>
#include <exception>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

#ifdef PLATFORM_ANDROID
GfxBufferStorageEXT glBufferStorage = 0;
#endif // PLATFORM_ANDROID

using namespace Gfx;

namespace 
{
    class CGfxBufferManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxBufferManager)
        
    public:

        CGfxBufferManager();

    public:

        void OnStart();
        void OnExit();

    public:

        CBufferPtr CreateBuffer(const SBufferDescriptor& _rDescriptor, SDataBehavior::Enum _Behavior);
        CBufferPtr GetBuffer(std::type_index _ClassKey);

    public:

        CBufferSetPtr CreateBufferSet(CBufferPtr* _pBufferPtrs, unsigned int _NumberOfBuffers);

    public:

        void CopyBufferToBuffer(CBufferPtr _TargetBufferPtr, CBufferPtr _SourceBufferPtr);
        void CopyBufferToBuffer(CBufferPtr _TargetBufferPtr, CBufferPtr _SourceBufferPtr, unsigned int _ReadOffset, unsigned int _WriteOffset, unsigned int _Range);

    public:

        void* MapBuffer(CBufferPtr _BufferPtr, CBuffer::EMap _Map);
        void* MapBufferRange(CBufferPtr _BufferPtr, CBuffer::EMap _Map, unsigned int _Offset, unsigned int _Range);
        void UnmapBuffer(CBufferPtr _BufferPtr);

        void UploadBufferData(CBufferPtr _BufferPtr, const void* _pData);
        void UploadBufferData(CBufferPtr _BufferPtr, const void* _pData, unsigned int _Offset, unsigned int _Range);

    public:

        void SetBufferLabel(CBufferPtr _BufferPtr, const char* _pLabel);

    private:

        class CInternBuffer : public CNativeBuffer
        {
            public:

                CInternBuffer();
               ~CInternBuffer();

            private:

                friend class CGfxBufferManager;
        };

        // -----------------------------------------------------------------------------
        // Represents a unique combination of up to 16 buffers.
        // -----------------------------------------------------------------------------
        class CInternBufferSet : public CNativeBufferSet
        {
            public:

                CInternBufferSet();
               ~CInternBufferSet();

            public:

                bool operator == (const CInternBufferSet& _rBufferSet) const;
                bool operator != (const CInternBufferSet& _rBufferSet) const;

            private:

                friend class CGfxBufferManager;
        };

    
        typedef Base::CManagedPool<CInternBuffer>    CBuffers;
        typedef CBuffers::CIterator                  CBufferIterator;
        typedef Base::CManagedPool<CInternBufferSet> CBufferSets;
        typedef CBufferSets::CIterator               CBufferSetIterator;
    
    private:

        CBuffers    m_Buffers;
        CBufferSets m_BufferSets;

    private:
        
        GLenum ConvertUsage(CBuffer::EUsage _Usage);
        GLenum ConvertBindFlag(CBuffer::EBinding _BindFlag);
        GLenum ConvertAccess(CBuffer::EAccess _Access);
        GLbitfield ConvertMap(CBuffer::EMap _Map);
    };
} // namespace

namespace 
{
    CGfxBufferManager::CInternBuffer::CInternBuffer()
        : CNativeBuffer()
    {
    }

    // -----------------------------------------------------------------------------

    CGfxBufferManager::CInternBuffer::~CInternBuffer()
    {
        // -----------------------------------------------------------------------------
        // Check if the buffer is the owner of CPU pixel data.
        // -----------------------------------------------------------------------------
        if (m_Info.m_IsBytesOwner)
        {
            assert(m_pBytes != nullptr);

            Base::CMemory::Free(m_pBytes);
        }
        
        glDeleteBuffers(1, &m_NativeBuffer);
    }
} // namespace

namespace
{
    CGfxBufferManager::CInternBufferSet::CInternBufferSet()
        : CNativeBufferSet()
    {
    }

    // -----------------------------------------------------------------------------

    CGfxBufferManager::CInternBufferSet::~CInternBufferSet()
    {

    }

    // -----------------------------------------------------------------------------

    bool CGfxBufferManager::CInternBufferSet::operator == (const CInternBufferSet& _rBufferSet) const
    {
        unsigned int IndexOfBuffer;

        if (m_NumberOfBuffers != _rBufferSet.m_NumberOfBuffers)
        {
            return false;
        }

        for (IndexOfBuffer = 0; IndexOfBuffer < m_NumberOfBuffers; ++ IndexOfBuffer)
        {
            if (m_BufferPtrs[IndexOfBuffer] != _rBufferSet.m_BufferPtrs[IndexOfBuffer])
            {
                return false;
            }
        }

        return true;
    }

    // -----------------------------------------------------------------------------

    bool CGfxBufferManager::CInternBufferSet::operator != (const CInternBufferSet& _rBufferSet) const
    {
        unsigned int IndexOfBuffer;

        if (m_NumberOfBuffers != _rBufferSet.m_NumberOfBuffers)
        {
            return true;
        }

        for (IndexOfBuffer = 0; IndexOfBuffer < m_NumberOfBuffers; ++ IndexOfBuffer)
        {
            if (m_BufferPtrs[IndexOfBuffer] != _rBufferSet.m_BufferPtrs[IndexOfBuffer])
            {
                return true;
            }
        }

        return false;
    }
} // namespace 

namespace 
{
    CGfxBufferManager::CGfxBufferManager()
        : m_Buffers         ()
        , m_BufferSets      ()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxBufferManager::OnStart()
    {
#ifdef PLATFORM_ANDROID
        if (Main::IsExtensionAvailable("GL_EXT_buffer_storage"))
        {
            glBufferStorage = reinterpret_cast<GfxBufferStorageEXT>(eglGetProcAddress("glBufferStorageEXT"));
        }
        else
        {
            BASE_THROWM("GL_EXT_buffer_storage is not supported but it is highly needed!");
        }
#endif
    }

    // -----------------------------------------------------------------------------

    void CGfxBufferManager::OnExit()
    {
        // -----------------------------------------------------------------------------
        // First release the sets to decrease the reference counters of the buffers.
        // -----------------------------------------------------------------------------
        m_BufferSets.Clear();
        m_Buffers   .Clear();
    }

    // -----------------------------------------------------------------------------

    CBufferPtr CGfxBufferManager::CreateBuffer(const SBufferDescriptor& _rDescriptor, SDataBehavior::Enum _Behavior)
    {
        void*                    pBytes;
        GLenum                   NativeBinding;
        int                      NativeUsage;
        GLbitfield               Flags;
        Gfx::CNativeBufferHandle NativeBuffer;

        // -----------------------------------------------------------------------------
        // Setup variables for exception safety.
        // -----------------------------------------------------------------------------

        pBytes        = nullptr;
        NativeBuffer  = 0;
        NativeBinding = ConvertBindFlag(_rDescriptor.m_Binding);
        NativeUsage   = 0;
        
        // -----------------------------------------------------------------------------
        // Generate OpenGL buffer
        // -----------------------------------------------------------------------------
        glGenBuffers(1, &NativeBuffer);

        glBindBuffer(NativeBinding, NativeBuffer);

        // -----------------------------------------------------------------------------
        // Setup storage of buffer.
        // If pBytes is NULL, a data store of the specified size is still created,
        // but its contents remain uninitialized and thus undefined.
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_Usage == CBuffer::Persistent)
        {
            Flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_DYNAMIC_STORAGE_BIT;

            glBufferStorage(NativeBinding, _rDescriptor.m_NumberOfBytes, _rDescriptor.m_pBytes, Flags);
        }
        else
        {
            NativeUsage = ConvertUsage(_rDescriptor.m_Usage);

            glBufferData(NativeBinding, _rDescriptor.m_NumberOfBytes, _rDescriptor.m_pBytes, NativeUsage);
        }

        // ----------------------------------------------------------------------------- 
        // Unbound buffer now 
        // ----------------------------------------------------------------------------- 
        glBindBuffer(NativeBinding, 0);
               
        // -----------------------------------------------------------------------------
        // Create the core resource behavior on the owner policy.
        // -----------------------------------------------------------------------------
        CBuffers::CPtr BufferPtr;
        
        try
        {
            BufferPtr = m_Buffers.Allocate();
            
            CInternBuffer& rBuffer = *BufferPtr;
            
            // -----------------------------------------------------------------------------
            // Setup the buffer.
            // -----------------------------------------------------------------------------
            rBuffer.m_Info.m_Stride  = _rDescriptor.m_Stride;
            rBuffer.m_Info.m_Usage   = _rDescriptor.m_Usage;
            rBuffer.m_Info.m_Binding = _rDescriptor.m_Binding;
            rBuffer.m_Info.m_Access  = _rDescriptor.m_Access;
            rBuffer.m_NumberOfBytes  = _rDescriptor.m_NumberOfBytes;
            rBuffer.m_NativeBuffer   = NativeBuffer;
            rBuffer.m_NativeBinding  = NativeBinding;
            rBuffer.m_NativeUsage    = NativeUsage;
            rBuffer.m_pStorage       = nullptr;

            // -----------------------------------------------------------------------------
            // Check the behavior.
            // -----------------------------------------------------------------------------
            if (_Behavior == SDataBehavior::Copy || _Behavior == SDataBehavior::CopyAndDelete)
            {
                pBytes = Base::CMemory::Allocate(_rDescriptor.m_NumberOfBytes);
                
                assert(pBytes                       != nullptr);
                assert(_rDescriptor.m_pBytes        != nullptr);
                assert(_rDescriptor.m_NumberOfBytes  > 0);

                memcpy(pBytes, _rDescriptor.m_pBytes, _rDescriptor.m_NumberOfBytes);
            }

            switch (_Behavior)
            {
                case SDataBehavior::LeftAlone:
                {
                    rBuffer.m_Info.m_IsBytesOwner = false;
                    rBuffer.m_pBytes              = nullptr;
                }
                break;

                case SDataBehavior::DeleteAfterUpload:
                {
                    rBuffer.m_Info.m_IsBytesOwner = false;
                    rBuffer.m_pBytes              = nullptr;

                    Base::CMemory::Free(_rDescriptor.m_pBytes);
                }
                break;

                case SDataBehavior::TakeOwnerShip:
                {
                    rBuffer.m_Info.m_IsBytesOwner = true;
                    rBuffer.m_pBytes              = _rDescriptor.m_pBytes;
                }
                break;

                case SDataBehavior::Copy:
                {
                    rBuffer.m_Info.m_IsBytesOwner = true;
                    rBuffer.m_pBytes              = pBytes;
                }
                break;

                case SDataBehavior::CopyAndDelete:
                {
                    rBuffer.m_Info.m_IsBytesOwner = true;
                    rBuffer.m_pBytes              = pBytes;

                    Base::CMemory::Free(_rDescriptor.m_pBytes);
                }
                break;
                    
                default:
                    BASE_CONSOLE_STREAMWARNING("Undefined buffer behavior while creating an buffer.");
                break;
            }
        }
        catch (...)
        {
            if (pBytes != nullptr)
            {
                Base::CMemory::Free(pBytes);
            }

            glDeleteBuffers(1, &NativeBuffer);

            BASE_THROWV("GL buffer number %i can't be created.", m_Buffers.GetNumberOfItems());
        }
    
        return CBufferPtr(BufferPtr);
    }

    // -----------------------------------------------------------------------------

    CBufferPtr CGfxBufferManager::GetBuffer(std::type_index _ClassKey)
    {
        BASE_UNUSED(_ClassKey);

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    CBufferSetPtr CGfxBufferManager::CreateBufferSet(CBufferPtr* _pBufferPtrs, unsigned int _NumberOfBuffers)
    {
        CBufferPtr BufferPtr;
        
        // -----------------------------------------------------------------------------
        // Allocate item in buffer
        // -----------------------------------------------------------------------------
        CBufferSets::CPtr BufferSetPtr = m_BufferSets.Allocate();
        
        CInternBufferSet& rBufferSet = *BufferSetPtr;
        
        // -----------------------------------------------------------------------------
        // Fill internal buffer set with general data
        // -----------------------------------------------------------------------------
        rBufferSet.m_NumberOfBuffers = _NumberOfBuffers;
        
        // -----------------------------------------------------------------------------
        // Setup internal buffer set with buffer information and define buffer array
        // by binding all buffers to it.
        // -----------------------------------------------------------------------------
        for (unsigned int CurrentBuffer = 0; CurrentBuffer < _NumberOfBuffers; ++CurrentBuffer)
        {
            BufferPtr = _pBufferPtrs[CurrentBuffer];
            
            rBufferSet.m_BufferPtrs[CurrentBuffer] = _pBufferPtrs[CurrentBuffer];
        }
        
        return CBufferSetPtr(BufferSetPtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxBufferManager::CopyBufferToBuffer(CBufferPtr _TargetBufferPtr, CBufferPtr _SourceBufferPtr)
    {
        if (_TargetBufferPtr != _SourceBufferPtr)
        {
            CInternBuffer& rTargetBuffer = *static_cast<CInternBuffer*>(&(*_TargetBufferPtr));
            CInternBuffer& rSourceBuffer = *static_cast<CInternBuffer*>(&(*_SourceBufferPtr));
            
            unsigned int NumberOfBytes = glm::min(rTargetBuffer.m_NumberOfBytes, rSourceBuffer.m_NumberOfBytes);

            glCopyBufferSubData(rTargetBuffer.m_NativeBuffer, rSourceBuffer.m_NativeBuffer, 0, 0, NumberOfBytes);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxBufferManager::CopyBufferToBuffer(CBufferPtr _TargetBufferPtr, CBufferPtr _SourceBufferPtr, unsigned int _ReadOffset, unsigned int _WriteOffset, unsigned int _Range)
    {
        if (_TargetBufferPtr != _SourceBufferPtr)
        {
            CInternBuffer& rTargetBuffer = *static_cast<CInternBuffer*>(&(*_TargetBufferPtr));
            CInternBuffer& rSourceBuffer = *static_cast<CInternBuffer*>(&(*_SourceBufferPtr));

            assert(_ReadOffset + _Range <= rTargetBuffer.m_NumberOfBytes);
            assert(_WriteOffset + _Range <= rSourceBuffer.m_NumberOfBytes);

            glCopyBufferSubData(rTargetBuffer.m_NativeBuffer, rSourceBuffer.m_NativeBuffer, _ReadOffset, _WriteOffset, _Range);
        }
    }

    // -----------------------------------------------------------------------------

    void* CGfxBufferManager::MapBuffer(CBufferPtr _BufferPtr, CBuffer::EMap _Map)
    {
        assert(_BufferPtr != nullptr && _BufferPtr.IsValid());

        CInternBuffer* pBuffer = static_cast<CInternBuffer*>(_BufferPtr.GetPtr());

        assert(pBuffer != nullptr);

        GLbitfield NativeMap     = ConvertMap(_Map);
        GLenum     NativeBinding = ConvertBindFlag(pBuffer->GetBinding());
 
        glBindBuffer(NativeBinding, pBuffer->m_NativeBuffer);

        return pBuffer->m_pStorage = glMapBufferRange(NativeBinding, 0, pBuffer->m_NumberOfBytes, NativeMap);
    }

    // -----------------------------------------------------------------------------

    void* CGfxBufferManager::MapBufferRange(CBufferPtr _BufferPtr, CBuffer::EMap _Map, unsigned int _Offset, unsigned int _Range)
    {
        assert(_BufferPtr != nullptr && _BufferPtr.IsValid());

        CInternBuffer* pBuffer = static_cast<CInternBuffer*>(_BufferPtr.GetPtr());

        assert(pBuffer != nullptr);

        GLbitfield NativeMap = ConvertMap(_Map);
        GLenum     NativeBinding = ConvertBindFlag(pBuffer->GetBinding());

        glBindBuffer(NativeBinding, pBuffer->m_NativeBuffer);

        return pBuffer->m_pStorage = glMapBufferRange(NativeBinding, _Offset, _Range, NativeMap);
    }

    // -----------------------------------------------------------------------------

    void CGfxBufferManager::UnmapBuffer(CBufferPtr _BufferPtr)
    {
        assert(_BufferPtr != nullptr && _BufferPtr.IsValid());

        CInternBuffer* pBuffer = static_cast<CInternBuffer*>(_BufferPtr.GetPtr());

        assert(pBuffer != nullptr);

        GLenum NativeBinding = ConvertBindFlag(pBuffer->GetBinding());

        glUnmapBuffer(NativeBinding);

        pBuffer->m_pStorage = nullptr;
    }
    
    // -----------------------------------------------------------------------------

    void CGfxBufferManager::UploadBufferData(CBufferPtr _BufferPtr, const void* _pData)
    {
        assert(_BufferPtr != nullptr && _BufferPtr.IsValid() && _pData);

        CInternBuffer* pBuffer = static_cast<CInternBuffer*>(_BufferPtr.GetPtr());

        assert(pBuffer != nullptr);

        GLenum NativeBinding = ConvertBindFlag(pBuffer->GetBinding());

        glBindBuffer(NativeBinding, pBuffer->m_NativeBuffer);

        glBufferSubData(NativeBinding, 0, pBuffer->m_NumberOfBytes, _pData);
    }

    // -----------------------------------------------------------------------------

    void CGfxBufferManager::UploadBufferData(CBufferPtr _BufferPtr, const void* _pData, unsigned int _Offset, unsigned int _Range)
    {
        assert(_BufferPtr != nullptr && _BufferPtr.IsValid() && _pData && _Range > 0);

        CInternBuffer* pBuffer = static_cast<CInternBuffer*>(_BufferPtr.GetPtr());

        assert(pBuffer != nullptr);
        assert(_Offset + _Range <= pBuffer->m_NumberOfBytes);

        GLenum NativeBinding = ConvertBindFlag(pBuffer->GetBinding());

        glBindBuffer(NativeBinding, pBuffer->m_NativeBuffer);

        glBufferSubData(NativeBinding, _Offset, _Range, _pData);
    }

    // -----------------------------------------------------------------------------

    void CGfxBufferManager::SetBufferLabel(CBufferPtr _BufferPtr, const char* _pLabel)
    {
        assert(_pLabel != nullptr);

        CInternBuffer* pInternBuffer = static_cast<CInternBuffer*>(_BufferPtr.GetPtr());

        glObjectLabel(GL_BUFFER, pInternBuffer->m_NativeBuffer, -1, _pLabel);
    }

    // -----------------------------------------------------------------------------

    GLenum  CGfxBufferManager::ConvertUsage(CBuffer::EUsage _Usage)
    {
        // -----------------------------------------------------------------------------
        // Usage is much more complicated then this four possibilities. In OpenGL
        // you can mix between these features:
        // @see: http://www.opengl.org/wiki/GLAPI/glBufferData
        //
        // STREAM
        // The data store contents will be modified once and used at most a few times.
        //
        // STATIC
        // The data store contents will be modified once and used many times.
        //
        // DYNAMIC
        // The data store contents will be modified repeatedly and used many times.
        // The nature of access may be one of these:
        //
        // +
        // 
        // DRAW
        // The data store contents are modified by the application, and used as the source for GL drawing and image specification commands.
        //
        // READ
        // The data store contents are modified by reading data from the GL, and used to return that data when queried by the application.
        //
        // COPY
        // The data store contents are modified by reading data from the GL, and used as the source for GL drawing and image specification commands.
        // -----------------------------------------------------------------------------
        static const GLenum  s_NativeUsage[] =
        {
            GL_DYNAMIC_DRAW,        //< GPUReadWrite
            GL_STATIC_DRAW,         //< GPURead
            GL_STREAM_DRAW,         //< GPUReadCPUWrite
            GL_STREAM_DRAW,         //< GPUToCPU
        };
        
        return s_NativeUsage[_Usage];
    }

    // -----------------------------------------------------------------------------

    GLenum  CGfxBufferManager::ConvertBindFlag(CBuffer::EBinding _BindFlag)
    {
        // -----------------------------------------------------------------------------
        // Much more:
        // @see: http://www.opengl.org/wiki/GLAPI/glBindBuffer
        // -----------------------------------------------------------------------------
        static const GLenum  s_NativeBindFlag[] =
        {
            static_cast<GLenum >(0),    //> No binding
            GL_ARRAY_BUFFER,            //> Vertices
            GL_ELEMENT_ARRAY_BUFFER,    //> Indices
            GL_UNIFORM_BUFFER,          //> Constant Buffer
            GL_SHADER_STORAGE_BUFFER,   //> Resource Buffer
            GL_ATOMIC_COUNTER_BUFFER,   //> Atomic Counter
        };

        return s_NativeBindFlag[_BindFlag];
    }
                            
    // -----------------------------------------------------------------------------
    
    GLenum  CGfxBufferManager::ConvertAccess(CBuffer::EAccess _Access)
    {
        // -----------------------------------------------------------------------------
        // Much more:
        // @see: http://www.opengl.org/sdk/docs/man/html/glMapBuffer.xhtml
        // -----------------------------------------------------------------------------
        static const GLenum s_NativeAccessFlag[] =
        {
            GL_WRITE_ONLY,
            GL_READ_ONLY,
            GL_READ_WRITE,
            static_cast<GLenum >(0),
        };
        
        return s_NativeAccessFlag[_Access];
    }

    // -----------------------------------------------------------------------------

    GLbitfield CGfxBufferManager::ConvertMap(CBuffer::EMap _Map)
    {
        static const GLenum s_NativeMap[] =
        {
            GL_MAP_READ_BIT,
            GL_MAP_WRITE_BIT,
            GL_MAP_READ_BIT | GL_MAP_WRITE_BIT,
            GL_MAP_READ_BIT,
            GL_MAP_WRITE_BIT,
            GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT,
        };

        return s_NativeMap[_Map];
    }
} // namespace

namespace Gfx
{
namespace BufferManager
{
    void OnStart()
    {
        CGfxBufferManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxBufferManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    CBufferPtr CreateBuffer(const SBufferDescriptor& _rDescriptor, SDataBehavior::Enum _Behavior)
    {
        return CGfxBufferManager::GetInstance().CreateBuffer(_rDescriptor, _Behavior);
    }

    // -----------------------------------------------------------------------------

    CBufferPtr GetBuffer(std::type_index _ClassKey)
    {
        return CGfxBufferManager::GetInstance().GetBuffer(_ClassKey);
    }

    // -----------------------------------------------------------------------------

    CBufferSetPtr CreateBufferSet(CBufferPtr _Buffer1Ptr)
    {
        CBufferPtr BufferPtrs[] = { _Buffer1Ptr, };

        return CGfxBufferManager::GetInstance().CreateBufferSet(BufferPtrs, 1);
    }

    // -----------------------------------------------------------------------------

    CBufferSetPtr CreateBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr)
    {
        CBufferPtr BufferPtrs[] = { _Buffer1Ptr, _Buffer2Ptr, };

        return CGfxBufferManager::GetInstance().CreateBufferSet(BufferPtrs, 2);
    }

    // -----------------------------------------------------------------------------

    CBufferSetPtr CreateBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr, CBufferPtr _Buffer3Ptr)
    {
        CBufferPtr BufferPtrs[] = { _Buffer1Ptr, _Buffer2Ptr, _Buffer3Ptr, };

        return CGfxBufferManager::GetInstance().CreateBufferSet(BufferPtrs, 3);
    }

    // -----------------------------------------------------------------------------

    CBufferSetPtr CreateBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr, CBufferPtr _Buffer3Ptr, CBufferPtr _Buffer4Ptr)
    {
        CBufferPtr BufferPtrs[] = { _Buffer1Ptr, _Buffer2Ptr, _Buffer3Ptr, _Buffer4Ptr, };

        return CGfxBufferManager::GetInstance().CreateBufferSet(BufferPtrs, 4);
    }

    // -----------------------------------------------------------------------------

    CBufferSetPtr CreateBufferSet(CBufferPtr* _pBufferPtrs, unsigned int _NumberOfBuffers)
    {
        return CGfxBufferManager::GetInstance().CreateBufferSet(_pBufferPtrs, _NumberOfBuffers);
    }

    // -----------------------------------------------------------------------------

    void CopyBufferToBuffer(CBufferPtr _TargetBufferPtr, CBufferPtr _SourceBufferPtr)
    {
        CGfxBufferManager::GetInstance().CopyBufferToBuffer(_TargetBufferPtr, _SourceBufferPtr);
    }

    // -----------------------------------------------------------------------------

    void CopyBufferToBuffer(CBufferPtr _TargetBufferPtr, CBufferPtr _SourceBufferPtr, unsigned int _ReadOffset, unsigned int _WriteOffset, unsigned int _Range)
    {
        CGfxBufferManager::GetInstance().CopyBufferToBuffer(_TargetBufferPtr, _SourceBufferPtr, _ReadOffset, _WriteOffset, _Range);
    }
    
    // -----------------------------------------------------------------------------

    void* MapBuffer(CBufferPtr _BufferPtr, CBuffer::EMap _Map)
    {
        return CGfxBufferManager::GetInstance().MapBuffer(_BufferPtr, _Map);
    }

    // -----------------------------------------------------------------------------

    void* MapBufferRange(CBufferPtr _BufferPtr, CBuffer::EMap _Map, unsigned int _Offset, unsigned int _Range)
    {
        return CGfxBufferManager::GetInstance().MapBufferRange(_BufferPtr, _Map, _Offset, _Range);
    }

    // -----------------------------------------------------------------------------

    void UnmapBuffer(CBufferPtr _BufferPtr)
    {
        CGfxBufferManager::GetInstance().UnmapBuffer(_BufferPtr);
    }

    // -----------------------------------------------------------------------------

    void UploadBufferData(CBufferPtr _BufferPtr, const void* _pData)
    {
        CGfxBufferManager::GetInstance().UploadBufferData(_BufferPtr, _pData);
    }

    // -----------------------------------------------------------------------------

    void UploadBufferData(CBufferPtr _BufferPtr, const void* _pData, unsigned int _Offset, unsigned int _Range)
    {
        CGfxBufferManager::GetInstance().UploadBufferData(_BufferPtr, _pData, _Offset, _Range);
    }

    // -----------------------------------------------------------------------------

    void SetBufferLabel(CBufferPtr _BufferPtr, const char* _pLabel)
    {
        CGfxBufferManager::GetInstance().SetBufferLabel(_BufferPtr, _pLabel);
    }
} // namespace BufferManager
} // namespace Gfx