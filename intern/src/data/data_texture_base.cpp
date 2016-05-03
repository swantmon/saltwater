
#include "data/data_precompiled.h"

#include "data/data_texture_base.h"

namespace Dt
{
    CTextureBase::CTextureBase()
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
        return m_FileName.GetConst();
    }

    // -----------------------------------------------------------------------------

    const Base::Char* CTextureBase::GetIdentifier() const
    {
        return m_Identifier.GetConst();
    }

    // -----------------------------------------------------------------------------

    unsigned int CTextureBase::GetHash() const
    {
        return m_Hash;
    }
} // namespace Dt
