
#pragma once

#include "base/base_aabb2.h"
#include "base/base_typedef.h"
#include "base/base_vector2.h"
#include "base/base_vector4.h"

#include "graphic/gfx_data_behavior.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_texture_1d.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_3d.h"
#include "graphic/gfx_texture_set.h"

namespace Gfx
{
    struct STextureDescriptor
    {
        static const unsigned int          s_NumberOfPixelsFromSource   = static_cast<unsigned int>(-1);
        static const unsigned int          s_NumberOfMipMapsFromSource  = static_cast<unsigned int>(-1);
        static const unsigned int          s_GenerateAllMipMaps         = static_cast<unsigned int>( 0);
        static const unsigned int          s_NumberOfTexturesFromSource = static_cast<unsigned int>(-1);
        static const CTextureBase::EFormat s_FormatFromSource           = CTextureBase::Unknown;

        unsigned int            m_NumberOfPixelsU;
        unsigned int            m_NumberOfPixelsV;
        unsigned int            m_NumberOfPixelsW;
        unsigned int            m_NumberOfMipMaps;
        unsigned int            m_NumberOfTextures;
        unsigned int            m_Binding;
        unsigned int            m_Access;
        CTextureBase::EFormat   m_Format;
        CTextureBase::EUsage    m_Usage;
        CTextureBase::ESemantic m_Semantic;
        const Base::Char*       m_pFileName;
        void*                   m_pPixels;
    };
} // namespace Gfx

namespace Gfx
{
namespace TextureManager
{
    void OnStart();
    void OnExit();

    CTexture1DPtr GetDummyTexture1D();
    CTexture2DPtr GetDummyTexture2D();
    CTexture3DPtr GetDummyTexture3D();
    CTexture2DPtr GetDummyCubeTexture();

    CTexture1DPtr CreateTexture1D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable = true, SDataBehavior::Enum _Behavior = SDataBehavior::LeftAlone);
    CTexture2DPtr CreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable = true, SDataBehavior::Enum _Behavior = SDataBehavior::LeftAlone);
    CTexture3DPtr CreateTexture3D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable = true, SDataBehavior::Enum _Behavior = SDataBehavior::LeftAlone);

    CTexture2DPtr CreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable = true, SDataBehavior::Enum _Behavior = SDataBehavior::LeftAlone);
    
    CTextureSetPtr CreateTextureSet(CTextureBasePtr _Texture1Ptr);
    CTextureSetPtr CreateTextureSet(CTextureBasePtr _Texture1Ptr, CTextureBasePtr _Texture2Ptr);
    CTextureSetPtr CreateTextureSet(CTextureBasePtr _Texture1Ptr, CTextureBasePtr _Texture2Ptr, CTextureBasePtr _Texture3Ptr);
    CTextureSetPtr CreateTextureSet(CTextureBasePtr _Texture1Ptr, CTextureBasePtr _Texture2Ptr, CTextureBasePtr _Texture3Ptr, CTextureBasePtr _Texture4Ptr);
    CTextureSetPtr CreateTextureSet(CTextureBasePtr* _pTexturePtrs, unsigned int _NumberOfTextures);

    CTexture1DPtr GetTexture1DByHash(unsigned int _Hash);
    CTexture2DPtr GetTexture2DByHash(unsigned int _Hash);
    CTexture3DPtr GetTexture3DByHash(unsigned int _Hash);

    void ClearTexture1D(CTexture1DPtr _TexturePtr, const Base::Float4& _rColor);
    void ClearTexture2D(CTexture2DPtr _TexturePtr, const Base::Float4& _rColor);
    void ClearTexture3D(CTexture3DPtr _TexturePtr, const Base::Float4& _rColor);

    void CopyToTexture2D(CTexture2DPtr _TexturePtr, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, void* _pBytes, bool _UpdateMipLevels = false);
    void CopyToTextureArray2D(CTexture2DPtr _TextureArrayPtr, unsigned int _IndexOfSlice, const Base::AABB2UInt& _rTargetRect, unsigned int _NumberOfBytesPerLine, void* _pBytes, bool _UpdateMipLevels = false);
    void CopyToTextureArray2D(CTexture2DPtr _TextureArrayPtr, unsigned int _IndexOfSlice, CTexture2DPtr _TexturePtr, bool _UpdateMipLevels = false);

    CTexture2DPtr GetMipmapFromTexture2D(CTexture2DPtr _TexturePtr, unsigned int _Mipmap);
    
    void UpdateMipmap(CTexture2DPtr _TexturePtr);

	void SetTexture2DLabel(CTexture2DPtr _TexturePtr, const char* _pLabel);
	void SetTexture3DLabel(CTexture3DPtr _TexturePtr, const char* _pLabel);
} // namespace TextureManager
} // namespace Gfx
