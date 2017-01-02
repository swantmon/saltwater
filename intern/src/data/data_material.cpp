
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
        , m_DirtyFlags       (0)
        , m_DirtyTime        (0)
    {

    }

    // -----------------------------------------------------------------------------

    CMaterial::~CMaterial()
    {
        m_Materialname.Clear();
    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetMaterialname(const char* _pMaterialname)
    {
        m_Materialname = _pMaterialname;
    }

    // -----------------------------------------------------------------------------

    const char* CMaterial::GetMaterialname() const
    {
        return m_Materialname.GetConst();
    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetFilename(const char* _pFilename)
    {
        m_FileName = _pFilename;
    }

    // -----------------------------------------------------------------------------

    const char* CMaterial::GetFileName() const
    {
        return m_FileName.GetConst();
    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetColorTexture(Dt::CTexture2D* _pColorTexture)
    {
        m_pColorTexture = _pColorTexture;
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

    void CMaterial::SetNormalTexture(Dt::CTexture2D* _pNormalTexture)
    {
        m_pNormalTexture = _pNormalTexture;
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

    void CMaterial::SetRoughnessTexture(Dt::CTexture2D* _pRoughnessTexture)
    {
        m_pRoughnessTexture = _pRoughnessTexture;
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

    void CMaterial::SetMetalTexture(Dt::CTexture2D* _pMetalTexture)
    {
        m_pMetalTexture = _pMetalTexture;
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

    void CMaterial::SetAmbientOcclusionTexture(Dt::CTexture2D* _pAmbientOcclusionTexture)
    {
        m_pAOTexture = _pAmbientOcclusionTexture;
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

    void CMaterial::SetBumpTexture(Dt::CTexture2D* _pBumpTexture)
    {
        m_pBumpTexture = _pBumpTexture;
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

    void CMaterial::SetColor(const Base::Float3& _rColor)
    {
        m_Color = _rColor;
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& CMaterial::GetColor() const
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetTilingOffset(const Base::Float4& _rTilingOffset)
    {
        m_TilingOffset = _rTilingOffset;
    }

    // -----------------------------------------------------------------------------

    const Base::Float4& CMaterial::GetTilingOffset() const
    {
        return m_TilingOffset;
    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetRoughness(float _Roughness)
    {
        m_Roughness = _Roughness;
    }

    // -----------------------------------------------------------------------------

    float CMaterial::GetRoughness() const
    {
        return m_Roughness;
    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetSmoothness(float _Smoothness)
    {
        m_Roughness = 1.0f - _Smoothness;
    }

    // -----------------------------------------------------------------------------

    float CMaterial::GetSmoothness() const
    {
        return 1.0f - m_Roughness;
    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetReflectance(float _Reflectance)
    {
        m_Reflectance = _Reflectance;
    }

    // -----------------------------------------------------------------------------

    float CMaterial::GetReflectance() const
    {
        return m_Reflectance;
    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetMetalness(float _Metalness)
    {
        m_MetalMask = _Metalness;
    }

    // -----------------------------------------------------------------------------

    float CMaterial::GetMetalness() const
    {
        return m_MetalMask;
    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetDisplacement(float _Displacement)
    {
        m_Displacement = _Displacement;
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

    // -----------------------------------------------------------------------------

    unsigned int CMaterial::GetDirtyFlags() const
    {
        return m_DirtyFlags;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CMaterial::GetDirtyTime() const
    {
        return m_DirtyTime;
    }
} // namespace Dt