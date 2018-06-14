
#include "engine/engine_precompiled.h"

#include "engine/data/data_material.h"

namespace Dt
{
    CMaterial::CMaterial()
        : m_Materialname("")
        , m_FileName("")
        , m_ColorTexture("")
        , m_NormalTexture("")
        , m_RoughnessTexture("")
        , m_MetalTexture("")
        , m_AmbientOcclusionTexture("")
        , m_BumpTexture("")
        , m_AlphaTexture("")
        , m_Color(1.0f, 1.0f, 1.0f, 1.0f)
        , m_TilingOffset(1.0f, 1.0f, 0.0f, 0.0f)
        , m_Roughness(1.0f)
        , m_Reflectance(0.0f)
        , m_MetalMask(0.0f)
        , m_Displacement(0.0f)
        , m_Hash(0)
    {
    }

    // -----------------------------------------------------------------------------

    CMaterial::~CMaterial()
    {

    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetMaterialname(const std::string& _rValue)
    {
        m_Materialname = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterial::GetMaterialname() const
    {
        return m_Materialname;
    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetFileName(const std::string& _rValue)
    {
        m_FileName = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterial::GetFileName() const
    {
        return m_FileName;
    }

    // -----------------------------------------------------------------------------


    void CMaterial::SetColorTexture(const std::string& _rValue)
    {
        m_ColorTexture = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterial::GetColorTexture() const
    {
        return m_ColorTexture;
    }

    // -----------------------------------------------------------------------------


    void CMaterial::SetNormalTexture(const std::string& _rValue)
    {
        m_NormalTexture = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterial::GetNormalTexture() const
    {
        return m_NormalTexture;
    }

    // -----------------------------------------------------------------------------


    void CMaterial::SetRoughnessTexture(const std::string& _rValue)
    {
        m_RoughnessTexture = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterial::GetRoughnessTexture() const
    {
        return m_RoughnessTexture;
    }

    // -----------------------------------------------------------------------------


    void CMaterial::SetMetalTexture(const std::string& _rValue)
    {
        m_MetalTexture = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterial::GetMetalTexture() const
    {
        return m_MetalTexture;
    }

    // -----------------------------------------------------------------------------


    void CMaterial::SetAmbientOcclusionTexture(const std::string& _rValue)
    {
        m_AmbientOcclusionTexture = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterial::GetAmbientOcclusionTexture() const
    {
        return m_AmbientOcclusionTexture;
    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetBumpTexture(const std::string& _rValue)
    {
        m_BumpTexture = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterial::GetBumpTexture() const
    {
        return m_BumpTexture;
    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetAlphaTexture(const std::string& _rValue)
    {
        m_AlphaTexture = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterial::GetAlphaTexture() const
    {
        return m_AlphaTexture;
    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetColor(const glm::vec4& _rValue)
    {
        m_Color = _rValue;
    }

    // -----------------------------------------------------------------------------

    const glm::vec4& CMaterial::GetColor() const
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetTiling(const glm::vec2& _rValue)
    {
        m_TilingOffset[0] = _rValue[0];
        m_TilingOffset[1] = _rValue[1];
    }

    // -----------------------------------------------------------------------------

    const glm::vec2 CMaterial::GetTiling() const
    {
        return glm::vec2(m_TilingOffset[0], m_TilingOffset[1]);
    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetOffset(const glm::vec2& _rValue)
    {
        m_TilingOffset[2] = _rValue[0];
        m_TilingOffset[3] = _rValue[1];
    }

    // -----------------------------------------------------------------------------

    const glm::vec2 CMaterial::GetOffset() const
    {
        return glm::vec2(m_TilingOffset[2], m_TilingOffset[3]);
    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetTilingOffset(const glm::vec4& _rValue)
    {
        m_TilingOffset = _rValue;
    }

    // -----------------------------------------------------------------------------

    const glm::vec4& CMaterial::GetTilingOffset() const
    {
        return m_TilingOffset;
    }

    // -----------------------------------------------------------------------------


    void CMaterial::SetRoughness(float _Value)
    {
        m_Roughness = _Value;
    }

    // -----------------------------------------------------------------------------

    float CMaterial::GetRoughness() const
    {
        return m_Roughness;
    }

    // -----------------------------------------------------------------------------


    void CMaterial::SetSmoothness(float _Value)
    {
        m_Roughness = 1.0f - _Value;
    }

    // -----------------------------------------------------------------------------

    float CMaterial::GetSmoothness() const
    {
        return  1.0f - m_Roughness;
    }

    // -----------------------------------------------------------------------------


    void CMaterial::SetReflectance(float _Value)
    {
        m_Reflectance = _Value;
    }

    // -----------------------------------------------------------------------------

    float CMaterial::GetReflectance() const
    {
        return m_Reflectance;
    }

    // -----------------------------------------------------------------------------


    void CMaterial::SetMetalness(float _Value)
    {
        m_MetalMask = _Value;
    }

    // -----------------------------------------------------------------------------

    float CMaterial::GetMetalness() const
    {
        return m_MetalMask;
    }

    // -----------------------------------------------------------------------------


    void CMaterial::SetDisplacement(float _Value)
    {
        m_Displacement = _Value;
    }

    // -----------------------------------------------------------------------------

    float CMaterial::GetDisplacement() const
    {
        return m_Displacement;
    }

    // -----------------------------------------------------------------------------

    void CMaterial::SetAlpha(float _Value)
    {
        m_Color[3] = _Value;
    }

    // -----------------------------------------------------------------------------

    float CMaterial::GetAlpha() const
    {
        return m_Color[3];
    }

    // -----------------------------------------------------------------------------

    CMaterial::BHash CMaterial::GetHash() const
    {
        return m_Hash;
    }
} // namespace Dt