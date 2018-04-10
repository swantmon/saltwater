
#include "engine/engine_precompiled.h"

#include "data/data_sky_component.h"

#include <string>

namespace Dt
{
    CSkyComponent::CSkyComponent()
        : m_RefreshMode(Static)
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

    void CSkyComponent::SetTexture(const std::string& _pTexture2D)
    {
        m_Texture = _pTexture2D;
    }

    // -----------------------------------------------------------------------------

    const std::string& CSkyComponent::GetTexture()
    {
        return m_Texture;
    }

    // -----------------------------------------------------------------------------

    bool CSkyComponent::GetHasTexture() const
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