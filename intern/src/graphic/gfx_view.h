
#pragma once

#include "base/base_include_glm.h"
#include "base/base_managed_pool.h"

namespace Gfx
{
    class CCamera;
} // namespace Gfx

namespace Gfx
{
    class CView : public Base::CManagedPoolItemBase
    {
        public:

            void SetRotationMatrix(const glm::mat3& _rMatrix);
            const glm::mat3& GetRotationMatrix() const;

            void SetPosition(float _AxisX, float _AxisY, float _AxisZ);
            void SetPosition(const glm::vec3& _rPosition);
            const glm::vec3& GetPosition() const;

            const glm::vec3& GetViewDirection() const;
            const glm::vec3& GetRightDirection() const;
            const glm::vec3& GetUpDirection() const;

            void SetViewMatrix(const glm::mat4& _rViewMatrix);
            const glm::mat4& GetViewMatrix() const;

        public:

            void Update();
		
        protected:

            glm::mat4 m_ViewMatrix;
            glm::mat3 m_RotationMatrix;

            glm::vec3 m_Position;
            glm::vec3 m_View;
            glm::vec3 m_Right;
            glm::vec3 m_Up;

            CCamera* m_pFirstCamera;

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
