
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_texture_base.h"

namespace Gfx
{
    CTextureBase::CTextureBase()
        : m_Info    ()
        , m_pPixels (0)
        , m_FileName("")
        , m_Hash    (static_cast<unsigned int>(-1))
    {
    }

    // -----------------------------------------------------------------------------

    CTextureBase::~CTextureBase()
    {
        if (m_Info.m_IsPixelOwner)
        {
            Base::CMemory::Free(m_pPixels);
        }

        m_FileName.clear();
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

    CTextureBase::EUsage CTextureBase::GetUsage() const
    {
        return static_cast<EUsage>(m_Info.m_Usage);
    }

    // -----------------------------------------------------------------------------

    CTextureBase::EAccess CTextureBase::GetAccess() const
    {
        return static_cast<EAccess>(m_Info.m_Access);
    }

    // -----------------------------------------------------------------------------

    unsigned int CTextureBase::GetBinding() const
    {
        return m_Info.m_Binding;
    }

    // -----------------------------------------------------------------------------

    unsigned int CTextureBase::GetNumberOfMipLevels() const
    {
        return m_Info.m_NumberOfMipLevels;
    }

    // -----------------------------------------------------------------------------

    unsigned int CTextureBase::GetNumberOfTextures() const
    {
        return m_Info.m_NumberOfTextures;
    }

    // -----------------------------------------------------------------------------

    unsigned int CTextureBase::GetCurrentMipLevel() const
    {
        return m_Info.m_CurrentMipLevel;
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

    const std::string& CTextureBase::GetFileName() const
    {
        return m_FileName;
    }

    // -----------------------------------------------------------------------------

    unsigned int CTextureBase::GetHash() const
    {
        return m_Hash;
    }
} // namespace Gfx
