
#include "app_droid/app_precompiled.h"

#include "app_droid/app_application.h"
#include "app_droid/app_load_map_state.h"

#include "base/base_include_glm.h"

#include "engine/core/core_asset_manager.h"

#include "engine/data/data_camera_component.h"
#include "engine/data/data_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_hierarchy_facet.h"
#include "engine/data/data_light_probe_component.h"
#include "engine/data/data_map.h"
#include "engine/data/data_material.h"
#include "engine/data/data_material_component.h"
#include "engine/data/data_material_manager.h"
#include "engine/data/data_mesh_component.h"
#include "engine/data/data_script_component.h"
#include "engine/data/data_sky_component.h"
#include "engine/data/data_ssao_component.h"
#include "engine/data/data_sun_component.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/script/script_ar_camera_control_script.h"
#include "engine/script/script_ar_place_object_on_touch_script.h"
#include "engine/script/script_light_estimation.h"
#include "engine/script/script_script_manager.h"

#define USE_HEAD_MODEL 0

namespace App
{
    void CreateDefaultScene();
} // namespace App

namespace App
{
    CLoadMapState& CLoadMapState::GetInstance()
    {
        static CLoadMapState s_Singleton;

        return s_Singleton;
    }
} // namespace App

namespace Base
{
    class CTextReader
    {
    };
} // namespace Base

namespace App
{
    CLoadMapState::CLoadMapState()
    {

    }

    // -----------------------------------------------------------------------------

    CLoadMapState::~CLoadMapState()
    {

    }

    // -----------------------------------------------------------------------------

    void CLoadMapState::InternOnEnter()
    {
        CreateDefaultScene();
    }

    // -----------------------------------------------------------------------------

    void CLoadMapState::InternOnLeave()
    {
    }

    // -----------------------------------------------------------------------------

    void CLoadMapState::InternOnRun()
    {
        App::Application::ChangeState(CState::Play);
    }
} // namespace App

namespace App
{
    void CreateDefaultScene()
    {
        // -----------------------------------------------------------------------------
        // Allocate a map
        // -----------------------------------------------------------------------------
        Dt::Map::AllocateMap(1, 1);

        // -----------------------------------------------------------------------------
        // Setup cameras
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Main Camera");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

            auto Component = Dt::CComponentManager::GetInstance().Allocate<Dt::CCameraComponent>();

            Component->SetProjectionType(Dt::CCameraComponent::Perspective);
            Component->SetClearFlag(Dt::CCameraComponent::Skybox);

            rEntity.AttachComponent(Component);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CCameraComponent::DirtyCreate);

            auto ScriptComponent = Dt::CComponentManager::GetInstance().Allocate<Scpt::CARCameraControlScript>();

            rEntity.AttachComponent(ScriptComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*ScriptComponent, Dt::CScriptComponent::DirtyCreate);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup light
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEnvironmentEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEnvironmentEntity.SetName("Environment");

            // -----------------------------------------------------------------------------
            // Transformation
            // -----------------------------------------------------------------------------
            Dt::CTransformationFacet* pTransformationFacet = rEnvironmentEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 20.0f));
            pTransformationFacet->SetScale   (glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            {
                auto SunComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CSunComponent>();

                SunComponent->EnableTemperature(false);
                SunComponent->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));

                SunComponent->SetDirection(glm::vec3(0.0f, 0.01f, -1.0f));
                SunComponent->SetIntensity(90600.0f);
                SunComponent->SetTemperature(0);
                SunComponent->SetRefreshMode(Dt::CSunComponent::Dynamic);

                SunComponent->UpdateLightness();

                rEnvironmentEntity.AttachComponent(SunComponent);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*SunComponent, Dt::CSunComponent::DirtyCreate);
            }

            {
                auto Component = Dt::CComponentManager::GetInstance().Allocate<Dt::CSkyComponent>();

                Component->SetRefreshMode(Dt::CSkyComponent::Static);
                Component->SetType(Dt::CSkyComponent::Procedural);
                Component->SetIntensity(10000.0f);
                Component->SetQuality(Dt::CSkyComponent::PX256);

                rEnvironmentEntity.AttachComponent(Component);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CSkyComponent::DirtyCreate);
            }

            {
                auto Component = Dt::CComponentManager::GetInstance().Allocate<Scpt::CLightEstimationScript>();

                Component->m_EstimationType = Scpt::CLightEstimationScript::LUT;

                rEnvironmentEntity.AttachComponent(Component);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CSkyComponent::DirtyCreate);
            }

            Dt::EntityManager::MarkEntityAsDirty(rEnvironmentEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rLightingEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rLightingEntity.SetName("Local light probe");

            Dt::CTransformationFacet* pTransformationFacet = rLightingEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            {
                auto LightProbeComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CLightProbeComponent>();

                LightProbeComponent->SetType(Dt::CLightProbeComponent::Sky);
                LightProbeComponent->SetQuality(Dt::CLightProbeComponent::PX128);
                LightProbeComponent->SetIntensity(1.0f);
                LightProbeComponent->SetRefreshMode(Dt::CLightProbeComponent::Dynamic);
                LightProbeComponent->SetNear(0.01f);
                LightProbeComponent->SetFar(1024.0f);
                LightProbeComponent->SetParallaxCorrection(false);
                LightProbeComponent->SetBoxSize(glm::vec3(1024.0f));

                rLightingEntity.AttachComponent(LightProbeComponent);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*LightProbeComponent, Dt::CLightProbeComponent::DirtyCreate);
            }

            Dt::EntityManager::MarkEntityAsDirty(rLightingEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup entities
        // -----------------------------------------------------------------------------
        Dt::SEntityDescriptor EntityDesc;

        EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
        EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

        Dt::CEntity& rRootEntity = Dt::EntityManager::CreateEntity(EntityDesc);

        rRootEntity.SetName("Root");

        Dt::CTransformationFacet* pTransformationFacet = rRootEntity.GetTransformationFacet();

        pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        pTransformationFacet->SetScale(glm::vec3(1.0f));
        pTransformationFacet->SetRotation(glm::vec3(0.0f));

        // -----------------------------------------------------------------------------

        auto pScriptComponent = Dt::CComponentManager::GetInstance().Allocate<Scpt::CARPlaceObjectOnTouchScript>();

        rRootEntity.AttachComponent(pScriptComponent);

        Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pScriptComponent, Dt::CScriptComponent::DirtyCreate);

        // -----------------------------------------------------------------------------

        Dt::EntityManager::MarkEntityAsDirty(rRootEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Object");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

#if USE_HEAD_MODEL == 1
            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.75f));
            pTransformationFacet->SetScale(glm::vec3(2.0f));
            pTransformationFacet->SetRotation(glm::vec3(glm::radians(-90.0f), 0.0f, 0.0f));
#else
            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 1.0f));
            pTransformationFacet->SetScale(glm::vec3(0.50f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));
#endif

            // -----------------------------------------------------------------------------

            auto pMeshComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMeshComponent>();

#if USE_HEAD_MODEL == 1
            pMeshComponent->SetFilename(Core::AssetManager::GetPathToAssets() + "/models/head.dae");
#else
            pMeshComponent->SetMeshType(Dt::CMeshComponent::Box);
#endif

            rEntity.AttachComponent(pMeshComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMeshComponent, Dt::CMeshComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            auto pMaterial = Dt::MaterialManager::CreateMaterialFromName("Chrome Sparrow");

            pMaterial->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
            pMaterial->SetMetalness(1.0f);
            pMaterial->SetRoughness(0.15f);

            auto pMaterialComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();

            pMaterialComponent->SetMaterial(pMaterial);

            rEntity.AttachComponent(pMaterialComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMaterialComponent, Dt::CMaterialComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);

            rRootEntity.Attach(rEntity);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Plane");
            rEntity.SetLayer(Dt::SEntityLayer::AR);

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            pTransformationFacet->SetScale(glm::vec3(1000.0f, 1000.0f, 0.0001f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            // -----------------------------------------------------------------------------

            auto pMeshComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMeshComponent>();

            pMeshComponent->SetMeshType(Dt::CMeshComponent::Box);

            rEntity.AttachComponent(pMeshComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMeshComponent, Dt::CMeshComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            auto pMaterial = Dt::MaterialManager::CreateMaterialFromName("Plane");

            pMaterial->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
            pMaterial->SetMetalness(0.0f);
            pMaterial->SetRoughness(1.0f);

            auto pMaterialComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();

            pMaterialComponent->SetMaterial(pMaterial);

            rEntity.AttachComponent(pMaterialComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMaterialComponent, Dt::CMaterialComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);

            rRootEntity.Attach(rEntity);
        }
    }
} //namespace App