
#include "engine/engine_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_type_info.h"

#include "engine/camera/cam_control_manager.h"
#include "engine/camera/cam_game_control.h"

#include "engine/core/core_console.h"

#include "engine/data/data_camera_component.h"
#include "engine/data/data_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/graphic/gfx_camera.h"
#include "engine/graphic/gfx_view_manager.h"

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

        if ((DirtyFlag & Dt::CEntity::DirtyRemove) != 0 && _pEntity == m_pRelatedEntity)
        {
            m_pRelatedEntity = 0;

            LookupNewRelatedEntity();
        }

        if ((DirtyFlag & Dt::CEntity::DirtyComponent) != 0 && m_pRelatedEntity == _pEntity)
        {
            if (m_pRelatedEntity->GetComponentFacet()->HasComponent<Dt::CCameraComponent>() == false || m_pRelatedEntity->GetComponentFacet()->GetComponent<Dt::CCameraComponent>()->IsActiveAndUsable() == false)
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
        else
        {
            Gfx::CCamera& rCamera = *Gfx::ViewManager::GetMainCamera();
            Gfx::CView&   rView   = *rCamera.GetView();

            rView.SetPosition(m_Position);
            rView.SetRotationMatrix(m_RotationMatrix);

            rView.Update();
        }
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
        m_Position = pTransformationFacet->GetPosition();

        // -----------------------------------------------------------------------------
        // Rotation
        // -----------------------------------------------------------------------------
        m_RotationMatrix = glm::toMat3(pTransformationFacet->GetRotation());
    }

    // -----------------------------------------------------------------------------

    void CGameControl::UpdateSettings(Dt::IComponent* _pComponent)
    {
        assert(_pComponent != nullptr);

        auto pCameraComponent = static_cast<Dt::CCameraComponent*>(_pComponent);

        Gfx::CCamera& rCamera = *Gfx::ViewManager::GetMainCamera();

        // -----------------------------------------------------------------------------
        // Projection
        // -----------------------------------------------------------------------------
        if (pCameraComponent->GetProjectionType() == Dt::CCameraComponent::Perspective)
        {
            rCamera.SetFieldOfView(pCameraComponent->GetFoV(), rCamera.GetAspectRatio(), pCameraComponent->GetNear(), pCameraComponent->GetFar());
        }
        else if (pCameraComponent->GetProjectionType() == Dt::CCameraComponent::Orthographic)
        {
            float Left   = -pCameraComponent->GetSize() / 2.0f;
            float Right  =  pCameraComponent->GetSize() / 2.0f;
            float Bottom = -pCameraComponent->GetSize() / 2.0f;
            float Top    =  pCameraComponent->GetSize() / 2.0f;

            rCamera.SetOrthographic(Left, Right, Bottom, Top, pCameraComponent->GetNear(), pCameraComponent->GetFar());
        }
        else if (pCameraComponent->GetProjectionType() == Dt::CCameraComponent::External)
        {
            glm::mat4& rProjectionMatrix = pCameraComponent->GetProjectionMatrix();

            // -----------------------------------------------------------------------------
            // Decompose left, right, top, bottom, near and far from projection
            // matrix:
            // -----------------------------------------------------------------------------
            float Near = rProjectionMatrix[2][3] / (rProjectionMatrix[2][2] - 1);
            float Far  = rProjectionMatrix[2][3] / (rProjectionMatrix[2][2] + 1);

            float Bottom = Near * (rProjectionMatrix[2][1] - 1.0f) / rProjectionMatrix[1][1];
            float Top    = Near * (rProjectionMatrix[2][1] + 1.0f) / rProjectionMatrix[1][1];
            float Left   = Near * (rProjectionMatrix[2][0] - 1.0f) / rProjectionMatrix[0][0];
            float Right  = Near * (rProjectionMatrix[2][0] + 1.0f) / rProjectionMatrix[0][0];

            rCamera.SetPerspective(Left, Right, Bottom, Top, Near, Far);
        }

        // -----------------------------------------------------------------------------
        // Camera mode + variables
        // -----------------------------------------------------------------------------
        rCamera.SetCameraMode(Gfx::CCamera::Auto);

        if (pCameraComponent->GetCameraMode() == Dt::CCameraComponent::Manual)
        {
            rCamera.SetCameraMode(Gfx::CCamera::Manual);

            rCamera.SetShutterSpeed(pCameraComponent->GetShutterSpeed());

            rCamera.SetAperture(pCameraComponent->GetAperture());

            rCamera.SetISO(pCameraComponent->GetISO());

            rCamera.SetEC(pCameraComponent->GetEC());
        }

        // -----------------------------------------------------------------------------
        // Other
        // -----------------------------------------------------------------------------
        rCamera.SetBackgroundColor(pCameraComponent->GetBackgroundColor());

        rCamera.SetCullingMask(pCameraComponent->GetCullingMask());

        rCamera.SetViewportRect(pCameraComponent->GetViewportRect());

        rCamera.SetDepth(pCameraComponent->GetDepth());
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
                Dt::CEntity* pNewEntity = Dt::EntityManager::GetEntityByID(pCameraComponent->GetHostEntity()->GetID());

                if (pNewEntity != nullptr && pNewEntity->IsInMap())
                {
                    m_pRelatedEntity = pNewEntity;

                    UpdateTransformation(m_pRelatedEntity);

                    UpdateSettings(pCameraComponent);
                }

                break;
            }
        }
    }
} // namespace Cam