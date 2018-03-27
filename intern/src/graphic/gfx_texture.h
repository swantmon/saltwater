
#pragma once

#include "base/base_managed_pool.h"
#include "base/base_typedef.h"

#include <string>

namespace Gfx
{
    class CTexture : public Base::CManagedPoolItemBase
    {
        public:
        
            enum EDimension
            {
                Dim2D,
                Dim3D,
                External
            };
        
            enum EFormat
            {
                R8_BYTE,
                R8G8_BYTE,
                R8G8B8_BYTE,
                R8G8B8A8_BYTE,
                R8_UBYTE,
                R8G8_UBYTE,
                R8G8B8_UBYTE,
                R8G8B8A8_UBYTE,
                R8_SHORT,
                R8G8_SHORT,
                R8G8B8_SHORT,
                R8G8B8A8_SHORT,
                R8_USHORT,
                R8G8_USHORT,
                R8G8B8_USHORT,
                R8G8B8A8_USHORT,
                R8_INT,
                R8G8_INT,
                R8G8B8_INT,
                R8G8B8A8_INT,
                R8_UINT,
                R8G8_UINT,
                R8G8B8_UINT,
                R8G8B8A8_UINT,
                
                R16_BYTE,
                R16G16_BYTE,
                R16G16B16_BYTE,
                R16G16B16A16_BYTE,
                R16_UBYTE,
                R16G16_UBYTE,
                R16G16B16_UBYTE,
                R16G16B16A16_UBYTE,
                R16_SHORT,
                R16G16_SHORT,
                R16G16B16_SHORT,
                R16G16B16A16_SHORT,
                R16_USHORT,
                R16G16_USHORT,
                R16G16B16_USHORT,
                R16G16B16A16_USHORT,
                R16_INT,
                R16G16_INT,
                R16G16B16_INT,
                R16G16B16A16_INT,
                R16_UINT,
                R16G16_UINT,
                R16G16B16_UINT,
                R16G16B16A16_UINT,
                R16_FLOAT,
                R16G16_FLOAT,
                R16G16B16_FLOAT,
                R16G16B16A16_FLOAT,
                
                R32_INT,
                R32G32_INT,
                R32G32B32_INT,
                R32G32B32A32_INT,
                R32_UINT,
                R32G32_UINT,
                R32G32B32_UINT,
                R32G32B32A32_UINT,
                R32_FLOAT,
                R32G32_FLOAT,
                R32G32B32_FLOAT,
                R32G32B32A32_FLOAT,
                
                R3G3B2_UBYTE,
                R4G4B4A4_USHORT,
                R5G5G5A1_USHORT,
                R10G10B10A2_UINT,
                
                Unknown = -1,
            };

            enum ESemantic
            {
                Diffuse,
                Specular,
                Normal,
                Height,
                HDR,
                NumberOfSemantics,
                UndefinedSemantic = -1,
            };

            enum EUsage
            {
                GPUReadWrite       =  0,
                GPURead            =  1,
                GPUReadCPUWrite    =  2,
                GPUToCPU           =  3,
            };

            enum EBinding
            {
                ShaderResource     =  1,
                RenderTarget       =  2,
                DepthStencilTarget =  4,
            };

            enum EAccess
            {
                CPUWrite           =  1,
                CPURead            =  2,
            };

        public:

            typedef Base::U16 BPixels;

        public:

            EDimension GetDimension() const;
            EFormat GetFormat() const;
            EUsage GetUsage() const;
            EAccess GetAccess() const;
            ESemantic GetSemantic() const;
            unsigned int GetBinding() const;
            unsigned int GetNumberOfTextures() const;
            unsigned int GetNumberOfMipLevels() const;
            unsigned int GetCurrentMipLevel() const;
            BPixels GetNumberOfPixelsU() const;
            BPixels GetNumberOfPixelsV() const;
            BPixels GetNumberOfPixelsW() const;

            bool IsArray() const;
            bool IsCube() const;
            bool IsDummy() const;

            void* GetPixels();
            const void* GetPixels() const;

            const std::string& GetFileName() const;

            unsigned int GetHash() const;

            virtual unsigned int GetNativeHandle() const = 0;

        protected:

            enum EUninitialized
            {
                Uninitialized,
            };

        protected:

            struct SInfo
            {
                Base::U32 m_Format            : 8;   ///< Defines the format of the texture.
                Base::U32 m_NumberOfTextures  : 8;   ///< The number of textures in case the texture is a texture array or a cube texture (otherwise the number is 1).
                Base::U32 m_NumberOfMipLevels : 4;   ///< The number of mip levels (at least 1).
                Base::U32 m_CurrentMipLevel   : 4;   ///< The current mip level of this texture (default is 0).
                Base::U32 m_Access            : 2;   ///< Defines if the CPU has read, write, or no access to the GPU data of the texture.
                Base::U32 m_Dimension         : 2;   ///< Defines the dimension of the texture.
                Base::U32 m_Usage             : 2;   ///< Defines the driver hint, whether to locate the texture in CPU or GPU memory.
                Base::U32 m_IsDeletable       : 1;   ///< Defines if the owner of the texture is the manager or an extern module.
                Base::U32 m_IsPixelOwner      : 1;   ///< Defines if the texture or an extern module is the owner of the CPU pixel data.
                Base::U32 m_IsCubeTexture     : 1;   ///< Defines if the texture is a cube texture.
                Base::U32 m_IsDummyTexture    : 1;   ///< Defines if the texture is a dummy returned if the creation of the real texture has failes.
                Base::U32 m_Binding           : 3;   ///< Defines if the texture can be bound as shader resource, render target, or depth stencil target.
                Base::U32 m_Semantic          : 8;   ///< Defines the semantic meaning of the texture if used as a shader resource.
            };
        
        protected:

            BPixels      m_NumberOfPixels[3];
            SInfo        m_Info;
            void*        m_pPixels;
            std::string  m_FileName;
            unsigned int m_Hash;

        protected:

            CTexture();
           ~CTexture();
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CTexture> CTexturePtr;
} // namespace Gfx
