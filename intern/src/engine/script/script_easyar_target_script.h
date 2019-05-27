
#pragma once

#include "base/base_coordinate_system.h"
#include "base/base_include_glm.h"

#include "engine/core/core_plugin_manager.h"

#include "engine/data/data_camera_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/script/script_script.h"

namespace Scpt
{
    class CEasyARTargetScript : public CScript<CEasyARTargetScript>
    {
    public:

        using AcquireNewTargetFunc = const void* (*)(const std::string& _rPathToFile);
        using ReleaseTargetFunc = const void(*)(const void* _pTarget);
        using GetTargetTrackingStateFunc = int(*)(const void* _pTarget);
        using GetTargetModelMatrixFunc = glm::mat4(*)(const void* _pTarget);

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

    public:

        inline void Read(Base::CTextReader& _rCodec) override
        {
            CComponent::Read(_rCodec);

            Base::Serialize(_rCodec, m_TargetFile);
        }

        inline void Write(Base::CTextWriter& _rCodec) override
        {
            CComponent::Write(_rCodec);

            Base::Serialize(_rCodec, m_TargetFile);
        }

        inline IComponent* Allocate() override
        {
            return new CEasyARTargetScript();
        }
    };
} // namespace Scpt