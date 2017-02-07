
#pragma once

#include "camera/cam_control.h"

#include "base/base_input_event.h"
#include "base/base_vector2.h"

namespace Cam
{
    class CEditorControl : public CControl
    {
    public:

        CEditorControl();
            ~CEditorControl();

    private:

        static const float s_MoveVelocityBorder[];
        static const float s_RotationVelocity;

    private:

        bool          m_IsFlying;
        bool          m_IsDragging;
        Base::Short2  m_LastCursorPosition;
        Base::Float3  m_CurrentRotation;
        unsigned int  m_MoveDirection;

    private:

        virtual void InternOnEvent(const Base::CInputEvent& _rEvent);
        virtual void InternOnDirtyEntity(Dt::CEntity* _pEntity);

    private:

        virtual void InternUpdate();
    };
}