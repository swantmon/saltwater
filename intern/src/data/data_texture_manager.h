
#pragma once

#include "base/base_aabb2.h"
#include "base/base_typedef.h"
#include "base/base_vector2.h"
#include "base/base_vector4.h"

#include "data/data_data_behavior.h"
#include "data/data_texture_1d.h"
#include "data/data_texture_2d.h"
#include "data/data_texture_cube.h"

namespace Dt
{
    struct STextureDescriptor
    {
        static const unsigned int          s_NumberOfPixelsFromSource   = static_cast<unsigned int>(-1);
        static const CTextureBase::EFormat s_FormatFromSource           = CTextureBase::Unknown;

        unsigned int            m_NumberOfPixelsU;
        unsigned int            m_NumberOfPixelsV;
        unsigned int            m_NumberOfPixelsW;
        unsigned int            m_NumberOfTextures;
        CTextureBase::EFormat   m_Format;
        CTextureBase::ESemantic m_Semantic;
        const Base::Char*       m_pFileName;
        void*                   m_pPixels;
        const Base::Char*       m_pIdentifier;
    };
} // namespace Dt

namespace Dt
{
namespace TextureManager
{
    void OnStart();
    void OnExit();

    CTexture1D* CreateTexture1D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable = true, SDataBehavior::Enum _Behavior = SDataBehavior::Listen);
    CTexture2D* CreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable = true, SDataBehavior::Enum _Behavior = SDataBehavior::Listen);

    CTextureCube* CreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable = true, SDataBehavior::Enum _Behavior = SDataBehavior::Listen);

    void CopyToTexture2D(CTexture2D* _pTexture2D, void* _pPixels);
    void CopyToTexture2D(CTexture2D* _pTexture2D, CTexture2D* _pTexture);

    void CopyToTextureCube(CTextureCube* _pTextureCube, CTextureCube::EFace _Face, CTexture2D* _pTexture);

    void SaveTexture2DToFile(CTexture2D* _pTexture2D, const Base::Char* _pPathToFile);
} // namespace TextureManager
} // namespace Dt
