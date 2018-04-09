
#include "editor/edit_precompiled.h"

#include "base/base_serialize_text_reader.h"

#include "core/core_console.h"

#include "data/data_camera_component.h"
#include "data/data_component.h"
#include "data/data_component_facet.h"
#include "data/data_component_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_light_probe_component.h"
#include "data/data_map.h"
#include "data/data_material.h"
#include "data/data_material_component.h"
#include "data/data_material_manager.h"
#include "data/data_mesh_component.h"
#include "data/data_script_component.h"
#include "data/data_sky_component.h"
#include "data/data_ssao_component.h"
#include "data/data_sun_component.h"
#include "data/data_transformation_facet.h"

#include "editor/edit_load_map_state.h"

#include "script/script_ar_camera_control_script.h"
#include "script/script_ar_controller_script.h"
#include "script/script_ar_place_object_on_touch_script.h"
#include "script/script_camera_control_script.h"
#include "script/script_script_manager.h"

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
} // namespace Edit

namespace Edit
{
    CLoadMapState::CLoadMapState()
        : m_pMapfile("")
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CLoadMapState::~CLoadMapState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CLoadMapState::SetMapfile(const char* _pFilename)
    {
        BASE_CONSOLE_STREAMINFO("Edit> Setting load level to " << _pFilename);

        m_pMapfile = _pFilename;
    }
    
    // -----------------------------------------------------------------------------
    
    const char* CLoadMapState::GetMapfile() const
    {
        return m_pMapfile;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CLoadMapState::InternOnEnter()
    {
        CreateDefaultScene();
        
        return Edit::CState::LoadMap;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CLoadMapState::InternOnLeave()
    {
        return Edit::CState::LoadMap;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CLoadMapState::InternOnRun()
    {
        return CState::Edit;
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

            auto ScriptComponent2 = Dt::CComponentManager::GetInstance().Allocate<Scpt::CARControllerScript>();

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
                Component->SetIntensity(120000.0f);

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

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Box");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 4.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(glm::radians(0.0f), glm::radians(0.0f), 0.0f));

            // -----------------------------------------------------------------------------

            auto pMeshComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMeshComponent>();

            pMeshComponent->SetMeshType(Dt::CMeshComponent::Cone);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMeshComponent, Dt::CMeshComponent::DirtyCreate);

            rEntity.AttachComponent(pMeshComponent);

            // -----------------------------------------------------------------------------

            auto pMaterial = Dt::MaterialManager::CreateMaterialFromName("Red Sparrow");

            pMaterial->SetColor(glm::vec3(1.0f, 0.0f, 0.0f));

            auto pMaterialComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();

            pMaterialComponent->SetMaterial(pMaterial);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMaterialComponent, Dt::CMaterialComponent::DirtyCreate);

            rEntity.AttachComponent(pMaterialComponent);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Plane");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            pTransformationFacet->SetScale(glm::vec3(4.0f, 4.0f, 0.1f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

            // -----------------------------------------------------------------------------

            auto pMeshComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMeshComponent>();

            pMeshComponent->SetMeshType(Dt::CMeshComponent::Box);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMeshComponent, Dt::CMeshComponent::DirtyCreate);

            rEntity.AttachComponent(pMeshComponent);

            // -----------------------------------------------------------------------------

            auto pMaterialComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();

            pMaterialComponent->SetMaterial(Dt::MaterialManager::GetDefaultMaterial());

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pMaterialComponent, Dt::CMaterialComponent::DirtyCreate);

            rEntity.AttachComponent(pMaterialComponent);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }
} // namespace Edit