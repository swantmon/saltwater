
#include "camera/cam_precompiled.h"

#include "camera/cam_control.h"

namespace Cam
{
    CControl::CControl(EType _Type)
        : m_Type                  (_Type)
        , m_Position              (Base::Float3::s_Zero)
        , m_RotationMatrix        (Base::Float3x3::s_Identity)
        , m_MoveVelocity          (10.0f)
        , m_HasPositionChanged    (false)
        , m_HasRotationChanged    (false)
        , m_HasMoveVelocityChanged(false)
    {
    }

    // -----------------------------------------------------------------------------

    CControl::~CControl()
    {
    }

    // -----------------------------------------------------------------------------

    void CControl::OnEvent(const Base::CInputEvent& _rEvent)
    {
        InternOnEvent(_rEvent);
    }

    // -----------------------------------------------------------------------------

    void CControl::Update()
    {
        m_HasPositionChanged     = false;
        m_HasRotationChanged     = false;
        m_HasMoveVelocityChanged = false;

        InternUpdate();
    }

    // -----------------------------------------------------------------------------

    CControl::EType CControl::GetType() const
    {
        return m_Type;
    }

    // -----------------------------------------------------------------------------

    void CControl::SetMoveVelocity(float _Velocity)
    {
        m_MoveVelocity = _Velocity;
    }

    // -----------------------------------------------------------------------------

    float CControl::GetMoveVelocity() const
    {
        return m_MoveVelocity;
    }

    // -----------------------------------------------------------------------------

    void CControl::SetPosition(const Base::Float3& _rPosition)
    {
        m_Position = _rPosition;
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& CControl::GetPosition() const
    {
        return m_Position;
    }

    // -----------------------------------------------------------------------------

    void CControl::SetRotation(const Base::Float3x3& _rMatrix)
    {
        m_RotationMatrix = _rMatrix;
    }

    // -----------------------------------------------------------------------------

    const Base::Float3x3& CControl::GetRotation() const
    {
        return m_RotationMatrix;
    }

    // -----------------------------------------------------------------------------

    bool CControl::HasPositionChanged() const
    {
        return m_HasPositionChanged;
    }

    // -----------------------------------------------------------------------------

    bool CControl::HasRotationChanged() const
    {
        return m_HasRotationChanged;
    }

    // -----------------------------------------------------------------------------

    bool CControl::HasMoveVelocityChanged() const
    {
        return m_HasMoveVelocityChanged;
    }
} // namespace Cam