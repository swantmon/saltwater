
#pragma once

#include "base/base_coordinate_system.h"
#include "base/base_include_glm.h"

#include "data/data_camera_component.h"
#include "data/data_component_facet.h"
#include "data/data_transformation_facet.h"

#include "script/script_script.h"

namespace Scpt
{
    class CARPlaceObjectOnTouchScript : public CScript<CARPlaceObjectOnTouchScript>
    {
    public:

        typedef const void* (*ArCoreAcquireNewMarkerFunc)(float _X, float _Y);
        typedef const void (*ArCoreReleaseMarkerFunc)(const void* _pMarker);
        typedef int (*ArCoreGetMarkerTrackingStateFunc)(const void* _pMarker);
        typedef glm::mat4 (*ArCoreGetMarkerModelMatrixFunc)(const void* _pMarker);

        ArCoreAcquireNewMarkerFunc AcquireNewMarker;
        ArCoreReleaseMarkerFunc ReleaseMarker;
        ArCoreGetMarkerTrackingStateFunc GetMarkerTrackingState;
        ArCoreGetMarkerModelMatrixFunc GetMarkerModelMatrix;

    public:

        Dt::CEntity* m_pEntity = nullptr;

    private:

        glm::mat3 m_MRToEngineMatrix = glm::mat3(1.0f);
        const void* m_pMarker;
        bool m_ArCoreAvailable = false;

    public:

        void Start() override
        {
            m_pEntity = GetEntity();

            m_MRToEngineMatrix = Base::CCoordinateSystem::GetBaseMatrix(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1));

            m_ArCoreAvailable = Core::PluginManager::HasPlugin("ArCore");

            AcquireNewMarker = (ArCoreAcquireNewMarkerFunc)(Core::PluginManager::GetPluginFunction("ArCore", "AcquireNewMarker"));
            ReleaseMarker = (ArCoreReleaseMarkerFunc)(Core::PluginManager::GetPluginFunction("ArCore", "ReleaseMarker"));

            GetMarkerTrackingState = (ArCoreGetMarkerTrackingStateFunc)(Core::PluginManager::GetPluginFunction("ArCore", "GetMarkerTrackingState"));
            GetMarkerModelMatrix = (ArCoreGetMarkerModelMatrixFunc)(Core::PluginManager::GetPluginFunction("ArCore", "GetMarkerModelMatrix"));
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {

        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            if (m_pEntity == nullptr || m_pMarker == nullptr || !m_ArCoreAvailable) return;

            if (GetMarkerTrackingState(m_pMarker) != 2) return;

            glm::mat4 ModelMatrix = GetMarkerModelMatrix(m_pMarker);

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

                const void* pNewMarker = AcquireNewMarker(x, y);

                if (pNewMarker != nullptr && m_pMarker != nullptr)
                {
                    ReleaseMarker(m_pMarker);
                }

                if (pNewMarker != nullptr)
                {
                    m_pMarker = pNewMarker;
                }
            }
        }
    };
} // namespace Scpt