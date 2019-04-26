
#pragma once

#include "engine/engine_config.h"

#include "base/base_aabb2.h"
#include "base/base_typedef.h"

#include "engine/graphic/gfx_data_behavior.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_set.h"

namespace Gfx
{
    struct STextureDescriptor
    {
        static const unsigned int      s_NumberOfPixelsFromSource   = static_cast<unsigned int>(-1);
        static const unsigned int      s_NumberOfMipMapsFromSource  = static_cast<unsigned int>(-1);
        static const unsigned int      s_GenerateAllMipMaps         = static_cast<unsigned int>( 0);
        static const unsigned int      s_NumberOfTexturesFromSource = static_cast<unsigned int>(-1);
        static const CTexture::EFormat s_FormatFromSource           = CTexture::Unknown;

        unsigned int        m_NumberOfPixelsU;
        unsigned int        m_NumberOfPixelsV;
        unsigned int        m_NumberOfPixelsW;
        unsigned int        m_NumberOfMipMaps;
        unsigned int        m_NumberOfTextures;
        unsigned int        m_Binding;
        unsigned int        m_Access;
        CTexture::EFormat   m_Format;
        CTexture::EUsage    m_Usage;
        CTexture::ESemantic m_Semantic;
        const Base::Char*   m_pFileName;
        void*               m_pPixels;
    };
} // namespace Gfx

namespace Gfx
{
namespace TextureManager
{
    ENGINE_API void OnStart();
    ENGINE_API void OnExit();

    ENGINE_API CTexturePtr GetDummyTexture2D();
    ENGINE_API CTexturePtr GetDummyTexture3D();
    ENGINE_API CTexturePtr GetDummyCubeTexture();

    ENGINE_API CTexturePtr CreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable = true, SDataBehavior::Enum _Behavior = SDataBehavior::LeftAlone);
    ENGINE_API CTexturePtr CreateTexture3D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable = true, SDataBehavior::Enum _Behavior = SDataBehavior::LeftAlone);

    ENGINE_API CTexturePtr CreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable = true, SDataBehavior::Enum _Behavior = SDataBehavior::LeftAlone);

    ENGINE_API CTexturePtr CreateExternalTexture();
    
    ENGINE_API CTextureSetPtr CreateTextureSet(CTexturePtr _Texture1Ptr);
    ENGINE_API CTextureSetPtr CreateTextureSet(CTexturePtr _Texture1Ptr, CTexturePtr _Texture2Ptr);
    ENGINE_API CTextureSetPtr CreateTextureSet(CTexturePtr _Texture1Ptr, CTexturePtr _Texture2Ptr, CTexturePtr _Texture3Ptr);
    ENGINE_API CTextureSetPtr CreateTextureSet(CTexturePtr _Texture1Ptr, CTexturePtr _Texture2Ptr, CTexturePtr _Texture3Ptr, CTexturePtr _Texture4Ptr);
    ENGINE_API CTextureSetPtr CreateTextureSet(CTexturePtr* _pTexturePtrs, unsigned int _NumberOfTextures);

    ENGINE_API CTexturePtr GetTextureByHash(unsigned int _Hash);

    ENGINE_API void ClearTextureLayer(CTexturePtr _TexturePtr, const void* _pData = nullptr, int _Layer = 0);
    ENGINE_API void ClearTexture(CTexturePtr _TexturePtr, const void* _pData = nullptr);

    ENGINE_API void CopyToTexture2D(CTexturePtr _TexturePtr, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, const void* _pBytes, bool _UpdateMipLevels = false);
    ENGINE_API void CopyToTextureArray2D(CTexturePtr _TextureArrayPtr, unsigned int _IndexOfSlice, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, const void* _pBytes, bool _UpdateMipLevels = false);
    ENGINE_API void CopyToTextureArray2D(CTexturePtr _TextureArrayPtr, unsigned int _IndexOfSlice, CTexturePtr _TexturePtr, bool _UpdateMipLevels = false);

    ENGINE_API CTexturePtr GetMipmapFromTexture2D(CTexturePtr _TexturePtr, unsigned int _Mipmap);
    
    ENGINE_API void UpdateMipmap(CTexturePtr _TexturePtr);

    ENGINE_API void SaveTexture(CTexturePtr _TexturePtr, const std::string& _rPathToFile);
    ENGINE_API void CopyTextureToCPU(CTexturePtr _TexturePtr, char* _pBuffer);

    ENGINE_API void SetTextureLabel(CTexturePtr _TexturePtr, const char* _pLabel);
} // namespace TextureManager
} // namespace Gfx
