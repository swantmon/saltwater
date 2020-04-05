
#pragma once

#include "base/base_coordinate_system.h"
#include "base/base_include_glm.h"

#include "engine/core/core_plugin_manager.h"

#include "engine/data/data_camera_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/script/script_script.h"
#include "script_ar_settings_script.h"

namespace Scpt
{
    class CARPlaceObjectOnTouchScript : public CScript<CARPlaceObjectOnTouchScript>
    {
    public:

        using ArCoreAcquireNewMarkerFunc = const void* (*)(float _X, float _Y);
        using ArCoreReleaseMarkerFunc = const void (*)(const void* _pMarker);
        using ArCoreGetMarkerTrackingStateFunc = int (*)(const void* _pMarker);
        using ArCoreGetMarkerModelMatrixFunc = glm::mat4 (*)(const void* _pMarker);

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

            m_ArCoreAvailable = Core::PluginManager::LoadPlugin("ArCore");

            if (m_ArCoreAvailable)
            {
                AcquireNewMarker = (ArCoreAcquireNewMarkerFunc)(Core::PluginManager::GetPluginFunction("ArCore", "AcquireNewMarker"));
                ReleaseMarker = (ArCoreReleaseMarkerFunc)(Core::PluginManager::GetPluginFunction("ArCore", "ReleaseMarker"));

                GetMarkerTrackingState = (ArCoreGetMarkerTrackingStateFunc)(Core::PluginManager::GetPluginFunction("ArCore", "GetMarkerTrackingState"));
                GetMarkerModelMatrix = (ArCoreGetMarkerModelMatrixFunc)(Core::PluginManager::GetPluginFunction("ArCore", "GetMarkerModelMatrix"));
            }
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

            //pTransformation->SetRotation(glm::eulerAngles(glm::toQuat(glm::mat3(ModelMatrix))));

            Dt::CEntityManager::GetInstance().MarkEntityAsDirty(*m_pEntity, Dt::CEntity::DirtyMove);
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            if (!m_ArCoreAvailable) return;

            if (_rEvent.GetAction() == Base::CInputEvent::TouchPressed)
            {
                auto x = (float)_rEvent.GetGlobalCursorPosition()[0];
                auto y = (float)_rEvent.GetGlobalCursorPosition()[1];

                const void* pNewMarker = AcquireNewMarker(x, y);

                if (pNewMarker != nullptr && m_pMarker != nullptr)
                {
                    ReleaseMarker(m_pMarker);
                }

                if (pNewMarker != nullptr)
                {
                    m_pMarker = pNewMarker;

                    auto ScriptComponents = Dt::CComponentManager::GetInstance().GetComponents<Dt::CScriptComponent>();

                    for (auto ScriptComponent : ScriptComponents)
                    {
                        auto pScriptComponent = static_cast<Dt::CScriptComponent*>(ScriptComponent);

                        if (pScriptComponent->GetScriptTypeInfo() == Base::CTypeInfo::Get<Scpt::CARSettingsScript>())
                        {
                            auto pARSettingsScript = static_cast<Scpt::CARSettingsScript*>(pScriptComponent);

                            if (pARSettingsScript->m_HidePlanesAndPointsOnFirstMarker)
                            {
                                pARSettingsScript->m_RenderPlanes = false;
                                pARSettingsScript->m_RenderPoints = false;

                                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pARSettingsScript, Dt::IComponent::DirtyInfo);
                            }

                            break;
                        }
                    }
                }
            }
        }

    public:

        inline void Read(CSceneReader& _rCodec) override
        {
            CComponent::Read(_rCodec);
        }

        inline void Write(CSceneWriter& _rCodec) override
        {
            CComponent::Write(_rCodec);
        }

        inline IComponent* Allocate() override
        {
            return new CARPlaceObjectOnTouchScript();
        }
    };
} // namespace Scpt