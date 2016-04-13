
#include "mr/mr_control.h"

namespace MR
{
    CControl::CControl(EType _Type)
        : m_Type     (_Type)
        , m_IsStarted(false)
    {
    }

    // -----------------------------------------------------------------------------

    CControl::~CControl()
    {
    }

    // -----------------------------------------------------------------------------

    void CControl::Start(const SControlDescription& _rDescriptor)
    {
        m_IsStarted = true;

        InternStart(_rDescriptor);
    }

    // -----------------------------------------------------------------------------

    void CControl::Stop()
    {
        InternStop();

        m_IsStarted = false;
    }

    // -----------------------------------------------------------------------------

    void CControl::Update()
    {
        InternUpdate();
    }

    // -----------------------------------------------------------------------------

    CControl::EType CControl::GetType() const
    {
        return m_Type;
    }

    // -----------------------------------------------------------------------------

    bool CControl::IsStarted() const
    {
        return m_IsStarted;
    }

    // -----------------------------------------------------------------------------

    SDeviceParameter& CControl::GetCameraParameters()
    {
        return InternGetCameraParameters();
    }

    // -----------------------------------------------------------------------------

    Base::Float3x3& CControl::GetProjectionMatrix()
    {
        return InternGetProjectionMatrix();
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CControl::GetOriginalFrame()
    {
        return InternGetOriginalFrame();
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CControl::GetConvertedFrame()
    {
        return InternGetConvertedFrame();
    }

    // -----------------------------------------------------------------------------

    Dt::CTextureCube* CControl::GetCubemap()
    {
        return InternGetCubemap();
    }
} // namespace AR