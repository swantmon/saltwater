
#pragma once

#include "base/base_aabb2.h"
#include "base/base_typedef.h"
#include "base/base_vector2.h"
#include "base/base_vector4.h"

#include "data/data_data_behavior.h"
#include "data/data_texture_2d.h"
#include "data/data_texture_cube.h"

#include <functional>

namespace Dt
{
namespace TextureManager
{
    typedef std::function<void(Dt::CTextureBase* _pTexture)> CTextureDelegate;
} // namespace TextureManager
} // namespace Dt

#define DATA_DIRTY_TEXTURE_METHOD(_Method) std::bind(_Method, this, std::placeholders::_1)

namespace Dt
{
    struct STextureDescriptor
    {
        static const unsigned int          s_NumberOfPixelsFromSource = static_cast<unsigned int>(-1);
        static const CTextureBase::EFormat s_FormatFromSource         = CTextureBase::UndefinedFormat;

        unsigned int            m_NumberOfPixelsU;
        unsigned int            m_NumberOfPixelsV;
        unsigned int            m_NumberOfPixelsW;
        unsigned int            m_NumberOfTextures;
        unsigned int            m_Binding;
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
    // TODO by tschwandt
    // return references instead of pointer

    void OnStart();
    void OnExit();

    CTextureBase* CreateTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable = true, SDataBehavior::Enum _Behavior = SDataBehavior::Listen);
    CTexture2D* CreateTexture2D(const STextureDescriptor& _rDescriptor, bool _IsDeleteable = true, SDataBehavior::Enum _Behavior = SDataBehavior::Listen);

    CTextureCube* CreateCubeTexture(const STextureDescriptor& _rDescriptor, bool _IsDeleteable = true, SDataBehavior::Enum _Behavior = SDataBehavior::Listen);

    CTextureBase* GetTextureByHash(unsigned int _Hash);
    CTexture2D* GetTexture2DByHash(unsigned int _Hash);
    CTextureCube* GetTextureCubeByHash(unsigned int _Hash);

    void CopyToTexture2D(CTexture2D* _pTexture2D, const Base::Char* _pFile);
    void CopyToTexture2D(CTexture2D* _pTexture2D, void* _pPixels);
    void CopyToTexture2D(CTexture2D* _pTexture2D, CTexture2D* _pTexture);

    void CopyToTextureCube(CTextureCube* _pTextureCube, CTextureCube::EFace _Face, void* _pPixels);
    void CopyToTextureCube(CTextureCube* _pTextureCube, CTextureCube::EFace _Face, CTexture2D* _pTexture);

    void SaveTexture2DToFile(CTexture2D* _pTexture2D, const Base::Char* _pPathToFile);

    void MarkTextureAsDirty(CTextureBase* _pTextureBase, unsigned int _DirtyFlags);

    void RegisterDirtyTextureHandler(CTextureDelegate _NewDelegate);
} // namespace TextureManager
} // namespace Dt
