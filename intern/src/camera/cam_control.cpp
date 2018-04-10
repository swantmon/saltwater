
#include "engine/engine_precompiled.h"

#include "camera/cam_control.h"

namespace Cam
{
    CControl::CControl(EType _Type)
        : m_Type                  (_Type)
        , m_Position              (0.0f)
        , m_RotationMatrix        (1.0f)
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

    void CControl::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        InternOnDirtyEntity(_pEntity);
    }

    // -----------------------------------------------------------------------------

    void CControl::OnDirtyComponent(Dt::IComponent* _pComponent)
    {
        InternOnDirtyComponent(_pComponent);
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

    void CControl::SetPosition(const glm::vec3& _rPosition)
    {
        m_Position = _rPosition;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CControl::GetPosition() const
    {
        return m_Position;
    }

    // -----------------------------------------------------------------------------

    void CControl::SetRotation(const glm::mat3& _rMatrix)
    {
        m_RotationMatrix = _rMatrix;
    }

    // -----------------------------------------------------------------------------

    const glm::mat3& CControl::GetRotation() const
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