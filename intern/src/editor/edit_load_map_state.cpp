
#include "editor/edit_precompiled.h"

#include "base/base_serialize_text_reader.h"

#include "editor/edit_load_map_state.h"

#include "engine/core/core_asset_manager.h"

#include "engine/camera/cam_control_manager.h"

#include "engine/core/core_program_parameters.h"

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
#include "engine/data/data_point_light_component.h"
#include "engine/data/data_post_aa_component.h"
#include "engine/data/data_script_component.h"
#include "engine/data/data_sky_component.h"
#include "engine/data/data_ssao_component.h"
#include "engine/data/data_sun_component.h"
#include "engine/data/data_transformation_facet.h"

#include "engine/script/script_camera_control_script.h"
#include "engine/script/script_ar_camera_control_script.h"
#include "engine/script/script_easyar_target_script.h"
#include "engine/script/script_light_estimation.h"
#include "engine/script/script_slam.h"
#include "engine/script/script_script_manager.h"

#include <assert.h>
#include <fstream>
#include <string>

namespace Edit
{
    CLoadMapState& CLoadMapState::GetInstance()
    {
        static CLoadMapState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Edit

namespace Edit
{
    void CreateEmptyScene();
    void CreateDefaultScene();
    void CreateDefaultARScene();
    void CreateCornellBoxScene();
    void CreateSLAMScene();
} // namespace Edit

namespace Edit
{
    CLoadMapState::CLoadMapState()
        : CState     (LoadMap)
        , m_Filename ("")
    {
        m_NextState = CState::Edit;
    }
    
    // -----------------------------------------------------------------------------
    
    CLoadMapState::~CLoadMapState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CLoadMapState::LoadFromFile(const std::string& _rFilename)
    {
        m_Filename = _rFilename;
    }

    // -----------------------------------------------------------------------------

    const std::string& CLoadMapState::GetFilename() const
    {
        return m_Filename;
    }
    
    // -----------------------------------------------------------------------------
    
    void CLoadMapState::InternOnEnter()
    {
        auto Scene = Core::CProgramParameters::GetInstance().Get("application:load_scene", "default");

        ENGINE_CONSOLE_INFOV("Loading scene '%s'", Scene.c_str());

        if (Scene == "empty")
        {
            CreateEmptyScene();
        }
        else if (Scene == "default")
        {
            CreateDefaultScene();
        }
        else if (Scene == "default_ar")
        {
            CreateDefaultARScene();
        }
        else if (Scene == "cornell_box")
        {
            CreateCornellBoxScene();
        }
        else if (Scene == "slam")
        {
            CreateSLAMScene();
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CLoadMapState::InternOnLeave()
    {
        m_NextState = LoadMap;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CLoadMapState::InternOnRun()
    {
        return m_NextState;
    }
} // namespace Edit

namespace Edit
{
    void CreateEmptyScene()
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
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Main Camera");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

            auto Component = Dt::CComponentManager::GetInstance().Allocate<Dt::CCameraComponent>();

            rEntity.AttachComponent(Component);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CCameraComponent::DirtyCreate);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

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
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Main Camera");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

            auto Component = Dt::CComponentManager::GetInstance().Allocate<Dt::CCameraComponent>();

            Component->SetProjectionType(Dt::CCameraComponent::Perspective);
            Component->SetClearFlag(Dt::CCameraComponent::Skybox);

            rEntity.AttachComponent(Component);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CCameraComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            auto ScriptComponent = Dt::CComponentManager::GetInstance().Allocate<Scpt::CCameraControlScript>();

            rEntity.AttachComponent(ScriptComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*ScriptComponent, Dt::CScriptComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Effects
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("AA");

            {
                auto Component = Dt::CComponentManager::GetInstance().Allocate<Dt::CPostAAComponent>();

                Component->SetType(Dt::CPostAAComponent::SMAA);

                rEntity.AttachComponent(Component);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CLightProbeComponent::DirtyCreate);
            }

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup light
        // -----------------------------------------------------------------------------
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
                LightProbeComponent->SetQuality(Dt::CLightProbeComponent::PX256);
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

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEnvironmentEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEnvironmentEntity.SetName("Environment");

            // -----------------------------------------------------------------------------
            // Transformation
            // -----------------------------------------------------------------------------
            Dt::CTransformationFacet* pTransformationFacet = rEnvironmentEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 20.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
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
                Component->SetIntensity(100000.0f);

                rEnvironmentEntity.AttachComponent(Component);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CSkyComponent::DirtyCreate);
            }

            Dt::EntityManager::MarkEntityAsDirty(rEnvironmentEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup entities
        // -----------------------------------------------------------------------------
        Dt::SEntityDescriptor EntityDesc;

        EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
        EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

        Dt::CEntity& rRootEntity = Dt::EntityManager::CreateEntity(EntityDesc);

        rRootEntity.SetName("Root");

        Dt::CTransformationFacet* pTransformationFacet = rRootEntity.GetTransformationFacet();

        pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        pTransformationFacet->SetScale(glm::vec3(1.0f));
        pTransformationFacet->SetRotation(glm::vec3(0.0f));

        // -----------------------------------------------------------------------------

        Dt::EntityManager::MarkEntityAsDirty(rRootEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);

        {
            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Sphere");

            pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 2.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            // -----------------------------------------------------------------------------

            auto pMeshComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMeshComponent>();

            pMeshComponent->SetMeshType(Dt::CMeshComponent::Sphere);

            rEntity.AttachComponent(pMeshComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMeshComponent, Dt::CMeshComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            auto pMaterialComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();

            pMaterialComponent->SetMaterialname("Chrome Sparrow");
            pMaterialComponent->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            pMaterialComponent->SetMetalness(1.0f);
            pMaterialComponent->SetRoughness(0.25f);

            rEntity.AttachComponent(pMaterialComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMaterialComponent, Dt::CMaterialComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);

            rRootEntity.Attach(rEntity);
        }

        {
            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Plane");

            pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            pTransformationFacet->SetScale(glm::vec3(4.0f, 4.0f, 0.001f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            // -----------------------------------------------------------------------------

            auto pMeshComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMeshComponent>();

            pMeshComponent->SetMeshType(Dt::CMeshComponent::Box);

            rEntity.AttachComponent(pMeshComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMeshComponent, Dt::CMeshComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            auto pMaterialComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();

            pMaterialComponent->SetMaterialname("Plane");
            pMaterialComponent->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.8f));
            pMaterialComponent->SetMetalness(0.0f);
            pMaterialComponent->SetRoughness(1.0f);

            rEntity.AttachComponent(pMaterialComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMaterialComponent, Dt::CMaterialComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);

            rRootEntity.Attach(rEntity);
        }
    }

    // -----------------------------------------------------------------------------

    void CreateSLAMScene()
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
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Main Camera");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

            auto Component = Dt::CComponentManager::GetInstance().Allocate<Dt::CCameraComponent>();

            Component->SetProjectionType(Dt::CCameraComponent::Perspective);
            Component->SetClearFlag(Dt::CCameraComponent::Skybox);

            rEntity.AttachComponent(Component);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CCameraComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            auto ScriptComponent = Dt::CComponentManager::GetInstance().Allocate<Scpt::CCameraControlScript>();

            rEntity.AttachComponent(ScriptComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*ScriptComponent, Dt::CScriptComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Effects
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("AA");

            {
                auto Component = Dt::CComponentManager::GetInstance().Allocate<Dt::CPostAAComponent>();

                Component->SetType(Dt::CPostAAComponent::SMAA);

                rEntity.AttachComponent(Component);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CLightProbeComponent::DirtyCreate);
            }

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup light
        // -----------------------------------------------------------------------------
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
                LightProbeComponent->SetQuality(Dt::CLightProbeComponent::PX256);
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

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEnvironmentEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEnvironmentEntity.SetName("Environment");

            // -----------------------------------------------------------------------------
            // Transformation
            // -----------------------------------------------------------------------------
            Dt::CTransformationFacet* pTransformationFacet = rEnvironmentEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 20.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
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
                Component->SetIntensity(100000.0f);

                rEnvironmentEntity.AttachComponent(Component);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CSkyComponent::DirtyCreate);
            }

            Dt::EntityManager::MarkEntityAsDirty(rEnvironmentEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup entities
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("SLAM");

            // -----------------------------------------------------------------------------

            auto ScriptComponent = Dt::CComponentManager::GetInstance().Allocate<Scpt::CSLAMScript>();

            rEntity.AttachComponent(ScriptComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*ScriptComponent, Dt::CScriptComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

    void CreateDefaultARScene()
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
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Main Camera");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

            auto Component = Dt::CComponentManager::GetInstance().Allocate<Dt::CCameraComponent>();

            Component->SetProjectionType(Dt::CCameraComponent::Perspective);
            Component->SetClearFlag(Dt::CCameraComponent::Skybox);

            rEntity.AttachComponent(Component);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CCameraComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            auto ScriptComponent = Dt::CComponentManager::GetInstance().Allocate<Scpt::CARCameraControlScript>();

            rEntity.AttachComponent(ScriptComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*ScriptComponent, Dt::CScriptComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            auto ScriptComponent2 = Dt::CComponentManager::GetInstance().Allocate<Scpt::CEasyARTargetScript>();

            ScriptComponent2->m_TargetFile = "/../data/marker/world_center.png";

            rEntity.AttachComponent(ScriptComponent2);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*ScriptComponent2, Dt::CScriptComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup light
        // -----------------------------------------------------------------------------
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
                LightProbeComponent->SetQuality(Dt::CLightProbeComponent::PX256);
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

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEnvironmentEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEnvironmentEntity.SetName("Environment");

            // -----------------------------------------------------------------------------
            // Transformation
            // -----------------------------------------------------------------------------
            Dt::CTransformationFacet* pTransformationFacet = rEnvironmentEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 20.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
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

                rEnvironmentEntity.AttachComponent(Component);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CSkyComponent::DirtyCreate);
            }

            {
                auto Component = Dt::CComponentManager::GetInstance().Allocate<Scpt::CLightEstimationScript>();

                rEnvironmentEntity.AttachComponent(Component);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CSkyComponent::DirtyCreate);
            }

            Dt::EntityManager::MarkEntityAsDirty(rEnvironmentEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Effects
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("AA");

            {
                auto Component = Dt::CComponentManager::GetInstance().Allocate<Dt::CPostAAComponent>();

                Component->SetType(Dt::CPostAAComponent::SMAA);

                rEntity.AttachComponent(Component);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CLightProbeComponent::DirtyCreate);
            }

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup entities
        // -----------------------------------------------------------------------------
        Dt::SEntityDescriptor RootEntityDesc;

        RootEntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
        RootEntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

        Dt::CEntity& rRootEntity = Dt::EntityManager::CreateEntity(RootEntityDesc);

        rRootEntity.SetName("Root");

        Dt::CTransformationFacet* pTransformationFacet = rRootEntity.GetTransformationFacet();

        pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        pTransformationFacet->SetScale(glm::vec3(1.0f));
        pTransformationFacet->SetRotation(glm::vec3(0.0f));

        // -----------------------------------------------------------------------------

        {
            Dt::SEntityDescriptor SphereDesc;

            SphereDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            SphereDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(SphereDesc);

            rEntity.SetName("Sphere");

            Dt::CTransformationFacet* pSphereTransformationFacet = rEntity.GetTransformationFacet();

            pSphereTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 1.0f));
            pSphereTransformationFacet->SetScale(glm::vec3(0.50f));
            pSphereTransformationFacet->SetRotation(glm::vec3(0.0f));

            // -----------------------------------------------------------------------------

            auto pMeshComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMeshComponent>();

            pMeshComponent->SetMeshType(Dt::CMeshComponent::Sphere);

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

            rRootEntity.Attach(rEntity);
        }

        {
            Dt::SEntityDescriptor PlaneEntityDesc;

            PlaneEntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            PlaneEntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(PlaneEntityDesc);

            rEntity.SetName("Plane");
            rEntity.SetLayer(Dt::SEntityLayer::AR);

            Dt::CTransformationFacet* pPlaneTransformationFacet = rEntity.GetTransformationFacet();

            pPlaneTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            pPlaneTransformationFacet->SetScale(glm::vec3(1.0f, 1.0f, 0.001f));
            pPlaneTransformationFacet->SetRotation(glm::vec3(0.0f));

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

            rRootEntity.Attach(rEntity);
        }

        Dt::EntityManager::MarkEntityAsDirty(rRootEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);

    }

    // -----------------------------------------------------------------------------

    void CreateCornellBoxScene()
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
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Main Camera");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, -5.0f, 2.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));

            auto Component = Dt::CComponentManager::GetInstance().Allocate<Dt::CCameraComponent>();

            Component->SetProjectionType(Dt::CCameraComponent::Perspective);
            Component->SetClearFlag(Dt::CCameraComponent::Skybox);

            rEntity.AttachComponent(Component);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CCameraComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            auto ScriptComponent = Dt::CComponentManager::GetInstance().Allocate<Scpt::CCameraControlScript>();

            rEntity.AttachComponent(ScriptComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*ScriptComponent, Dt::CScriptComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);

            // -----------------------------------------------------------------------------

            Cam::ControlManager::GetActiveControl().SetPosition(pTransformationFacet->GetPosition());
            Cam::ControlManager::GetActiveControl().SetRotation(glm::toMat3(pTransformationFacet->GetRotation()));
        }

        // -----------------------------------------------------------------------------
        // Effects
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("AA");

            {
                auto Component = Dt::CComponentManager::GetInstance().Allocate<Dt::CPostAAComponent>();

                Component->SetType(Dt::CPostAAComponent::SMAA);

                rEntity.AttachComponent(Component);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::CLightProbeComponent::DirtyCreate);
            }

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup Cornell box
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;
        
            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);
        
            rEntity.SetName("Floor");
        
            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();
        
            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            pTransformationFacet->SetScale(glm::vec3(4.0f, 4.0f, 0.001f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));
        
            // -----------------------------------------------------------------------------
        
            auto pMeshComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMeshComponent>();
        
            pMeshComponent->SetMeshType(Dt::CMeshComponent::Box);
        
            rEntity.AttachComponent(pMeshComponent);
        
            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMeshComponent, Dt::CMeshComponent::DirtyCreate);
        
            // -----------------------------------------------------------------------------
        
            auto pMaterialComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();
        
            pMaterialComponent->SetMaterialname("Floor");
            pMaterialComponent->SetColor(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
            pMaterialComponent->SetMetalness(0.0f);
            pMaterialComponent->SetRoughness(1.0f);
        
            rEntity.AttachComponent(pMaterialComponent);
        
            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMaterialComponent, Dt::CMaterialComponent::DirtyCreate);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
        
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;
        
            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);
        
            rEntity.SetName("WallL");
        
            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();
        
            pTransformationFacet->SetPosition(glm::vec3(-2.0f, 0.0f, 2.0f));
            pTransformationFacet->SetScale(glm::vec3(0.001f, 4.0f, 4.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));
        
            // -----------------------------------------------------------------------------
        
            auto pMeshComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMeshComponent>();
        
            pMeshComponent->SetMeshType(Dt::CMeshComponent::Box);
        
            rEntity.AttachComponent(pMeshComponent);
        
            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMeshComponent, Dt::CMeshComponent::DirtyCreate);
        
            // -----------------------------------------------------------------------------
        
            auto pMaterialComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();
        
            pMaterialComponent->SetMaterialname("WallL");
            pMaterialComponent->SetColor(glm::vec4(0.8f, 0.0f, 0.0f, 1.0f));
            pMaterialComponent->SetMetalness(0.0f);
            pMaterialComponent->SetRoughness(1.0f);
        
            rEntity.AttachComponent(pMaterialComponent);
        
            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMaterialComponent, Dt::CMaterialComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("WallR");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(2.0f, 0.0f, 2.0f));
            pTransformationFacet->SetScale(glm::vec3(0.001f, 4.0f, 4.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            // -----------------------------------------------------------------------------

            auto pMeshComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMeshComponent>();

            pMeshComponent->SetMeshType(Dt::CMeshComponent::Box);

            rEntity.AttachComponent(pMeshComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMeshComponent, Dt::CMeshComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            auto pMaterialComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();

            pMaterialComponent->SetMaterialname("WallR");
            pMaterialComponent->SetColor(glm::vec4(0.0f, 0.0f, 0.8f, 1.0f));
            pMaterialComponent->SetMetalness(0.0f);
            pMaterialComponent->SetRoughness(1.0f);

            rEntity.AttachComponent(pMaterialComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMaterialComponent, Dt::CMaterialComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("WallB");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 2.0f, 2.0f));
            pTransformationFacet->SetScale(glm::vec3(4.0f, 0.001f, 4.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            // -----------------------------------------------------------------------------

            auto pMeshComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMeshComponent>();

            pMeshComponent->SetMeshType(Dt::CMeshComponent::Box);

            rEntity.AttachComponent(pMeshComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMeshComponent, Dt::CMeshComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            auto pMaterialComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();

            pMaterialComponent->SetMaterialname("WallB");
            pMaterialComponent->SetColor(glm::vec4(0.0f, 0.8f, 0.0f, 1.0f));
            pMaterialComponent->SetMetalness(0.0f);
            pMaterialComponent->SetRoughness(1.0f);

            rEntity.AttachComponent(pMaterialComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMaterialComponent, Dt::CMaterialComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Roof");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 4.0f));
            pTransformationFacet->SetScale(glm::vec3(4.0f, 4.0f, 0.001f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            // -----------------------------------------------------------------------------

            auto pMeshComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMeshComponent>();

            pMeshComponent->SetMeshType(Dt::CMeshComponent::Box);

            rEntity.AttachComponent(pMeshComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMeshComponent, Dt::CMeshComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            auto pMaterialComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();

            pMaterialComponent->SetMaterialname("Roof");
            pMaterialComponent->SetColor(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
            pMaterialComponent->SetMetalness(0.0f);
            pMaterialComponent->SetRoughness(1.0f);

            rEntity.AttachComponent(pMaterialComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMaterialComponent, Dt::CMaterialComponent::DirtyCreate);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }


        // -----------------------------------------------------------------------------
        // Object
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Sphere");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 1.0f));
            pTransformationFacet->SetScale(glm::vec3(2.5f));
            pTransformationFacet->SetRotation(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));

            // -----------------------------------------------------------------------------

            auto LightProbeComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CLightProbeComponent>();

            LightProbeComponent->SetActive(false);
            LightProbeComponent->SetType(Dt::CLightProbeComponent::Local);
            LightProbeComponent->SetQuality(Dt::CLightProbeComponent::PX256);
            LightProbeComponent->SetIntensity(1.0f);
            LightProbeComponent->SetRefreshMode(Dt::CLightProbeComponent::Dynamic);
            LightProbeComponent->SetNear(0.01f);
            LightProbeComponent->SetFar(1024.0f);
            LightProbeComponent->SetParallaxCorrection(true);
            LightProbeComponent->SetBoxSize(glm::vec3(1024.0f));

            rEntity.AttachComponent(LightProbeComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*LightProbeComponent, Dt::CLightProbeComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            auto pMeshComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMeshComponent>();

            pMeshComponent->SetMeshType(Dt::CMeshComponent::Asset);
            pMeshComponent->SetFilename("/models/bunny.dae");

            rEntity.AttachComponent(pMeshComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMeshComponent, Dt::CMeshComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            auto pMaterialComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();

            pMaterialComponent->SetMaterialname("Glass");
            pMaterialComponent->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            pMaterialComponent->SetMetalness(1.0f);
            pMaterialComponent->SetRoughness(0.25f);
            pMaterialComponent->SetRefractionIndex(1.46f);

            rEntity.AttachComponent(pMaterialComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMaterialComponent, Dt::CMaterialComponent::DirtyCreate);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup light
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Light Probe");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 1.5f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            // -----------------------------------------------------------------------------

            auto LightProbeComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CLightProbeComponent>();

            LightProbeComponent->SetType(Dt::CLightProbeComponent::Local);
            LightProbeComponent->SetQuality(Dt::CLightProbeComponent::PX256);
            LightProbeComponent->SetIntensity(1.0f);
            LightProbeComponent->SetRefreshMode(Dt::CLightProbeComponent::Dynamic);
            LightProbeComponent->SetNear(0.01f);
            LightProbeComponent->SetFar(1024.0f);
            LightProbeComponent->SetParallaxCorrection(false);
            LightProbeComponent->SetBoxSize(glm::vec3(1024.0f));

            rEntity.AttachComponent(LightProbeComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*LightProbeComponent, Dt::CLightProbeComponent::DirtyCreate);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rLightingEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rLightingEntity.SetName("Point Light");

            Dt::CTransformationFacet* pTransformationFacet = rLightingEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 3.9f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            {
                auto LightComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CPointLightComponent>();

                LightComponent->SetOuterConeAngle(glm::radians(100.0f));
                LightComponent->SetInnerConeAngle(glm::radians(90.0f));
                LightComponent->SetShadowQuality(Dt::CPointLightComponent::VeryHigh);
                LightComponent->SetRefreshMode(Dt::CPointLightComponent::Dynamic);

                LightComponent->UpdateLightness();

                rLightingEntity.AttachComponent(LightComponent);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*LightComponent, Dt::CPointLightComponent::DirtyCreate);
            }

            Dt::EntityManager::MarkEntityAsDirty(rLightingEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }
} // namespace Edit