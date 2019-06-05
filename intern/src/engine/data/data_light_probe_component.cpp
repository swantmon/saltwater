
#include "engine/engine_precompiled.h"

#include "engine/data/data_light_probe_component.h"

namespace Dt
{
    REGISTER_COMPONENT_SER(CLightProbeComponent);

    CLightProbeComponent::CLightProbeComponent()
        : m_RefreshMode       (Static)
        , m_Type              (Sky)
        , m_Quality           (PX128)
        , m_ClearFlag         (Skybox)
        , m_Cubemap           ("")
        , m_Intensity         (0)
        , m_Near              (0.1f)
        , m_Far               (10.0f)
        , m_ParallaxCorrection(false)
        , m_BoxSize           (10.0f)
    {

    }

    // -----------------------------------------------------------------------------

    CLightProbeComponent::~CLightProbeComponent()
    {

    }

    // -----------------------------------------------------------------------------

    void CLightProbeComponent::SetRefreshMode(ERefreshMode _RefreshMode)
    {
        m_RefreshMode = _RefreshMode;
    }

    // -----------------------------------------------------------------------------

    CLightProbeComponent::ERefreshMode CLightProbeComponent::GetRefreshMode()
    {
        return m_RefreshMode;
    }

    // -----------------------------------------------------------------------------

    void CLightProbeComponent::SetType(EType _Type)
    {
        m_Type = _Type;
    }

    // -----------------------------------------------------------------------------

    CLightProbeComponent::EType CLightProbeComponent::GetType() const
    {
        return m_Type;
    }

    // -----------------------------------------------------------------------------

    void CLightProbeComponent::SetQuality(EQuality _Quality)
    {
        m_Quality = _Quality;
    }

    // -----------------------------------------------------------------------------

    CLightProbeComponent::EQuality CLightProbeComponent::GetQuality() const
    {
        return m_Quality;
    }

    // -----------------------------------------------------------------------------

    unsigned int CLightProbeComponent::GetQualityInPixel() const
    {
        static unsigned int s_QualityInPixel[s_NumberOfQualities] = { 32, 64, 128, 256, 512, 1024, 2048 };

        return s_QualityInPixel[m_Quality];
    }

    // -----------------------------------------------------------------------------

    void CLightProbeComponent::SetClearFlag(EClearFlag _ClearFlag)
    {
        m_ClearFlag = _ClearFlag;
    }

    // -----------------------------------------------------------------------------

    CLightProbeComponent::EClearFlag CLightProbeComponent::GetClearFlag() const
    {
        return m_ClearFlag;
    }

    // -----------------------------------------------------------------------------

    void CLightProbeComponent::SetCubemap(const std::string& _pCubemap)
    {
        m_Cubemap = _pCubemap;
    }

    // -----------------------------------------------------------------------------

    const std::string& CLightProbeComponent::GetCubemap()
    {
        return m_Cubemap;
    }

    // -----------------------------------------------------------------------------

    void CLightProbeComponent::SetIntensity(float _Intensity)
    {
        m_Intensity = _Intensity;
    }

    // -----------------------------------------------------------------------------

    float CLightProbeComponent::GetIntensity() const
    {
        return m_Intensity;
    }

    // -----------------------------------------------------------------------------

    void CLightProbeComponent::SetNear(float _Near)
    {
        m_Near = _Near;
    }

    // -----------------------------------------------------------------------------

    float CLightProbeComponent::GetNear() const
    {
        return m_Near;
    }

    // -----------------------------------------------------------------------------

    void CLightProbeComponent::SetFar(float _Far)
    {
        m_Far = _Far;
    }

    // -----------------------------------------------------------------------------

    float CLightProbeComponent::GetFar() const
    {
        return m_Far;
    }

    void CLightProbeComponent::SetParallaxCorrection(bool _Flag)
    {
        m_ParallaxCorrection = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CLightProbeComponent::GetParallaxCorrection() const
    {
        return m_ParallaxCorrection;
    }

    // -----------------------------------------------------------------------------

    void CLightProbeComponent::SetBoxSize(const glm::vec3& _rSize)
    {
        m_BoxSize = _rSize;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CLightProbeComponent::GetBoxSize() const
    {
        return m_BoxSize;
    }
} // namespace Dt