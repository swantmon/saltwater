
#include "mr/mr_precompiled.h"

#include "mr/mr_control.h"

namespace MR
{
    CControl::CControl(EType _Type)
        : m_Type            (_Type)
        , m_IsStarted       (false)
        , m_IsFreezed       (false)
        , m_pOriginalFrame  (nullptr)
        , m_pConvertedFrame (nullptr)
        , m_CameraParameters()
        , m_ProjectionMatrix(Base::Float3x3::s_Identity)
    {
    }

    // -----------------------------------------------------------------------------

    CControl::~CControl()
    {
    }

    // -----------------------------------------------------------------------------

    void CControl::Start(const Base::Char* m_pCameraParameterFile)
    {
        m_IsStarted = true;

        InternStart(m_pCameraParameterFile);
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
    
    void CControl::FreezeOuput(bool _Flag)
    {
        m_IsFreezed = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CControl::IsStarted() const
    {
        return m_IsStarted;
    }

    // -----------------------------------------------------------------------------

    CControl::EType CControl::GetType() const
    {
        return m_Type;
    }

    // -----------------------------------------------------------------------------

    void CControl::SetConvertedFrame(Dt::CTexture2D* _pTexture)
    {
        m_pConvertedFrame = _pTexture;
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CControl::GetConvertedFrame()
    {
        return m_pConvertedFrame;
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CControl::GetOriginalFrame()
    {
        return m_pOriginalFrame;
    }

    // -----------------------------------------------------------------------------

    SDeviceParameter& CControl::GetCameraParameters()
    {
        return m_CameraParameters;
    }

    // -----------------------------------------------------------------------------

    Base::Float3x3& CControl::GetProjectionMatrix()
    {
        return m_ProjectionMatrix;
    }

    
} // namespace AR