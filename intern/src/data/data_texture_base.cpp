
#include "data/data_precompiled.h"

#include "data/data_texture_base.h"

namespace Dt
{
    CTextureBase::CTextureBase()
        : m_Info      ()
        , m_pPixels   (0)
        , m_FileName  ()
        , m_Identifier()
        , m_Hash      (0)
        , m_DirtyFlags(0)
        , m_DirtyTime (0)
    {
    }

    // -----------------------------------------------------------------------------

    CTextureBase::~CTextureBase()
    {
    }

    // -----------------------------------------------------------------------------

    CTextureBase::ESemantic CTextureBase::GetSemantic() const
    {
        return static_cast<ESemantic>(m_Info.m_Semantic);
    }

    // -----------------------------------------------------------------------------

    unsigned int CTextureBase::GetBinding() const
    {
        return m_Info.m_Binding;
    }

    // -----------------------------------------------------------------------------

    CTextureBase::EDimension CTextureBase::GetDimension() const
    {
        return static_cast<EDimension>(m_Info.m_Dimension);
    }

    // -----------------------------------------------------------------------------

    CTextureBase::EFormat CTextureBase::GetFormat() const
    {
        return static_cast<EFormat>(m_Info.m_Format);
    }

    // -----------------------------------------------------------------------------

    bool CTextureBase::IsArray() const
    {
        return (m_Info.m_NumberOfTextures > 1) && (!m_Info.m_IsCubeTexture);
    }

    // -----------------------------------------------------------------------------

    bool CTextureBase::IsCube() const
    {
        return (m_Info.m_NumberOfTextures == 6) && (m_Info.m_IsCubeTexture);
    }

    // -----------------------------------------------------------------------------

    bool CTextureBase::IsDummy() const
    {
        return m_Info.m_IsDummyTexture != 0;
    }

    // -----------------------------------------------------------------------------

    void* CTextureBase::GetPixels()
    {
        return m_pPixels;
    }

    // -----------------------------------------------------------------------------

    const void* CTextureBase::GetPixels() const
    {
        return m_pPixels;
    }

    // -----------------------------------------------------------------------------

    const Base::Char* CTextureBase::GetFileName() const
    {
        return m_FileName.c_str();
    }

    // -----------------------------------------------------------------------------

    bool CTextureBase::HasFileName() const
    {
        return m_FileName.length() > 0;
    }

    // -----------------------------------------------------------------------------

    const Base::Char* CTextureBase::GetIdentifier() const
    {
        return m_Identifier.c_str();
    }

    // -----------------------------------------------------------------------------

    bool CTextureBase::HasIdentifier() const
    {
        return m_Identifier.length() > 0;
    }

    // -----------------------------------------------------------------------------

    unsigned int CTextureBase::GetHash() const
    {
        return m_Hash;
    }

    // -----------------------------------------------------------------------------

    unsigned int CTextureBase::GetDirtyFlags() const
    {
        return m_DirtyFlags;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CTextureBase::GetDirtyTime() const
    {
        return m_DirtyTime;
    }
} // namespace Dt
