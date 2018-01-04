
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_texture.h"

namespace Gfx
{
    CTexture::CTexture()
        : m_Info    ()
        , m_pPixels (0)
        , m_FileName("")
        , m_Hash    (static_cast<unsigned int>(-1))
    {
        Base::CMemory::Zero(m_NumberOfPixels, sizeof(m_NumberOfPixels));
    }

    // -----------------------------------------------------------------------------

    CTexture::~CTexture()
    {
        if (m_Info.m_IsPixelOwner)
        {
            Base::CMemory::Free(m_pPixels);
        }

        m_FileName.clear();
    }

    // -----------------------------------------------------------------------------

    CTexture::ESemantic CTexture::GetSemantic() const
    {
        return static_cast<ESemantic>(m_Info.m_Semantic);
    }

    // -----------------------------------------------------------------------------

    CTexture::EDimension CTexture::GetDimension() const
    {
        return static_cast<EDimension>(m_Info.m_Dimension);
    }

    // -----------------------------------------------------------------------------

    CTexture::EFormat CTexture::GetFormat() const
    {
        return static_cast<EFormat>(m_Info.m_Format);
    }

    // -----------------------------------------------------------------------------

    CTexture::EUsage CTexture::GetUsage() const
    {
        return static_cast<EUsage>(m_Info.m_Usage);
    }

    // -----------------------------------------------------------------------------

    CTexture::EAccess CTexture::GetAccess() const
    {
        return static_cast<EAccess>(m_Info.m_Access);
    }

    // -----------------------------------------------------------------------------

    unsigned int CTexture::GetBinding() const
    {
        return m_Info.m_Binding;
    }

    // -----------------------------------------------------------------------------

    unsigned int CTexture::GetNumberOfMipLevels() const
    {
        return m_Info.m_NumberOfMipLevels;
    }

    // -----------------------------------------------------------------------------

    unsigned int CTexture::GetNumberOfTextures() const
    {
        return m_Info.m_NumberOfTextures;
    }

    // -----------------------------------------------------------------------------

    unsigned int CTexture::GetCurrentMipLevel() const
    {
        return m_Info.m_CurrentMipLevel;
    }

    // -----------------------------------------------------------------------------

    CTexture::BPixels CTexture::GetNumberOfPixelsU() const
    {
        return m_NumberOfPixels[0];
    }

    // -----------------------------------------------------------------------------

    CTexture::BPixels CTexture::GetNumberOfPixelsV() const
    {
        return m_NumberOfPixels[1];
    }

    // -----------------------------------------------------------------------------

    CTexture::BPixels CTexture::GetNumberOfPixelsW() const
    {
        return m_NumberOfPixels[2];
    }

    // -----------------------------------------------------------------------------

    bool CTexture::IsArray() const
    {
        return (m_Info.m_NumberOfTextures > 1) && (!m_Info.m_IsCubeTexture);
    }

    // -----------------------------------------------------------------------------

    bool CTexture::IsCube() const
    {
        return (m_Info.m_NumberOfTextures == 6) && (m_Info.m_IsCubeTexture);
    }

    // -----------------------------------------------------------------------------

    bool CTexture::IsDummy() const
    {
        return m_Info.m_IsDummyTexture != 0;
    }

    // -----------------------------------------------------------------------------

    void* CTexture::GetPixels()
    {
        return m_pPixels;
    }

    // -----------------------------------------------------------------------------

    const void* CTexture::GetPixels() const
    {
        return m_pPixels;
    }

    // -----------------------------------------------------------------------------

    const std::string& CTexture::GetFileName() const
    {
        return m_FileName;
    }

    // -----------------------------------------------------------------------------

    unsigned int CTexture::GetHash() const
    {
        return m_Hash;
    }
} // namespace Gfx
