
#include "camera/cam_control_manager.h"
#include "camera/cam_game_control.h"

#include "graphic/gfx_camera_interface.h"

namespace Cam
{
    CGameControl::CGameControl()
        : CControl    (CControl::GameControl)
    {
        m_Position[0] = 0.0f;
        m_Position[1] = 0.0f;
        m_Position[2] = 1.0f;
    }
    
    // -----------------------------------------------------------------------------
    
    CGameControl::~CGameControl()
    {

        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGameControl::SetProjectionMatrix(const Base::Float3x3& _rProjectionMatrix)
    {
        Gfx::Cam::InjectCameraMatrix(_rProjectionMatrix);
    }

    // -----------------------------------------------------------------------------

    void CGameControl::SetMarkerTransformation(const Base::Float3x3& _rRotationMatrix, const Base::Float3& _rTranslation)
    {
        m_RotationMatrix[0][0] = _rRotationMatrix[0][0];
        m_RotationMatrix[0][1] = _rRotationMatrix[0][1];
        m_RotationMatrix[0][2] = _rRotationMatrix[0][2];

        m_RotationMatrix[1][0] = -_rRotationMatrix[1][0];
        m_RotationMatrix[1][1] = -_rRotationMatrix[1][1];
        m_RotationMatrix[1][2] = -_rRotationMatrix[1][2];

        m_RotationMatrix[2][0] = -_rRotationMatrix[2][0];
        m_RotationMatrix[2][1] = -_rRotationMatrix[2][1];
        m_RotationMatrix[2][2] = -_rRotationMatrix[2][2];

        m_Position[0] = -_rTranslation[0];
        m_Position[1] =  _rTranslation[1];
        m_Position[2] =  _rTranslation[2];

        m_Position = m_RotationMatrix.GetInverted() * m_Position;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGameControl::InternOnEvent(const Base::CInputEvent& _rEvent)
    {
        BASE_UNUSED(_rEvent);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGameControl::InternUpdate()
    {
        Gfx::Cam::SetPosition(m_Position);
        Gfx::Cam::SetRotation(m_RotationMatrix);
        Gfx::Cam::Update();
    }
} // namespace Cam