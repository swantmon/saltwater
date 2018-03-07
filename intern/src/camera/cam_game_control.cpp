
#include "camera/cam_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_type_info.h"

#include "camera/cam_control_manager.h"
#include "camera/cam_game_control.h"

#include "data/data_camera_component.h"
#include "data/data_component.h"
#include "data/data_component_facet.h"
#include "data/data_component_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_transformation_facet.h"

#include "graphic/gfx_camera_interface.h"

namespace Cam
{
    CGameControl::CGameControl()
        : CControl        (CControl::GameControl)
        , m_pRelatedEntity(nullptr)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGameControl::~CGameControl()
    {
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

        auto DirtyFlag = _pEntity->GetDirtyFlags();

        if ((DirtyFlag & Dt::CEntity::DirtyAdd) != 0)
        {
            if (_pEntity->GetComponentFacet()->HasComponent<Dt::CCameraComponent>())
            {
                auto pCameraComponent = _pEntity->GetComponentFacet()->GetComponent<Dt::CCameraComponent>();

                if (_pEntity->GetID() < m_pRelatedEntity->GetID())
                {
                    m_pRelatedEntity = _pEntity;

                    UpdateTransformation(m_pRelatedEntity);

                    UpdateSettings(pCameraComponent);
                }
            }
        }

        if ((DirtyFlag & Dt::CEntity::DirtyRemove) != 0 && _pEntity == m_pRelatedEntity)
        {
            m_pRelatedEntity = 0;

            LookupNewRelatedEntity();
        }

        if ((DirtyFlag & Dt::CEntity::DirtyComponent) != 0 && m_pRelatedEntity == _pEntity)
        {
            if (m_pRelatedEntity->GetComponentFacet()->HasComponent<Dt::CCameraComponent>() == false)
            {
                m_pRelatedEntity = 0;

                LookupNewRelatedEntity();
            }
        }
        
        if ((DirtyFlag & Dt::CEntity::DirtyMove) != 0 && m_pRelatedEntity == _pEntity)
        {
            UpdateTransformation(m_pRelatedEntity);
        }
    }

    // -----------------------------------------------------------------------------

    void CGameControl::InternOnDirtyComponent(Dt::IComponent* _pComponent)
    {
        if (_pComponent->GetTypeID() != Base::CTypeInfo::GetTypeID<Dt::CCameraComponent>()) return;

        if (_pComponent->GetHostEntity() == m_pRelatedEntity)
        {
            UpdateSettings(_pComponent);
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGameControl::InternUpdate()
    {
        if (m_pRelatedEntity == nullptr)
        {
            LookupNewRelatedEntity();
        }

        Gfx::Cam::SetPosition(m_Position);
        Gfx::Cam::SetRotationMatrix(m_RotationMatrix);

        Gfx::Cam::Update();
    }

    // -----------------------------------------------------------------------------

    void CGameControl::UpdateTransformation(Dt::CEntity* _pEntity)
    {
        assert(_pEntity != nullptr);

        Dt::CTransformationFacet* pTransformationFacet = m_pRelatedEntity->GetTransformationFacet();

        assert(pTransformationFacet != nullptr);

        // -----------------------------------------------------------------------------
        // Position
        // -----------------------------------------------------------------------------
        m_Position = m_pRelatedEntity->GetWorldPosition();

        // -----------------------------------------------------------------------------
        // Rotation
        // -----------------------------------------------------------------------------
        glm::vec3& rRotation = pTransformationFacet->GetRotation();

        m_RotationMatrix = glm::eulerAngleXYZ(rRotation[0], rRotation[1], rRotation[2]);
    }

    // -----------------------------------------------------------------------------

    void CGameControl::UpdateSettings(Dt::IComponent* _pComponent)
    {
        assert(_pComponent != nullptr);

        auto pCameraComponent = static_cast<Dt::CCameraComponent*>(_pComponent);

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
        else if (pCameraComponent->GetProjectionType() == Dt::CCameraComponent::External)
        {
            Gfx::Cam::SetProjectionMatrix(pCameraComponent->GetProjectionMatrix(), pCameraComponent->GetNear(), pCameraComponent->GetFar());
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

    void CGameControl::LookupNewRelatedEntity()
    {
        auto CameraComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CCameraComponent>();

        for (auto pComponent : CameraComponents)
        {
            auto* pCameraComponent = static_cast<Dt::CCameraComponent*>(pComponent);

            if (pCameraComponent->IsActiveAndUsable())
            {
                m_pRelatedEntity = Dt::EntityManager::GetEntityByID(pCameraComponent->GetHostEntity()->GetID());

                UpdateTransformation(m_pRelatedEntity);

                UpdateSettings(pCameraComponent);

                break;
            }
        }
    }
} // namespace Cam