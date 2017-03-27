
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_managed_pool.h"
#include "base/base_math_operations.h"
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

#include "GL/glew.h"

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
        CBufferSetPtr CreateVertexBufferSet(CBufferPtr* _pBufferPtrs, unsigned int _NumberOfBuffers);

    public:

        void CopyBufferToBuffer(CBufferPtr _TargetBufferPtr, CBufferPtr _SourceBufferPtr);

    public:

        void* MapVertexBuffer(CBufferPtr _BufferPtr, CBuffer::EMap _Map);
        void UnmapVertexBuffer(CBufferPtr _BufferPtr);

        void* MapIndexBuffer(CBufferPtr _BufferPtr, CBuffer::EMap _Map);
        void UnmapIndexBuffer(CBufferPtr _BufferPtr);

        void* MapConstantBuffer(CBufferPtr _BufferPtr, CBuffer::EMap _Map);
        void* MapConstantBufferRange(CBufferPtr _BufferPtr, CBuffer::EMap _Map, Base::Size _Range);
        void UnmapConstantBuffer(CBufferPtr _BufferPtr);

        void UploadVertexBufferData(CBufferPtr _BufferPtr, const void* _pData);
        void UploadConstantBufferData(CBufferPtr _BufferPtr, const void* _pData);

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
        GLenum ConvertMap(CBuffer::EMap _Map);
        GLenum ConvertMapRange(CBuffer::EMap _Map);
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
        if (m_NativeBufferArrayHandle != 0)
        {
            glDeleteVertexArrays(1, &m_NativeBufferArrayHandle);
        }
    }

    // -----------------------------------------------------------------------------

    bool CGfxBufferManager::CInternBufferSet::operator == (const CInternBufferSet& _rBufferSet) const
    {
        unsigned int IndexOfBuffer;

        if (m_NumberOfBuffers != _rBufferSet.m_NumberOfBuffers)
        {
            return false;
        }
        
        if (m_NativeBufferArrayHandle != _rBufferSet.m_NativeBufferArrayHandle)
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
        
        if (m_NativeBufferArrayHandle != _rBufferSet.m_NativeBufferArrayHandle)
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
        int                      NativeBinding;
        int                      NativeUsage;
        Gfx::CNativeBufferHandle NativeBuffer;

        // -----------------------------------------------------------------------------
        // Setup variables for exception safety.
        // -----------------------------------------------------------------------------
        pBytes         = nullptr;
        NativeBuffer   = 0;
        NativeBinding  = ConvertBindFlag(_rDescriptor.m_Binding);
        NativeUsage    = ConvertUsage(_rDescriptor.m_Usage);
        
        // -----------------------------------------------------------------------------
        // Generate OpenGL buffer
        // -----------------------------------------------------------------------------
        glCreateBuffers(1, &NativeBuffer);
               
        // -----------------------------------------------------------------------------
        // Setup storage of buffer.
        // If pBytes is NULL, a data store of the specified size is still created,
        // but its contents remain uninitialized and thus undefined.
        // -----------------------------------------------------------------------------
        glNamedBufferData(NativeBuffer, _rDescriptor.m_NumberOfBytes, _rDescriptor.m_pBytes, NativeUsage);
               
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
        CBufferPtr         BufferPtr;
        
        // -----------------------------------------------------------------------------
        // Allocate item in buffer
        // -----------------------------------------------------------------------------
        CBufferSets::CPtr BufferSetPtr = m_BufferSets.Allocate();
        
        CInternBufferSet& rBufferSet = *BufferSetPtr;
        
        // -----------------------------------------------------------------------------
        // Fill internal buffer set with general data
        // -----------------------------------------------------------------------------
        rBufferSet.m_NumberOfBuffers         = _NumberOfBuffers;
        rBufferSet.m_NativeBufferArrayHandle = 0;
        
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
    
    CBufferSetPtr CGfxBufferManager::CreateVertexBufferSet(CBufferPtr* _pBufferPtrs, unsigned int _NumberOfBuffers)
    {
        CInternBuffer*           pBuffer;
        CBufferPtr               BufferPtr;
        Gfx::CNativeBufferHandle BufferArrayHandle;
        
        // -----------------------------------------------------------------------------
        // Allocate item in buffer
        // -----------------------------------------------------------------------------
        CBufferSets::CPtr BufferSetPtr = m_BufferSets.Allocate();
        
        CInternBufferSet& rBufferSet = *BufferSetPtr;
        
        // -----------------------------------------------------------------------------
        // Setup an buffer array object as a set of buffer
        // -----------------------------------------------------------------------------
        glCreateVertexArrays(1, &BufferArrayHandle);
        
        glBindVertexArray(BufferArrayHandle);
        
        // -----------------------------------------------------------------------------
        // Fill internal buffer set with general data
        // -----------------------------------------------------------------------------
        rBufferSet.m_NumberOfBuffers         = _NumberOfBuffers;
        rBufferSet.m_NativeBufferArrayHandle = BufferArrayHandle;
        
        // -----------------------------------------------------------------------------
        // Setup internal bufferset with buffer information and define buffer array
        // by binding all buffers to it.
        // -----------------------------------------------------------------------------
        for (unsigned int CurrentBuffer = 0; CurrentBuffer < _NumberOfBuffers; ++CurrentBuffer)
        {
            BufferPtr = _pBufferPtrs[CurrentBuffer];
            pBuffer   = static_cast<CInternBuffer*>(BufferPtr.GetPtr());
            
            rBufferSet.m_BufferPtrs[CurrentBuffer] = _pBufferPtrs[CurrentBuffer];
            
            glBindBuffer(ConvertBindFlag(pBuffer->GetBinding()), pBuffer->m_NativeBuffer);
        }
        
        // -----------------------------------------------------------------------------
        // Unbind new vertex array object
        // -----------------------------------------------------------------------------
        glBindVertexArray(0);
        
        return CBufferSetPtr(BufferSetPtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxBufferManager::CopyBufferToBuffer(CBufferPtr _TargetBufferPtr, CBufferPtr _SourceBufferPtr)
    {
        if (_TargetBufferPtr != _SourceBufferPtr)
        {
            CInternBuffer& rTargetBuffer = *static_cast<CInternBuffer*>(&(*_TargetBufferPtr));
            CInternBuffer& rSourceBuffer = *static_cast<CInternBuffer*>(&(*_SourceBufferPtr));
            
            unsigned int NumberOfBytes = Base::Min(rTargetBuffer.m_NumberOfBytes, rSourceBuffer.m_NumberOfBytes);
            
            glCopyNamedBufferSubData(rTargetBuffer.m_NativeBuffer, rSourceBuffer.m_NativeBuffer, 0, 0, NumberOfBytes);
        }
    }

    // -----------------------------------------------------------------------------

    void* CGfxBufferManager::MapVertexBuffer(CBufferPtr _BufferPtr, CBuffer::EMap _Map)
    {
        assert(_BufferPtr != nullptr && _BufferPtr.IsValid());

        CInternBuffer* pBuffer = static_cast<CInternBuffer*>(_BufferPtr.GetPtr());

        assert(pBuffer != nullptr);

        int NativeMap = ConvertMap(_Map);

        return glMapNamedBuffer(pBuffer->m_NativeBuffer, NativeMap);
    }

    // -----------------------------------------------------------------------------

    void CGfxBufferManager::UnmapVertexBuffer(CBufferPtr _BufferPtr)
    {
        assert(_BufferPtr != nullptr && _BufferPtr.IsValid());

        CInternBuffer* pBuffer = static_cast<CInternBuffer*>(_BufferPtr.GetPtr());

        assert(pBuffer != nullptr);

        glUnmapNamedBuffer(pBuffer->m_NativeBuffer);
    }

    // -----------------------------------------------------------------------------

    void* CGfxBufferManager::MapIndexBuffer(CBufferPtr _BufferPtr, CBuffer::EMap _Map)
    {
        assert(_BufferPtr != nullptr && _BufferPtr.IsValid());

        BASE_UNUSED(_Map);

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxBufferManager::UnmapIndexBuffer(CBufferPtr _BufferPtr)
    {
        assert(_BufferPtr != nullptr && _BufferPtr.IsValid());
    }

    // -----------------------------------------------------------------------------

    void* CGfxBufferManager::MapConstantBuffer(CBufferPtr _BufferPtr, CBuffer::EMap _Map)
    {
        assert(_BufferPtr != nullptr && _BufferPtr.IsValid());

        CInternBuffer* pBuffer = static_cast<CInternBuffer*>(_BufferPtr.GetPtr());

        assert(pBuffer != nullptr);

        int NativeMap = ConvertMap(_Map);

        return glMapNamedBuffer(pBuffer->m_NativeBuffer, NativeMap);
    }

    // -----------------------------------------------------------------------------

    void* CGfxBufferManager::MapConstantBufferRange(CBufferPtr _BufferPtr, CBuffer::EMap _Map, Base::Size _Range)
    {
        assert(_BufferPtr != nullptr && _BufferPtr.IsValid());
        assert(_Range > 0);

        CInternBuffer* pBuffer = static_cast<CInternBuffer*>(_BufferPtr.GetPtr());

        assert(pBuffer != nullptr);

        int NativeMap = ConvertMapRange(_Map);

        return glMapNamedBufferRange(pBuffer->m_NativeBuffer, 0, _Range, NativeMap);
    }

    // -----------------------------------------------------------------------------

    void CGfxBufferManager::UnmapConstantBuffer(CBufferPtr _BufferPtr)
    {
        assert(_BufferPtr != nullptr && _BufferPtr.IsValid());

        CInternBuffer* pBuffer = static_cast<CInternBuffer*>(_BufferPtr.GetPtr());

        assert(pBuffer != nullptr);

        glUnmapNamedBuffer(pBuffer->m_NativeBuffer);
    }

    // -----------------------------------------------------------------------------

    void CGfxBufferManager::UploadVertexBufferData(CBufferPtr _BufferPtr, const void* _pData)
    {
        assert(_BufferPtr != nullptr && _BufferPtr.IsValid());

        GLenum Binding = ConvertBindFlag(_BufferPtr->GetBinding());

        CInternBuffer* pBuffer = static_cast<CInternBuffer*>(_BufferPtr.GetPtr());

        assert(pBuffer != nullptr);

        glNamedBufferSubData(pBuffer->m_NativeBuffer, 0, pBuffer->m_NumberOfBytes, _pData);
    }

    // -----------------------------------------------------------------------------

    void CGfxBufferManager::UploadConstantBufferData(CBufferPtr _BufferPtr, const void* _pData)
    {
        assert(_BufferPtr != nullptr && _BufferPtr.IsValid() && _pData);

        CInternBuffer* pBuffer = static_cast<CInternBuffer*>(_BufferPtr.GetPtr());

        assert(pBuffer != nullptr);

        glNamedBufferSubData(pBuffer->m_NativeBuffer, 0, pBuffer->m_NumberOfBytes, _pData);
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

    GLenum  CGfxBufferManager::ConvertMap(CBuffer::EMap _Map)
    {
        static const GLenum  s_NativeMap[] =
        {
            GL_READ_ONLY,
            GL_WRITE_ONLY,
            GL_READ_WRITE,
            GL_WRITE_ONLY,
            GL_WRITE_ONLY,
        };

        return s_NativeMap[_Map];
    }

    // -----------------------------------------------------------------------------

    GLenum  CGfxBufferManager::ConvertMapRange(CBuffer::EMap _Map)
    {
        static const GLenum  s_NativeMap[] =
        {
            GL_MAP_READ_BIT,
            GL_MAP_WRITE_BIT,
            GL_MAP_READ_BIT,
            GL_MAP_WRITE_BIT,
            GL_MAP_WRITE_BIT,
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
    
    CBufferSetPtr CreateVertexBufferSet(CBufferPtr _Buffer1Ptr)
    {
        CBufferPtr BufferPtrs[] = { _Buffer1Ptr, };
        
        return CGfxBufferManager::GetInstance().CreateVertexBufferSet(BufferPtrs, 1);
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferSetPtr CreateVertexBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr)
    {
        CBufferPtr BufferPtrs[] = { _Buffer1Ptr, _Buffer2Ptr, };
        
        return CGfxBufferManager::GetInstance().CreateVertexBufferSet(BufferPtrs, 2);
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferSetPtr CreateVertexBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr, CBufferPtr _Buffer3Ptr)
    {
        CBufferPtr BufferPtrs[] = { _Buffer1Ptr, _Buffer2Ptr, _Buffer3Ptr, };
        
        return CGfxBufferManager::GetInstance().CreateVertexBufferSet(BufferPtrs, 3);
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferSetPtr CreateVertexBufferSet(CBufferPtr _Buffer1Ptr, CBufferPtr _Buffer2Ptr, CBufferPtr _Buffer3Ptr, CBufferPtr _Buffer4Ptr)
    {
        CBufferPtr BufferPtrs[] = { _Buffer1Ptr, _Buffer2Ptr, _Buffer3Ptr, _Buffer4Ptr, };
        
        return CGfxBufferManager::GetInstance().CreateVertexBufferSet(BufferPtrs, 4);
    }
    
    // -----------------------------------------------------------------------------
    
    CBufferSetPtr CreateVertexBufferSet(CBufferPtr* _pBufferPtrs, unsigned int _NumberOfBuffers)
    {
        return CGfxBufferManager::GetInstance().CreateVertexBufferSet(_pBufferPtrs, _NumberOfBuffers);
    }

    // -----------------------------------------------------------------------------

    void CopyBufferToBuffer(CBufferPtr _TargetBufferPtr, CBufferPtr _SourceBufferPtr)
    {
        CGfxBufferManager::GetInstance().CopyBufferToBuffer(_TargetBufferPtr, _SourceBufferPtr);
    }

    // -----------------------------------------------------------------------------

    void* MapVertexBuffer(CBufferPtr _BufferPtr, CBuffer::EMap _Map)
    {
        return CGfxBufferManager::GetInstance().MapVertexBuffer(_BufferPtr, _Map);
    }

    // -----------------------------------------------------------------------------

    void UnmapVertexBuffer(CBufferPtr _BufferPtr)
    {
        CGfxBufferManager::GetInstance().UnmapVertexBuffer(_BufferPtr);
    }

    // -----------------------------------------------------------------------------

    void* MapIndexBuffer(CBufferPtr _BufferPtr, CBuffer::EMap _Map)
    {
        return CGfxBufferManager::GetInstance().MapIndexBuffer(_BufferPtr, _Map);
    }

    // -----------------------------------------------------------------------------

    void UnmapIndexBuffer(CBufferPtr _BufferPtr)
    {
        CGfxBufferManager::GetInstance().UnmapIndexBuffer(_BufferPtr);
    }

    // -----------------------------------------------------------------------------

    void* MapConstantBuffer(CBufferPtr _BufferPtr, CBuffer::EMap _Map)
    {
        return CGfxBufferManager::GetInstance().MapConstantBuffer(_BufferPtr, _Map);
    }

    // -----------------------------------------------------------------------------

    void* MapConstantBufferRange(CBufferPtr _BufferPtr, CBuffer::EMap _Map, Base::Size _Range)
    {
        return CGfxBufferManager::GetInstance().MapConstantBufferRange(_BufferPtr, _Map, _Range);
    }

    // -----------------------------------------------------------------------------

    void UnmapConstantBuffer(CBufferPtr _BufferPtr)
    {
        CGfxBufferManager::GetInstance().UnmapConstantBuffer(_BufferPtr);
    }

    // -----------------------------------------------------------------------------

    void UploadVertexBufferData(CBufferPtr _BufferPtr, const void* _pData)
    {
        CGfxBufferManager::GetInstance().UploadVertexBufferData(_BufferPtr, _pData);
    }

    // -----------------------------------------------------------------------------

    void UploadConstantBufferData(CBufferPtr _BufferPtr, const void* _pData)
    {
        CGfxBufferManager::GetInstance().UploadConstantBufferData(_BufferPtr, _pData);
    }
} // namespace BufferManager
} // namespace Gfx