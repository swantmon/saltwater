
#include "camera/cam_precompiled.h"

#include "base/base_lib_glm.h"

#include "core/core_time.h"

#include "base/base_input_event.h"
#include "base/base_math_operations.h"

#include "camera/cam_control_manager.h"
#include "camera/cam_editor_control.h"

#include "graphic/gfx_camera_interface.h"

#include <cmath>
#include <iostream>

namespace Cam
{
    const float CEditorControl::s_MoveVelocityBorder[] =  { 0.1f, 100.0f, 0.008f };
    const float CEditorControl::s_RotationVelocity     =  0.35f;
} // namespace Cam

namespace Cam
{
    CEditorControl::CEditorControl()
        : CControl            (CControl::EditorControl)
        , m_IsFlying          (false)
        , m_IsDragging        (false)
        , m_LastCursorPosition( )
        , m_CurrentRotation   ( )
        , m_MoveDirection     (0)
    {
        m_Position[0] = 0.0f;
        m_Position[1] = 0.0f;
        m_Position[2] = 10.0f;
    }

    // -----------------------------------------------------------------------------

    CEditorControl::~CEditorControl()
    {
    }

    // -----------------------------------------------------------------------------

    void CEditorControl::InternOnEvent(const Base::CInputEvent& _rEvent)
    {
        if (_rEvent.GetAction() == Base::CInputEvent::MouseMiddlePressed && !m_IsFlying)
        {
            m_IsDragging = true;

            m_LastCursorPosition = _rEvent.GetCursorPosition();
        }
        else if (_rEvent.GetAction() == Base::CInputEvent::MouseRightPressed && !m_IsDragging)
        {
            m_IsFlying = true;

            m_LastCursorPosition = _rEvent.GetCursorPosition();
        }
        else if (_rEvent.GetAction() == Base::CInputEvent::MouseMiddleReleased || _rEvent.GetAction() == Base::CInputEvent::MouseRightReleased)
        {
            m_IsFlying   = false;
            m_IsDragging = false;
        }
        else if (_rEvent.GetAction() == Base::CInputEvent::KeyPressed && m_IsFlying)
        {
            switch (_rEvent.GetKey())
            {
                case Base::CInputEvent::KeyW: m_MoveDirection |= 0x01; break;
                case Base::CInputEvent::KeyS: m_MoveDirection |= 0x02; break;
                case Base::CInputEvent::KeyA: m_MoveDirection |= 0x04; break;
                case Base::CInputEvent::KeyD: m_MoveDirection |= 0x08; break;
            }
        }
        else if (_rEvent.GetAction() == Base::CInputEvent::KeyReleased)
        {
            switch (_rEvent.GetKey())
            {
                case Base::CInputEvent::KeyW: m_MoveDirection &= ~0x01; break;
                case Base::CInputEvent::KeyS: m_MoveDirection &= ~0x02; break;
                case Base::CInputEvent::KeyA: m_MoveDirection &= ~0x04; break;
                case Base::CInputEvent::KeyD: m_MoveDirection &= ~0x08; break;
            }
        }
        else if (_rEvent.GetAction() == Base::CInputEvent::MouseWheel)
        {
            float WheelDelta;

            WheelDelta = static_cast<float>(_rEvent.GetWheelDelta()) * s_MoveVelocityBorder[2];

            m_MoveVelocity = Base::Clamp(m_MoveVelocity + WheelDelta, s_MoveVelocityBorder[0], s_MoveVelocityBorder[1]);
        }
        else if (_rEvent.GetAction() == Base::CInputEvent::MouseMove)
        {
            if (m_IsDragging)
            {
                glm::vec3 Forward(0.0f, 0.0f, -1.0f);
                glm::vec3 Right(1.0f, 0.0f, 0.0f);
                glm::vec3 Up(0.0f, 0.0f, 1.0f);

                const glm::vec2& rCursorPosition = _rEvent.GetCursorPosition();

                float DeltaTime = static_cast<float>(Core::Time::GetDeltaTimeLastFrame());

                float CurrentVelocityX = (rCursorPosition[0] - m_LastCursorPosition[0]);
                float CurrentVelocityY = (rCursorPosition[1] - m_LastCursorPosition[1]);

                Forward = Forward * m_RotationMatrix;
                Right   = Right   * m_RotationMatrix;
                Up      = glm::cross(Forward, Right);

                m_Position -= (Right * CurrentVelocityX * DeltaTime);
                m_Position -= (Up    * CurrentVelocityY * DeltaTime);

                m_LastCursorPosition = rCursorPosition;
            }

            if (m_IsFlying)
            {
                const glm::vec2& rCursorPosition = _rEvent.GetCursorPosition();

                m_CurrentRotation[0] += (rCursorPosition[0] - m_LastCursorPosition[0]) * s_RotationVelocity;
                m_CurrentRotation[1] += (rCursorPosition[1] - m_LastCursorPosition[1]) * s_RotationVelocity;

                if (m_CurrentRotation[0] < 0.0f)
                {
                    m_CurrentRotation[0] = 360.0f + m_CurrentRotation[0];
                }

                m_CurrentRotation[0] = Base::Modulo(m_CurrentRotation[0], 360.0f);

                if (m_CurrentRotation[1] < 0.0f)
                {
                    m_CurrentRotation[1] = 360.0f + m_CurrentRotation[1];
                }

                m_CurrentRotation[1] = Base::Modulo(m_CurrentRotation[1], 360.0f);

                glm::mat3 RotationX(1.0f);
                glm::mat3 RotationZ(1.0f);

                RotationX = glm::eulerAngleX(Base::DegreesToRadians(m_CurrentRotation[1]));
                RotationZ = glm::eulerAngleZ(Base::DegreesToRadians(m_CurrentRotation[0]));

                m_RotationMatrix = RotationX * RotationZ;

                m_LastCursorPosition = rCursorPosition;
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CEditorControl::InternOnDirtyEntity(Dt::CEntity* _pEntity)
    {
        BASE_UNUSED(_pEntity);
    }

    // -----------------------------------------------------------------------------

    void CEditorControl::InternUpdate()
    {
        glm::vec3 Forward(0.0f, 0.0f, -1.0f);
        glm::vec3 Right  (1.0f, 0.0f,  0.0f);

        float DeltaTime = static_cast<float>(Core::Time::GetDeltaTimeLastFrame());

        Forward = Forward * m_RotationMatrix;
        Right   = Right   * m_RotationMatrix;

        if (m_MoveDirection & 0x01) m_Position += (Forward * m_MoveVelocity * DeltaTime);
        if (m_MoveDirection & 0x02) m_Position -= (Forward * m_MoveVelocity * DeltaTime);
        if (m_MoveDirection & 0x04) m_Position -= (Right   * m_MoveVelocity * DeltaTime);
        if (m_MoveDirection & 0x08) m_Position += (Right   * m_MoveVelocity * DeltaTime);

        Gfx::Cam::SetPosition(m_Position);
        Gfx::Cam::SetRotationMatrix(m_RotationMatrix);
        Gfx::Cam::Update();
    }
} // namespace Cam