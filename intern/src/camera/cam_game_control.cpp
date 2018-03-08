
#include "camera/cam_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_type_info.h"

#include "camera/cam_control_manager.h"
#include "camera/cam_game_control.h"

#include "data/data_camera_component.h"
#include "data/data_component.h"
#include "data/data_component_facet.h"
#include "data/data_entity.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_camera_interface.h"

namespace Cam
{
    CGameControl::CGameControl()
        : CControl           (CControl::GameControl)
        , m_pMainCameraEntity(nullptr)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGameControl::~CGameControl()
    {
    }

    // -----------------------------------------------------------------------------

    void CGameControl::SetEntity(Dt::CEntity& _rEntity)
    {
        m_pMainCameraEntity = &_rEntity;

        OnDirtyEntity(m_pMainCameraEntity);
    }

    // -----------------------------------------------------------------------------

    Dt::CEntity* CGameControl::GetEntity()
    {
        return m_pMainCameraEntity;
    }

    // -----------------------------------------------------------------------------

    const Dt::CEntity* CGameControl::GetEntity() const
    {
        return m_pMainCameraEntity;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGameControl::InternOnEvent(const Base::CInputEvent& _rEvent)
    {
        BASE_UNUSED(_rEvent);
    }

    // -----------------------------------------------------------------------------

    void CGameControl::InternOnDirtyEntity(Dt::CEntity* _pEntity)
    {
        assert(_pEntity != 0);

        if (m_pMainCameraEntity != _pEntity) return;

        auto pCameraComponent = m_pMainCameraEntity->GetComponentFacet()->GetComponent<Dt::CCameraComponent>();

        assert(pCameraComponent != nullptr);

        if (pCameraComponent->GetProjectionType() == Dt::CCameraComponent::External) return;

        if (m_pMainCameraEntity->GetDirtyFlags() & Dt::CEntity::DirtyMove)
        {
            Dt::CTransformationFacet* pTransformationFacet = m_pMainCameraEntity->GetTransformationFacet();

            assert(pTransformationFacet != nullptr);

            // -----------------------------------------------------------------------------
            // Position
            // -----------------------------------------------------------------------------
            m_Position = m_pMainCameraEntity->GetWorldPosition();

            // -----------------------------------------------------------------------------
            // Rotation
            // -----------------------------------------------------------------------------
            m_RotationMatrix = glm::toMat3(pTransformationFacet->GetRotation());
        }

        // -----------------------------------------------------------------------------
        // Projection
        // -----------------------------------------------------------------------------
        if (pCameraComponent->GetProjectionType() == Dt::CCameraComponent::Perspective)
        {
            Gfx::Cam::SetFieldOfView(pCameraComponent->GetFoV(), pCameraComponent->GetNear(), pCameraComponent->GetFar());
        }
        else if (pCameraComponent->GetProjectionType() == Dt::CCameraComponent::Orthographic)
        {
            float Left   = -pCameraComponent->GetSize() / 2.0f;
            float Right  =  pCameraComponent->GetSize() / 2.0f;
            float Bottom = -pCameraComponent->GetSize() / 2.0f;
            float Top    =  pCameraComponent->GetSize() / 2.0f;
                
            Gfx::Cam::SetOrthographic(Left, Right, Bottom, Top, pCameraComponent->GetNear(), pCameraComponent->GetFar());
        }

        // -----------------------------------------------------------------------------
        // Camera mode + variables
        // -----------------------------------------------------------------------------
        Gfx::Cam::SetAutoCameraMode();
            
        if (pCameraComponent->GetCameraMode() == Dt::CCameraComponent::Manual)
        {
            Gfx::Cam::SetManualCameraMode();

            Gfx::Cam::SetShutterSpeed(pCameraComponent->GetShutterSpeed());

            Gfx::Cam::SetAperture(pCameraComponent->GetAperture());

            Gfx::Cam::SetISO(pCameraComponent->GetISO());

            Gfx::Cam::SetEC(pCameraComponent->GetEC());
        }

        // -----------------------------------------------------------------------------
        // Other
        // -----------------------------------------------------------------------------
        Gfx::Cam::SetBackgroundColor(pCameraComponent->GetBackgroundColor());
            
        Gfx::Cam::SetCullingMask(pCameraComponent->GetCullingMask());

        Gfx::Cam::SetViewportRect(pCameraComponent->GetViewportRect());

        Gfx::Cam::SetDepth(pCameraComponent->GetDepth());
    }
    
    // -----------------------------------------------------------------------------
    
    void CGameControl::InternUpdate()
    {
        if (m_pMainCameraEntity != 0)
        {
            Dt::CCameraComponent* pCameraComponent = m_pMainCameraEntity->GetComponentFacet()->GetComponent<Dt::CCameraComponent>();

            if (pCameraComponent->GetProjectionType() == Dt::CCameraComponent::External) return;
        }

        Gfx::Cam::SetPosition(m_Position);
        Gfx::Cam::SetRotationMatrix(m_RotationMatrix);

        Gfx::Cam::Update();
    }
} // namespace Cam