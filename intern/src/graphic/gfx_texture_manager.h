
#pragma once

#include "base/base_aabb2.h"
#include "base/base_typedef.h"
#include "base/base_vector2.h"
#include "base/base_vector4.h"

#include "graphic/gfx_data_behavior.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_texture.h"
#include "graphic/gfx_texture_set.h"

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
    void OnStart();
    void OnExit();

    CTexturePtr GetDummyTexture2D();
    CTexturePtr GetDummyTexture3D();
    CTexturePtr GetDummyCubeTexture();

    CTexturePtr CreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable = true, SDataBehavior::Enum _Behavior = SDataBehavior::LeftAlone);
    CTexturePtr CreateTexture3D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable = true, SDataBehavior::Enum _Behavior = SDataBehavior::LeftAlone);

    CTexturePtr CreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable = true, SDataBehavior::Enum _Behavior = SDataBehavior::LeftAlone);
    
    CTextureSetPtr CreateTextureSet(CTexturePtr _Texture1Ptr);
    CTextureSetPtr CreateTextureSet(CTexturePtr _Texture1Ptr, CTexturePtr _Texture2Ptr);
    CTextureSetPtr CreateTextureSet(CTexturePtr _Texture1Ptr, CTexturePtr _Texture2Ptr, CTexturePtr _Texture3Ptr);
    CTextureSetPtr CreateTextureSet(CTexturePtr _Texture1Ptr, CTexturePtr _Texture2Ptr, CTexturePtr _Texture3Ptr, CTexturePtr _Texture4Ptr);
    CTextureSetPtr CreateTextureSet(CTexturePtr* _pTexturePtrs, unsigned int _NumberOfTextures);

    CTexturePtr GetTextureByHash(unsigned int _Hash);

    void ClearTexture2D(CTexturePtr _TexturePtr, const Base::Float4& _rColor);
    void ClearTexture3D(CTexturePtr _TexturePtr, const Base::Float4& _rColor);

    void CopyToTexture2D(CTexturePtr _TexturePtr, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, void* _pBytes, bool _UpdateMipLevels = false);
    void CopyToTextureArray2D(CTexturePtr _TextureArrayPtr, unsigned int _IndexOfSlice, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, void* _pBytes, bool _UpdateMipLevels = false);
    void CopyToTextureArray2D(CTexturePtr _TextureArrayPtr, unsigned int _IndexOfSlice, CTexturePtr _TexturePtr, bool _UpdateMipLevels = false);

    CTexturePtr GetMipmapFromTexture2D(CTexturePtr _TexturePtr, unsigned int _Mipmap);
    
    void UpdateMipmap(CTexturePtr _TexturePtr);

	void SetTextureLabel(CTexturePtr _TexturePtr, const char* _pLabel);
} // namespace TextureManager
} // namespace Gfx
