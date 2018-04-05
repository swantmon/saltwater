
#pragma once

#include "base/base_coordinate_system.h"
#include "base/base_include_glm.h"

#include "data/data_camera_component.h"
#include "data/data_component_facet.h"
#include "data/data_transformation_facet.h"

#include "plugin_arcore/mr_control_manager.h"

#include "script/script_script.h"

namespace Scpt
{
    class CARPlaceObjectOnTouchScript : public CScript<CARPlaceObjectOnTouchScript>
    {
    public:

        Dt::CEntity* m_pEntity = nullptr;

    private:

        glm::mat3 m_MRToEngineMatrix = glm::mat3(1.0f);
        const MR::CMarker* m_pMarker;

    public:

        void Start() override
        {
            m_pEntity = GetEntity();

            m_MRToEngineMatrix = Base::CCoordinateSystem::GetBaseMatrix(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1));
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {

        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            if (m_pEntity == nullptr || m_pMarker == nullptr) return;

            if (m_pMarker->GetTackingState() != MR::CCamera::Tracking) return;

            glm::mat4 ModelMatrix = m_pMarker->GetModelMatrix();

            Dt::CTransformationFacet* pTransformation = m_pEntity->GetTransformationFacet();

            pTransformation->SetPosition(glm::mat4(m_MRToEngineMatrix) * ModelMatrix[3]);

            pTransformation->SetRotation(glm::eulerAngles(glm::toQuat(glm::mat3(ModelMatrix))));

            Dt::EntityManager::MarkEntityAsDirty(*m_pEntity, Dt::CEntity::DirtyMove);
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            if (_rEvent.GetAction() == Base::CInputEvent::TouchPressed)
            {
                float x = _rEvent.GetCursorPosition()[0];
                float y = _rEvent.GetCursorPosition()[1];

                const MR::CMarker* pNewMarker = MR::ControlManager::AcquireNewMarker(x, y);

                if (pNewMarker != nullptr && m_pMarker != nullptr)
                {
                    MR::ControlManager::ReleaseMarker(m_pMarker);
                }

                if (pNewMarker != nullptr)
                {
                    m_pMarker = pNewMarker;
                }
            }
        }
    };
} // namespace Scpt