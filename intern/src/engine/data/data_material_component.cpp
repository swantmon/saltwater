
#include "engine/engine_precompiled.h"

#include "engine/data/data_material_component.h"

namespace Dt
{
    CMaterialComponent::CMaterialComponent()
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
    {
    }

    // -----------------------------------------------------------------------------

    CMaterialComponent::~CMaterialComponent()
    {

    }

    // -----------------------------------------------------------------------------

    void CMaterialComponent::SetMaterialname(const std::string& _rValue)
    {
        m_Materialname = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterialComponent::GetMaterialname() const
    {
        return m_Materialname;
    }

    // -----------------------------------------------------------------------------

    void CMaterialComponent::SetFileName(const std::string& _rValue)
    {
        m_FileName = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterialComponent::GetFileName() const
    {
        return m_FileName;
    }

    // -----------------------------------------------------------------------------


    void CMaterialComponent::SetColorTexture(const std::string& _rValue)
    {
        m_ColorTexture = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterialComponent::GetColorTexture() const
    {
        return m_ColorTexture;
    }

    // -----------------------------------------------------------------------------


    void CMaterialComponent::SetNormalTexture(const std::string& _rValue)
    {
        m_NormalTexture = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterialComponent::GetNormalTexture() const
    {
        return m_NormalTexture;
    }

    // -----------------------------------------------------------------------------


    void CMaterialComponent::SetRoughnessTexture(const std::string& _rValue)
    {
        m_RoughnessTexture = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterialComponent::GetRoughnessTexture() const
    {
        return m_RoughnessTexture;
    }

    // -----------------------------------------------------------------------------


    void CMaterialComponent::SetMetalTexture(const std::string& _rValue)
    {
        m_MetalTexture = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterialComponent::GetMetalTexture() const
    {
        return m_MetalTexture;
    }

    // -----------------------------------------------------------------------------


    void CMaterialComponent::SetAmbientOcclusionTexture(const std::string& _rValue)
    {
        m_AmbientOcclusionTexture = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterialComponent::GetAmbientOcclusionTexture() const
    {
        return m_AmbientOcclusionTexture;
    }

    // -----------------------------------------------------------------------------

    void CMaterialComponent::SetBumpTexture(const std::string& _rValue)
    {
        m_BumpTexture = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterialComponent::GetBumpTexture() const
    {
        return m_BumpTexture;
    }

    // -----------------------------------------------------------------------------

    void CMaterialComponent::SetAlphaTexture(const std::string& _rValue)
    {
        m_AlphaTexture = _rValue;
    }

    // -----------------------------------------------------------------------------

    const std::string& CMaterialComponent::GetAlphaTexture() const
    {
        return m_AlphaTexture;
    }

    // -----------------------------------------------------------------------------

    void CMaterialComponent::SetColor(const glm::vec4& _rValue)
    {
        m_Color = _rValue;
    }

    // -----------------------------------------------------------------------------

    const glm::vec4& CMaterialComponent::GetColor() const
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    void CMaterialComponent::SetTiling(const glm::vec2& _rValue)
    {
        m_TilingOffset[0] = _rValue[0];
        m_TilingOffset[1] = _rValue[1];
    }

    // -----------------------------------------------------------------------------

    const glm::vec2 CMaterialComponent::GetTiling() const
    {
        return glm::vec2(m_TilingOffset[0], m_TilingOffset[1]);
    }

    // -----------------------------------------------------------------------------

    void CMaterialComponent::SetOffset(const glm::vec2& _rValue)
    {
        m_TilingOffset[2] = _rValue[0];
        m_TilingOffset[3] = _rValue[1];
    }

    // -----------------------------------------------------------------------------

    const glm::vec2 CMaterialComponent::GetOffset() const
    {
        return glm::vec2(m_TilingOffset[2], m_TilingOffset[3]);
    }

    // -----------------------------------------------------------------------------

    void CMaterialComponent::SetTilingOffset(const glm::vec4& _rValue)
    {
        m_TilingOffset = _rValue;
    }

    // -----------------------------------------------------------------------------

    const glm::vec4& CMaterialComponent::GetTilingOffset() const
    {
        return m_TilingOffset;
    }

    // -----------------------------------------------------------------------------


    void CMaterialComponent::SetRoughness(float _Value)
    {
        m_Roughness = _Value;
    }

    // -----------------------------------------------------------------------------

    float CMaterialComponent::GetRoughness() const
    {
        return m_Roughness;
    }

    // -----------------------------------------------------------------------------


    void CMaterialComponent::SetSmoothness(float _Value)
    {
        m_Roughness = 1.0f - _Value;
    }

    // -----------------------------------------------------------------------------

    float CMaterialComponent::GetSmoothness() const
    {
        return  1.0f - m_Roughness;
    }

    // -----------------------------------------------------------------------------


    void CMaterialComponent::SetReflectance(float _Value)
    {
        m_Reflectance = _Value;
    }

    // -----------------------------------------------------------------------------

    float CMaterialComponent::GetReflectance() const
    {
        return m_Reflectance;
    }

    // -----------------------------------------------------------------------------


    void CMaterialComponent::SetMetalness(float _Value)
    {
        m_MetalMask = _Value;
    }

    // -----------------------------------------------------------------------------

    float CMaterialComponent::GetMetalness() const
    {
        return m_MetalMask;
    }

    // -----------------------------------------------------------------------------


    void CMaterialComponent::SetDisplacement(float _Value)
    {
        m_Displacement = _Value;
    }

    // -----------------------------------------------------------------------------

    float CMaterialComponent::GetDisplacement() const
    {
        return m_Displacement;
    }

    // -----------------------------------------------------------------------------

    void CMaterialComponent::SetAlpha(float _Value)
    {
        m_Color[3] = _Value;
    }

    // -----------------------------------------------------------------------------

    float CMaterialComponent::GetAlpha() const
    {
        return m_Color[3];
    }
} // namespace Dt