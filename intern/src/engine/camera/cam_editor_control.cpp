
#include "engine/engine_precompiled.h"

#include "base/base_include_glm.h"

#include "engine/core/core_time.h"

#include "base/base_input_event.h"

#include "engine/camera/cam_control_manager.h"
#include "engine/camera/cam_editor_control.h"

#include "engine/graphic/gfx_camera.h"
#include "engine/graphic/gfx_view_manager.h"

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

    void CEditorControl::SetProjectionMatrix(const glm::mat4& _rProjectionMatrix)
    {
        Gfx::CCamera& rCamera = *Gfx::ViewManager::GetMainCamera();

        float Near = glm::abs(_rProjectionMatrix[2][3] / (_rProjectionMatrix[2][2] - 1.0f));
        float Far  = glm::abs(_rProjectionMatrix[2][3] / (_rProjectionMatrix[2][2] + 1.0f));

        float Bottom = Near * (_rProjectionMatrix[2][1] - 1.0f) / _rProjectionMatrix[1][1];
        float Top    = Near * (_rProjectionMatrix[2][1] + 1.0f) / _rProjectionMatrix[1][1];
        float Left   = Near * (_rProjectionMatrix[2][0] - 1.0f) / _rProjectionMatrix[0][0];
        float Right  = Near * (_rProjectionMatrix[2][0] + 1.0f) / _rProjectionMatrix[0][0];

        rCamera.SetPerspective(Left, Right, Bottom, Top, Near, Far);
    }

    // -----------------------------------------------------------------------------

    void CEditorControl::SetRotation(const glm::mat3& _rRotationMatrix)
    {
        CControl::SetRotation(_rRotationMatrix);

        // -----------------------------------------------------------------------------

        float X, Y, Z;

        glm::extractEulerAngleXYZ(glm::mat4(m_RotationMatrix), X, Y, Z);

        m_CurrentRotation[0] = glm::degrees(Z);
        m_CurrentRotation[1] = glm::degrees(Y);
    }

    // -----------------------------------------------------------------------------

    void CEditorControl::InternOnEvent(const Base::CInputEvent& _rEvent)
    {
        if (_rEvent.GetAction() == Base::CInputEvent::MouseMiddlePressed && !m_IsFlying)
        {
            m_IsDragging = true;

            m_LastCursorPosition = _rEvent.GetGlobalCursorPosition();
        }
        else if (_rEvent.GetAction() == Base::CInputEvent::MouseRightPressed && !m_IsDragging)
        {
            m_IsFlying = true;

            m_LastCursorPosition = _rEvent.GetGlobalCursorPosition();
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

            WheelDelta = static_cast<float>(_rEvent.GetDelta()) * s_MoveVelocityBorder[2];

            m_MoveVelocity = glm::clamp(m_MoveVelocity + WheelDelta, s_MoveVelocityBorder[0], s_MoveVelocityBorder[1]);
        }
        else if (_rEvent.GetAction() == Base::CInputEvent::MouseMove)
        {
            if (m_IsDragging)
            {
                glm::vec3 Forward(0.0f, 0.0f, -1.0f);
                glm::vec3 Right(1.0f, 0.0f, 0.0f);
                glm::vec3 Up(0.0f, 0.0f, 1.0f);

                const glm::vec2& rCursorPosition = _rEvent.GetGlobalCursorPosition();

                auto DeltaTime = static_cast<float>(Core::Time::GetDeltaTimeLastFrame());

                glm::vec2 CurrentVelocity = (rCursorPosition - m_LastCursorPosition);

                Forward = m_RotationMatrix * Forward;
                Right   = m_RotationMatrix * Right;
                Up      = glm::cross(Forward, Right);

                m_Position -= (Right * CurrentVelocity[0] * DeltaTime);
                m_Position -= (Up    * CurrentVelocity[1] * DeltaTime);

                m_LastCursorPosition = rCursorPosition;
            }

            if (m_IsFlying)
            {
                const glm::vec2& rCursorPosition = _rEvent.GetGlobalCursorPosition();

                m_CurrentRotation -= (rCursorPosition - m_LastCursorPosition) * s_RotationVelocity;

                m_RotationMatrix = glm::eulerAngleZX(glm::radians(m_CurrentRotation[0]), glm::radians(m_CurrentRotation[1]));

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

    void CEditorControl::InternOnDirtyComponent(Dt::IComponent* _pComponent)
    {
        BASE_UNUSED(_pComponent);
    }

    // -----------------------------------------------------------------------------

    void CEditorControl::InternUpdate()
    {
        glm::vec3 Forward(0.0f, 0.0f, -1.0f);
        glm::vec3 Right  (1.0f, 0.0f,  0.0f);

        auto DeltaTime = static_cast<float>(Core::Time::GetDeltaTimeLastFrame());

        Forward = m_RotationMatrix * Forward;
        Right   = m_RotationMatrix * Right;

        if (m_MoveDirection & 0x01) m_Position += (Forward * m_MoveVelocity * DeltaTime);
        if (m_MoveDirection & 0x02) m_Position -= (Forward * m_MoveVelocity * DeltaTime);
        if (m_MoveDirection & 0x04) m_Position -= (Right   * m_MoveVelocity * DeltaTime);
        if (m_MoveDirection & 0x08) m_Position += (Right   * m_MoveVelocity * DeltaTime);

        Gfx::CCamera& rCamera = *Gfx::ViewManager::GetMainCamera();
        Gfx::CView&   rView = *rCamera.GetView();

        rView.SetPosition(m_Position);
        rView.SetRotationMatrix(m_RotationMatrix);

        rView.Update();
    }
} // namespace Cam