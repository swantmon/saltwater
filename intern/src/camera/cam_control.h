
#pragma once

#include "base/base_input_event.h"
#include "base/base_matrix3x3.h"
#include "base/base_vector3.h"

namespace Dt
{
    class CEntity;
} // namespace Dt

namespace Cam
{
	class CControl
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

        public:

            void Update();

        public:

            EType GetType() const;

        public:

            void SetMoveVelocity(float _Velocity);
            float GetMoveVelocity() const;

            void SetPosition(const Base::Float3& _rPosition);
            const Base::Float3& GetPosition() const;

            void SetRotation(const Base::Float3x3& _rMatrix);
            const Base::Float3x3& GetRotation() const;

        public:

            bool HasPositionChanged() const;
            bool HasRotationChanged() const;
            bool HasMoveVelocityChanged() const;

        protected:

            EType          m_Type;
            Base::Float3   m_Position;
            Base::Float3x3 m_RotationMatrix;
            float          m_MoveVelocity;
            bool           m_HasPositionChanged;
            bool           m_HasRotationChanged;
            bool           m_HasMoveVelocityChanged;

        protected:

            CControl(EType _Type);

        private:

            virtual void InternOnEvent(const Base::CInputEvent& _rEvent) = 0;

            virtual void InternOnDirtyEntity(Dt::CEntity* _pEntity) = 0;

        private:

            virtual void InternUpdate() = 0;
	};
} // namespace Cam
