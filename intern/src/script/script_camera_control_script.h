
#pragma once

#include "base/base_include_glm.h"

#include "core/core_time.h"

#include "data/data_transformation_facet.h"

#include "script/script_script.h"

namespace Scpt
{
    class CCameraControlScript : public IScript
    {
    public:

        bool m_IsFlying = false;
        glm::vec2 m_LastCursorPosition;
        glm::vec2 m_CurrentRotation;
        unsigned int m_MoveDirection = 0;
        glm::mat3 m_RotationMatrix;
        glm::vec3 m_Position;
        float m_MovingSpeed = 50.0f;
        float m_RotationSpeed = 0.35f;

    public:

        void Start() override
        {
            m_RotationMatrix = glm::toMat3(GetTransformation()->GetRotation());
            m_Position       = GetTransformation()->GetPosition();
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {

        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            glm::vec3 Forward(0.0f, 0.0f, -1.0f);
            glm::vec3 Right(1.0f, 0.0f, 0.0f);

            float DeltaTime = static_cast<float>(Core::Time::GetDeltaTimeLastFrame());

            Forward = m_RotationMatrix * Forward;
            Right   = m_RotationMatrix * Right;

            if (m_MoveDirection & 0x01) m_Position += (Forward * m_MovingSpeed * DeltaTime);
            if (m_MoveDirection & 0x02) m_Position -= (Forward * m_MovingSpeed * DeltaTime);
            if (m_MoveDirection & 0x04) m_Position -= (Right   * m_MovingSpeed * DeltaTime);
            if (m_MoveDirection & 0x08) m_Position += (Right   * m_MovingSpeed * DeltaTime);

            GetTransformation()->SetPosition(m_Position);
            GetTransformation()->SetRotation(glm::toQuat(m_RotationMatrix));

            Dt::EntityManager::MarkEntityAsDirty(*GetEntity(), Dt::CEntity::DirtyMove);
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            if (_rEvent.GetAction() == Base::CInputEvent::MouseRightPressed)
            {
                m_IsFlying = true;

                m_LastCursorPosition = _rEvent.GetCursorPosition();
            }
            else if (_rEvent.GetAction() == Base::CInputEvent::MouseRightReleased)
            {
                m_IsFlying = false;
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
            else if (_rEvent.GetAction() == Base::CInputEvent::MouseMove)
            {
                if (m_IsFlying)
                {
                    const glm::vec2& rCursorPosition = _rEvent.GetCursorPosition();

                    m_CurrentRotation -= (rCursorPosition - m_LastCursorPosition) * m_RotationSpeed;

                    m_RotationMatrix = glm::eulerAngleZX(glm::radians(m_CurrentRotation[0]), glm::radians(m_CurrentRotation[1]));

                    m_LastCursorPosition = rCursorPosition;
                }
            }
        }
    };
} // namespace Scpt