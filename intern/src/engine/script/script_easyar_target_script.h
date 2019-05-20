
#pragma once

#include "base/base_coordinate_system.h"
#include "base/base_include_glm.h"

#include "engine/data/data_camera_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/script/script_script.h"

namespace Scpt
{
    class CEasyARTargetScript : public CScript<CEasyARTargetScript>
    {
    public:

        typedef const void* (*AcquireNewTargetFunc)(const std::string& _rPathToFile);
        typedef const void(*ReleaseTargetFunc)(const void* _pTarget);
        typedef int(*GetTargetTrackingStateFunc)(const void* _pTarget);
        typedef glm::mat4(*GetTargetModelMatrixFunc)(const void* _pTarget);

        AcquireNewTargetFunc AcquireNewTarget;
        ReleaseTargetFunc ReleaseTarget;
        GetTargetTrackingStateFunc GetTargetTrackingState;
        GetTargetModelMatrixFunc GetTargetModelMatrix;

    public:

        std::string m_TargetFile = "";

    private:

        glm::mat3 m_MRToEngineMatrix = Base::CCoordinateSystem::GetBaseMatrix(glm::vec3(-1, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
        Dt::CEntity* m_pEntity = nullptr;
        const void* m_pTarget;
        bool m_IsPluginAvailable = false;

    public:

        void Start() override
        {
            m_pEntity = GetEntity();

            m_MRToEngineMatrix = Base::CCoordinateSystem::GetBaseMatrix(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1));

            m_IsPluginAvailable = Core::PluginManager::LoadPlugin("EasyAR");

            AcquireNewTarget = (AcquireNewTargetFunc)(Core::PluginManager::GetPluginFunction("EasyAR", "AcquireNewTarget"));
            ReleaseTarget = (ReleaseTargetFunc)(Core::PluginManager::GetPluginFunction("EasyAR", "ReleaseTarget"));

            GetTargetTrackingState = (GetTargetTrackingStateFunc)(Core::PluginManager::GetPluginFunction("EasyAR", "GetTargetTrackingState"));
            GetTargetModelMatrix = (GetTargetModelMatrixFunc)(Core::PluginManager::GetPluginFunction("EasyAR", "GetTargetModelMatrix"));

            if (m_IsPluginAvailable)
            {
                m_pTarget = AcquireNewTarget(m_TargetFile);
            }
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {

        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            if (m_pEntity == nullptr || m_pTarget == nullptr || !m_IsPluginAvailable) return;

            if (GetTargetTrackingState(m_pTarget) != 2) return;

            glm::mat4 ModelMatrix = GetTargetModelMatrix(m_pTarget);

            Dt::CTransformationFacet* pTransformation = m_pEntity->GetTransformationFacet();

            pTransformation->SetPosition(glm::mat4(m_MRToEngineMatrix) * ModelMatrix[3]);

            pTransformation->SetRotation(glm::eulerAngles(glm::toQuat(glm::mat3(ModelMatrix))));

            Dt::CEntityManager::GetInstance().MarkEntityAsDirty(*m_pEntity, Dt::CEntity::DirtyMove);
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            BASE_UNUSED(_rEvent);
        }
    };
} // namespace Scpt