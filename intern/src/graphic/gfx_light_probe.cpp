
#include "engine/engine_precompiled.h"

#include "graphic/gfx_light_probe.h"

namespace Gfx
{
    CLightProbe::CLightProbe()
        : m_DiffusePtr ()
        , m_SpecularPtr()
        , m_DepthPtr   ()
        , m_TimeStamp  (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CLightProbe::~CLightProbe()
    {
        m_DiffusePtr  = 0;
        m_SpecularPtr = 0;
        m_DepthPtr    = 0;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CLightProbe::GetDiffusePtr() const
    {
        return m_DiffusePtr;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CLightProbe::GetSpecularPtr() const
    {
        return m_SpecularPtr;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CLightProbe::GetDepthPtr() const
    {
        return m_DepthPtr;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CLightProbe::GetTimeStamp() const
    {
        return m_TimeStamp;
    }
} // namespace Gfx