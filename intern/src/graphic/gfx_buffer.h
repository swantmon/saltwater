
#pragma once

#include "base/base_managed_pool.h"

namespace Gfx
{
    class CBuffer : public Base::CManagedPoolItemBase
    {
        public:

            static const unsigned int s_InvalidID = static_cast<unsigned int>(-1);
        
        public:
            
            enum EUsage
            {
                GPUReadWrite     = 0,
                GPURead          = 1,
                GPUReadCPUWrite  = 2,
                GPUToCPU         = 3,
            };
            
            enum EBinding
            {
                NoBinding           = 0,
                VertexBuffer        = 1,
                IndexBuffer         = 2,
                ConstantBuffer      = 3,
                ResourceBuffer      = 4,
				AtomicCounterBuffer = 5,
            };
            
            enum EAccess
            {
                CPUWrite         = 0,
                CPURead          = 1,
                CPUReadWrite     = 2,
                CPUNone          = 3,
            };
            
            enum EMap
            {
                Read             = 0,
                Write            = 1,
                ReadWrite        = 2,
                WriteDiscard     = 3,
                WriteNoOverwrite = 4,
            };
        
        public:

            struct SInfo
            {
                unsigned int m_Stride       : 16;  ///< Defines the number of bytes per unit in the buffer.
                unsigned int m_Binding      :  3;  ///< Defines if the buffer can be bound as index, vertex, or constant buffer.
                unsigned int m_IsBytesOwner :  1;  ///< Defines if the buffer or an extern module is the owner of the CPU data.
                unsigned int m_Usage        :  2;  ///< Defines the driver hint, whether to locate the buffer in CPU or GPU memory.
                unsigned int m_Access       :  2;  ///< Defines if the CPU has read, write, or no access to the GPU data of the buffer.
            };

        public:

            EUsage GetUsage() const;
            EBinding GetBinding() const;
            
            unsigned int GetAccess() const;
            unsigned int GetStride() const;
            
            unsigned int GetNumberOfBytes() const;
            
            void* GetBytes();
            const void* GetBytes() const;
        
        protected:
        
            SInfo             m_Info;             ///< Stores information about the buffer.
            unsigned int      m_NumberOfBytes;    ///< The number of bytes stored in the buffer.
            void*             m_pBytes;           ///< The bytes stored in the buffer.
        
        protected:
            
            CBuffer();
            ~CBuffer();
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CBuffer> CBufferPtr;
} // namespace Gfx
