
#pragma once

#include "engine/camera/cam_control.h"

#include "base/base_input_event.h"
#include "base/base_include_glm.h"

namespace Cam
{
    class ENGINE_API CEditorControl : public CControl
    {
    public:

        CEditorControl();
       ~CEditorControl();

    public:

        void SetProjectionMatrix(const glm::mat4& _rProjectionMatrix);

        void SetRotation(const glm::mat3& _rRotationMatrix);

        bool IsFlying();

        bool IsDragging();

    private:

        static const float s_MoveVelocityBorder[];
        static const float s_RotationVelocity;

    private:

        bool          m_IsFlying;
        bool          m_IsDragging;
        glm::vec2     m_LastCursorPosition;
        glm::vec2     m_CurrentRotation;
        unsigned int  m_MoveDirection;

    private:

        void InternOnEvent(const Base::CInputEvent& _rEvent) override;
        void InternOnDirtyEntity(Dt::CEntity* _pEntity) override;
        void InternOnDirtyComponent(Dt::IComponent* _pComponent) override;

    private:

        virtual void InternUpdate();
    };
}