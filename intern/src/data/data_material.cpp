
#include "data/data_precompiled.h"

#include "data/data_material.h"

namespace Dt
{
    CMaterial::CMaterial()
        : m_Materialname     ()
        , m_FileName         ()
        , m_pColorTexture    (0)
        , m_pNormalTexture   (0)
        , m_pRoughnessTexture(0)
        , m_pMetalTexture    (0)
        , m_pAOTexture       (0)
        , m_pBumpTexture     (0)
        , m_Color            (1.0f, 1.0f, 1.0f)
        , m_TilingOffset     (0.0f, 0.0f, 1.0f, 1.0f)
        , m_Roughness        (1.0f)
        , m_Reflectance      (0.0f)
        , m_MetalMask        (0.0f)
        , m_Displacement     (0.0f)
        , m_Hash             (0)
    {
    }

    // -----------------------------------------------------------------------------

    CMaterial::~CMaterial()
    {
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterial::GetMaterialname() const
    {
        return m_Materialname;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterial::GetFileName() const
    {
        return m_FileName;
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CMaterial::GetColorTexture()
    {
        return m_pColorTexture;
    }

    // -----------------------------------------------------------------------------

    const Dt::CTexture2D* CMaterial::GetColorTexture() const
    {
        return m_pColorTexture;
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CMaterial::GetNormalTexture()
    {
        return m_pNormalTexture;
    }

    // -----------------------------------------------------------------------------

    const Dt::CTexture2D* CMaterial::GetNormalTexture() const
    {
        return m_pNormalTexture;
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CMaterial::GetRoughnessTexture()
    {
        return m_pRoughnessTexture;
    }

    // -----------------------------------------------------------------------------

    const Dt::CTexture2D* CMaterial::GetRoughnessTexture() const
    {
        return m_pRoughnessTexture;
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CMaterial::GetMetalTexture()
    {
        return m_pMetalTexture;
    }

    // -----------------------------------------------------------------------------

    const Dt::CTexture2D* CMaterial::GetMetalTexture() const
    {
        return m_pMetalTexture;
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CMaterial::GetAmbientOcclusionTexture()
    {
        return m_pAOTexture;
    }

    // -----------------------------------------------------------------------------

    const Dt::CTexture2D* CMaterial::GetAmbientOcclusionTexture() const
    {
        return m_pAOTexture;
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CMaterial::GetBumpTexture()
    {
        return m_pBumpTexture;
    }

    // -----------------------------------------------------------------------------

    const Dt::CTexture2D* CMaterial::GetBumpTexture() const
    {
        return m_pBumpTexture;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CMaterial::GetColor() const
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    const glm::vec4& CMaterial::GetTilingOffset() const
    {
        return m_TilingOffset;
    }

    // -----------------------------------------------------------------------------

    float CMaterial::GetRoughness() const
    {
        return m_Roughness;
    }

    // -----------------------------------------------------------------------------

    float CMaterial::GetSmoothness() const
    {
        return 1.0f - m_Roughness;
    }

    // -----------------------------------------------------------------------------

    float CMaterial::GetReflectance() const
    {
        return m_Reflectance;
    }

    // -----------------------------------------------------------------------------

    float CMaterial::GetMetalness() const
    {
        return m_MetalMask;
    }

    // -----------------------------------------------------------------------------

    float CMaterial::GetDisplacement() const
    {
        return m_Displacement;
    }

    // -----------------------------------------------------------------------------

    unsigned int CMaterial::GetHash() const
    {
        return m_Hash;
    }
} // namespace Dt