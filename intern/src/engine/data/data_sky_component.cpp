
#include "engine/engine_precompiled.h"

#include "engine/data/data_sky_component.h"

#include <string>

namespace Dt
{
    REGISTER_COMPONENT_SER(CSkyComponent);

    CSkyComponent::CSkyComponent()
        : m_RefreshMode(Static)
        , m_Quality    (PX1024)
        , m_Type       (Procedural)
        , m_HasHDR     (true)
        , m_Texture    ("")
        , m_Intensity  (0.0f)
    {

    }

    // -----------------------------------------------------------------------------

    CSkyComponent::~CSkyComponent()
    {

    }

    // -----------------------------------------------------------------------------

    void CSkyComponent::SetRefreshMode(ERefreshMode _RefreshMode)
    {
        m_RefreshMode = _RefreshMode;
    }

    // -----------------------------------------------------------------------------

    CSkyComponent::ERefreshMode CSkyComponent::GetRefreshMode()
    {
        return m_RefreshMode;
    }

    // -----------------------------------------------------------------------------

    void CSkyComponent::SetType(EType _Type)
    {
        m_Type = _Type;
    }

    // -----------------------------------------------------------------------------

    CSkyComponent::EType CSkyComponent::GetType() const
    {
        return m_Type;
    }

    // -----------------------------------------------------------------------------

    void CSkyComponent::SetQuality(EQuality _Quality)
    {
        m_Quality = _Quality;
    }

    // -----------------------------------------------------------------------------

    CSkyComponent::EQuality CSkyComponent::GetQuality() const
    {
        return m_Quality;
    }

    // -----------------------------------------------------------------------------

    unsigned int CSkyComponent::GetQualityInPixel() const
    {
        static unsigned int s_QualityInPixel[s_NumberOfQualities] = { 64, 128, 256, 512, 1024, 2048 };

        return s_QualityInPixel[m_Quality];
    }

    // -----------------------------------------------------------------------------

    void CSkyComponent::SetTexture(const std::string& _rTexture)
    {
        m_Texture = _rTexture;
    }

    // -----------------------------------------------------------------------------

    const std::string& CSkyComponent::GetTexture() const
    {
        return m_Texture;
    }

    // -----------------------------------------------------------------------------

    bool CSkyComponent::HasTexture() const
    {
        return m_Texture.length() > 0;
    }

    // -----------------------------------------------------------------------------

    void CSkyComponent::SetIntensity(float _Intensity)
    {
        m_Intensity = _Intensity;
    }

    // -----------------------------------------------------------------------------

    float CSkyComponent::GetIntensity() const
    {
        return m_Intensity;
    }
} // namespace Dt