
#pragma once

#include "base/base_managed_pool.h"
#include "base/base_matrix3x3.h"
#include "base/base_matrix4x4.h"
#include "base/base_vector3.h"

namespace Gfx
{
    class CCamera;
} // namespace Gfx

namespace Gfx
{
    class CView : public Base::CManagedPoolItemBase
    {
        public:

            void SetRotationMatrix();
            void SetRotationMatrix(const Base::Float3x3& _rMatrix);
            const Base::Float3x3& GetRotationMatrix() const;

            void SetPosition(float _AxisX, float _AxisY, float _AxisZ);
            void SetPosition(const Base::Float3& _rPosition);
            const Base::Float3& GetPosition() const;

            const Base::Float3& GetViewDirection() const;
            const Base::Float3& GetRightDirection() const;
            const Base::Float3& GetUpDirection() const;

            void SetViewMatrix(const Base::Float4x4& _rViewMatrix);
            const Base::Float4x4& GetViewMatrix() const;

        public:

            void Update();
		
        protected:

            Base::Float4x4 m_ViewMatrix;
            Base::Float3x3 m_RotationMatrix;

            Base::Float3   m_Position;
            Base::Float3   m_View;
            Base::Float3   m_Right;
            Base::Float3   m_Up;

            CCamera*       m_pFirstCamera;

        protected:

            CView();
           ~CView();

        private:

            friend class CCamera;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CView> CViewPtr;
} // namespace Gfx
