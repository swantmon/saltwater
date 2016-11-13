
#pragma once

#include "base/base_string.h"
#include "base/base_typedef.h"

namespace Dt
{
    class CTextureBase
    {
        public:
        
            enum EDimension
            {
                Dim1D,
                Dim2D,
                Dim3D
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
                Normal,
                Height,
                HDR,
                NumberOfSemantics,
                UndefinedSemantic = -1,
            };

            enum EDirtyFlags
            {
                DirtyCreate  = 0x01,
                DirtyFile    = 0x02,
                DirtyData    = 0x04,
                DirtyInfo    = 0x08,
                DirtyDestroy = 0x10,
            };

        public:

            typedef Base::U16 BPixels;

        public:

            EDimension GetDimension() const;
            EFormat GetFormat() const;
            ESemantic GetSemantic() const;

            bool IsArray() const;
            bool IsCube() const;
            bool IsDummy() const;

            void* GetPixels();
            const void* GetPixels() const;

            const Base::Char* GetFileName() const;

            const Base::Char* GetIdentifier() const;

            unsigned int GetHash() const;

            unsigned int GetDirtyFlags() const;

            Base::U64 GetDirtyTime() const;

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
                Base::U32 m_Dimension         : 2;   ///< Defines the dimension of the texture.
                Base::U32 m_IsDeletable       : 1;   ///< Defines if the owner of the texture is the manager or an extern module.
                Base::U32 m_IsPixelOwner      : 1;   ///< Defines if the texture or an extern module is the owner of the CPU pixel data.
                Base::U32 m_IsCubeTexture     : 1;   ///< Defines if the texture is a cube texture.
                Base::U32 m_IsDummyTexture    : 1;   ///< Defines if the texture is a dummy returned if the creation of the real texture has fails.
                Base::U32 m_Semantic          : 8;   ///< Defines the semantic meaning of the texture if used for further optimization (maybe shader, converting, ...).
            };
        
        protected:

            SInfo            m_Info;
            void*            m_pPixels;
            Base::CharString m_FileName;
            Base::CharString m_Identifier;
            unsigned int     m_Hash;
            unsigned int     m_DirtyFlags;
            Base::U64        m_DirtyTime;

        protected:

            CTextureBase();
           ~CTextureBase();
    };
} // namespace Dt
