
#pragma once

#include "base/base_input_event.h"
#include "base/base_include_glm.h"

namespace Dt
{
    class CEntity;
    class IComponent;
} // namespace Dt

namespace Cam
{
    class ENGINE_API CControl
    {
        public:

            enum EType
            {
                EditorControl,
                GameControl,
                NumberOfControls,
                Undefined = -1,
            };

        public:

            virtual ~CControl();

        public:

            void OnEvent(const Base::CInputEvent& _rEvent);
            void OnDirtyEntity(Dt::CEntity* _pEntity);
            void OnDirtyComponent(Dt::IComponent* _pComponent);

        public:

            void Update();

        public:

            EType GetType() const;

        public:

            void SetMoveVelocity(float _Velocity);
            float GetMoveVelocity() const;

            void SetPosition(const glm::vec3& _rPosition);
            const glm::vec3& GetPosition() const;

            void SetRotation(const glm::mat3& _rMatrix);
            const glm::mat3& GetRotation() const;

        public:

            bool HasPositionChanged() const;
            bool HasRotationChanged() const;
            bool HasMoveVelocityChanged() const;

        protected:

            EType     m_Type;
            glm::vec3 m_Position;
            glm::mat3 m_RotationMatrix;
            float     m_MoveVelocity;
            bool      m_HasPositionChanged;
            bool      m_HasRotationChanged;
            bool      m_HasMoveVelocityChanged;

        protected:

            CControl(EType _Type);

        private:

            virtual void InternOnEvent(const Base::CInputEvent& _rEvent) = 0;

            virtual void InternOnDirtyEntity(Dt::CEntity* _pEntity) = 0;

            virtual void InternOnDirtyComponent(Dt::IComponent* _pComponent) = 0;

        private:

            virtual void InternUpdate() = 0;
	};
} // namespace Cam
