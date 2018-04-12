
#include "engine/engine_precompiled.h"

#include "engine/data/data_sky_component.h"

#include <string>

namespace Dt
{
    CSkyComponent::CSkyComponent()
        : m_RefreshMode(Static)
        , m_Type       (Procedural)
        , m_HasHDR     (true)
        , m_TexturePtr (nullptr)
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

    void CSkyComponent::SetTexture(Gfx::CTexturePtr _TexturePtr)
    {
        m_TexturePtr = _TexturePtr;
    }

    // -----------------------------------------------------------------------------

    Gfx::CTexturePtr CSkyComponent::GetTexture()
    {
        return m_TexturePtr;
    }

    // -----------------------------------------------------------------------------

    const Gfx::CTexturePtr CSkyComponent::GetTexture() const
    {
        return m_TexturePtr;
    }

    // -----------------------------------------------------------------------------

    bool CSkyComponent::HasTexture() const
    {
        return m_TexturePtr != nullptr;
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