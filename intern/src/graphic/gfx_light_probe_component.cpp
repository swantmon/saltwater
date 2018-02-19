
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_light_probe_component.h"

namespace Gfx
{
    CLightProbeComponent::CLightProbeComponent()
        : m_DiffusePtr ()
        , m_SpecularPtr()
        , m_DepthPtr   ()
        , m_TimeStamp  (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CLightProbeComponent::~CLightProbeComponent()
    {
        m_DiffusePtr  = 0;
        m_SpecularPtr = 0;
        m_DepthPtr    = 0;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CLightProbeComponent::GetDiffusePtr() const
    {
        return m_DiffusePtr;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CLightProbeComponent::GetSpecularPtr() const
    {
        return m_SpecularPtr;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CLightProbeComponent::GetDepthPtr() const
    {
        return m_DepthPtr;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CLightProbeComponent::GetTimeStamp() const
    {
        return m_TimeStamp;
    }
} // namespace Gfx