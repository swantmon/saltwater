
#pragma once

#include "base/base_coordinate_system.h"
#include "base/base_include_glm.h"

#include "data/data_camera_component.h"
#include "data/data_component_facet.h"
#include "data/data_transformation_facet.h"

#include "mr/mr_camera_manager.h"

#include "script/script_script.h"

namespace Scpt
{
    class CARCameraControlScript : public CScript<CARCameraControlScript>
    {
    public:

        Dt::CEntity* m_pCameraEntity = nullptr;
        Dt::CCameraComponent* m_pCameraComponent = nullptr;

    private:

        glm::mat3 m_MRToEngineMatrix = glm::mat3(1.0f);

    public:

        void Start() override
        {
            m_pCameraEntity = GetEntity();

            if (m_pCameraEntity != nullptr)
            {
                m_pCameraComponent = m_pCameraEntity->GetComponentFacet()->GetComponent<Dt::CCameraComponent>();
            }

            m_MRToEngineMatrix = Base::CCoordinateSystem::GetBaseMatrix(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1));
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {

        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            const MR::CCamera& rCamera = MR::CameraManager::GetCamera();

            if (rCamera.GetTackingState() != MR::CCamera::Tracking) return;

            if (m_pCameraEntity != nullptr)
            {
                glm::mat3 WSRotation = m_MRToEngineMatrix * glm::transpose(glm::mat3(rCamera.GetViewMatrix()));

                glm::vec3 WSPosition = WSRotation * rCamera.GetViewMatrix()[3] * -1.0f;

                m_pCameraEntity->SetWorldPosition(WSPosition);

                m_pCameraEntity->GetTransformationFacet()->SetPosition(WSPosition);

                m_pCameraEntity->GetTransformationFacet()->SetScale(glm::vec3(1.0f));

                m_pCameraEntity->GetTransformationFacet()->SetRotation(glm::toQuat(WSRotation));

                Dt::EntityManager::MarkEntityAsDirty(*m_pCameraEntity, Dt::CEntity::DirtyMove);
            }

            if (m_pCameraComponent != nullptr)
            {
                m_pCameraComponent->SetNear(rCamera.GetNear());

                m_pCameraComponent->SetNear(rCamera.GetFar());

                m_pCameraComponent->SetProjectionMatrix(rCamera.GetProjectionMatrix());

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*m_pCameraComponent, Dt::CCameraComponent::DirtyInfo);
            }
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            BASE_UNUSED(_rEvent);
        }
    };
} // namespace Scpt