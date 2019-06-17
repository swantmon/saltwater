
#include "app_droid/app_precompiled.h"

#include "app_droid/app_application.h"
#include "app_droid/app_load_map_state.h"

#include "base/base_include_glm.h"
#include "base/base_serialize_text_reader.h"

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
#include "engine/data/data_material_component.h"
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

#define USE_SCENE 1
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
#if USE_SCENE
        // -----------------------------------------------------------------------------
        // Get filename
        // -----------------------------------------------------------------------------
        std::string Filename = Core::CProgramParameters::GetInstance().Get("application:last_scene", "Default Scene.sws");

        // -----------------------------------------------------------------------------
        // Load
        // -----------------------------------------------------------------------------
        std::ifstream iStream;

        iStream.open(Core::AssetManager::GetPathToAssets() + "/" + Filename);

        if (iStream.is_open())
        {
            Base::CTextReader Reader(iStream, 1);

            Dt::CComponentManager::GetInstance().Read(Reader);

            Dt::Map::Read(Reader);

            Dt::CEntityManager::GetInstance().Read(Reader);

            iStream.close();
        }
        else
        {
            CreateDefaultScene();
        }
#else
        CreateDefaultScene();
#endif
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

            Dt::CEntity& rEntity = Dt::CEntityManager::GetInstance().CreateEntity(EntityDesc);

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

            Dt::CEntityManager::GetInstance().MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup light
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEnvironmentEntity = Dt::CEntityManager::GetInstance().CreateEntity(EntityDesc);

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
                Component->SetQuality(Dt::CSkyComponent::PX128);

                rEnvironmentEntity.AttachComponent(Component);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CSkyComponent::DirtyCreate);
            }

            {
                auto Component = Dt::CComponentManager::GetInstance().Allocate<Scpt::CLightEstimationScript>();

                rEnvironmentEntity.AttachComponent(Component);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CSkyComponent::DirtyCreate);
            }

            Dt::CEntityManager::GetInstance().MarkEntityAsDirty(rEnvironmentEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rLightingEntity = Dt::CEntityManager::GetInstance().CreateEntity(EntityDesc);

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

            Dt::CEntityManager::GetInstance().MarkEntityAsDirty(rLightingEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup entities
        // -----------------------------------------------------------------------------
        Dt::SEntityDescriptor EntityDesc;

        EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
        EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

        Dt::CEntity& rRootEntity = Dt::CEntityManager::GetInstance().CreateEntity(EntityDesc);

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

        Dt::CEntityManager::GetInstance().MarkEntityAsDirty(rRootEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::CEntityManager::GetInstance().CreateEntity(EntityDesc);

            rEntity.SetName("Object");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

#if USE_HEAD_MODEL == 1
            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 1.0f));
            pTransformationFacet->SetScale(glm::vec3(2.0f));
            pTransformationFacet->SetRotation(glm::vec3(glm::radians(-90.0f), 0.0f, 0.0f));
#else
            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 1.0f));
            pTransformationFacet->SetScale(glm::vec3(0.40f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));
#endif

            // -----------------------------------------------------------------------------

            auto pMeshComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMeshComponent>();

#if USE_HEAD_MODEL == 1
            pMeshComponent->SetFilename("/models/head.dae");
#else
            pMeshComponent->SetMeshType(Dt::CMeshComponent::Sphere);
#endif

            rEntity.AttachComponent(pMeshComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMeshComponent, Dt::CMeshComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            auto pMaterialComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();

            pMaterialComponent->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            pMaterialComponent->SetMetalness(1.0f);
            pMaterialComponent->SetRoughness(0.25f);

            rEntity.AttachComponent(pMaterialComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMaterialComponent, Dt::CMaterialComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::CEntityManager::GetInstance().MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);

            rRootEntity.Attach(rEntity);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::CEntityManager::GetInstance().CreateEntity(EntityDesc);

            rEntity.SetName("Plane");
            rEntity.SetLayer(Dt::SEntityLayer::ShadowOnly);

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            pTransformationFacet->SetScale(glm::vec3(100000.0f, 100000.0f, 0.0001f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            // -----------------------------------------------------------------------------

            auto pMeshComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMeshComponent>();

            pMeshComponent->SetMeshType(Dt::CMeshComponent::Box);

            rEntity.AttachComponent(pMeshComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMeshComponent, Dt::CMeshComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            auto pMaterialComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();

            pMaterialComponent->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            pMaterialComponent->SetMetalness(0.0f);
            pMaterialComponent->SetRoughness(1.0f);

            rEntity.AttachComponent(pMaterialComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMaterialComponent, Dt::CMaterialComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::CEntityManager::GetInstance().MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);

            rRootEntity.Attach(rEntity);
        }
    }
} //namespace App