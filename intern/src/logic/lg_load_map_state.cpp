
#include "logic/lg_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "camera/cam_control_manager.h"
#include "camera/cam_game_control.h"

#include "data/data_camera_component.h"
#include "data/data_component_manager.h"
#include "data/data_component_facet.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_light_probe_component.h"
#include "data/data_map.h"
#include "data/data_material_manager.h"
#include "data/data_mesh_component.h"
#include "data/data_model_manager.h"
#include "data/data_sky_component.h"
#include "data/data_ssao_component.h"
#include "data/data_sun_component.h"
#include "data/data_texture_manager.h"
#include "data/data_transformation_facet.h"

#include "logic/lg_load_map_state.h"

namespace
{
    class CLgLoadMapState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CLgLoadMapState)
        
    public:
        
        int OnEnter(Base::CTextReader& _rSerializer);
        int OnLeave();
        int OnRun();
        
    private:
        
        void CreateEmptyScene();
        void CreateDefaultScene();
     
    };
} // namespace

namespace
{
    int CLgLoadMapState::OnEnter(Base::CTextReader& _rSerializer)
    {
        BASE_UNUSED(_rSerializer);

        // -----------------------------------------------------------------------------
        // Give serialize module to all data items that is needed for loading a
        // certain map
        // -----------------------------------------------------------------------------        
        //CreateEmptyScene();
        CreateDefaultScene();

        BASE_CONSOLE_STREAMINFO("Logic> Loading level finished.");
        
        return Lg::LoadMap::SResult::LoadMap;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgLoadMapState::OnLeave()
    {
        return Lg::LoadMap::SResult::LoadMap;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgLoadMapState::OnRun()
    {
        return Lg::LoadMap::SResult::Play;
    }

    // -----------------------------------------------------------------------------

    void CLgLoadMapState::CreateEmptyScene()
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

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

            auto Component = Dt::CComponentManager::GetInstance().Allocate<Dt::CCameraComponent>();

            Component->SetMainCamera(true);

            rEntity.AttachComponent(Component);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(Component, Dt::CCameraComponent::DirtyCreate);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------
#if PLATFORM_ANDROID
    void CLgLoadMapState::CreateDefaultScene()
    {
        // -----------------------------------------------------------------------------
        // Allocate a map
        // -----------------------------------------------------------------------------
        Dt::Map::AllocateMap(1, 1);

        Dt::CEntity* pCameraEntity = nullptr;

        // -----------------------------------------------------------------------------
        // Setup cameras
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            pCameraEntity = &rEntity;

            rEntity.SetName("Main Camera");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

            Dt::CCameraComponent* pFacet = Dt::CComponentManager::GetInstance().Allocate<Dt::CCameraComponent>();

            pFacet->SetMainCamera(true);

            rEntity.GetComponentFacet()->AddComponent(pFacet);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pFacet, Dt::CCameraComponent::DirtyCreate);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup environment
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetComponents;

            Dt::CEntity& rEnvironment = Dt::EntityManager::CreateEntity(EntityDesc);

            rEnvironment.SetName("Environment");

            Dt::CSkyComponent* pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CSkyComponent>();

            pComponent->SetRefreshMode(Dt::CSkyComponent::Static);
            pComponent->SetType(Dt::CSkyComponent::Procedural);
            pComponent->SetIntensity(40000.0f);

            rEnvironment.GetComponentFacet()->AddComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pComponent, Dt::CSkyComponent::DirtyCreate);

            Dt::EntityManager::MarkEntityAsDirty(rEnvironment, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup light
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEnvironmentEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEnvironmentEntity.SetName("Sun");

            // -----------------------------------------------------------------------------
            // Transformation
            // -----------------------------------------------------------------------------
            Dt::CTransformationFacet* pTransformationFacet = rEnvironmentEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 20.0f));
            pTransformationFacet->SetScale   (glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            Dt::CSunComponent* pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CSunComponent>();

            pComponent->EnableTemperature(false);
            pComponent->SetColor         (glm::vec3(1.0f, 1.0f, 1.0f));
            pComponent->SetDirection     (glm::vec3(0.0f, 0.01f, -1.0f));
            pComponent->SetIntensity     (90600.0f);
            pComponent->SetTemperature   (0);
            pComponent->SetRefreshMode   (Dt::CSunComponent::Dynamic);

            pComponent->UpdateLightness();

            rEnvironmentEntity.GetComponentFacet()->AddComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pComponent, Dt::CSunComponent::DirtyCreate);

            Dt::EntityManager::MarkEntityAsDirty(rEnvironmentEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup entities
        // -----------------------------------------------------------------------------
        {
            Dt::SModelFileDescriptor ModelFileDesc;

            ModelFileDesc.m_pFileName = "models/MatTester.obj";
            ModelFileDesc.m_GenFlag = Dt::SGeneratorFlag::DefaultFlipUVs;

            Dt::CModel& rModel = Dt::ModelManager::CreateModel(ModelFileDesc);

            // -----------------------------------------------------------------------------

            Dt::CEntity& rSphere = Dt::EntityManager::CreateEntityFromModel(rModel);

            rSphere.SetName("Sphere");

            Dt::CTransformationFacet* pTransformationFacet = rSphere.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.1f));
            pTransformationFacet->SetScale(glm::vec3(0.01f));
            pTransformationFacet->SetRotation(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));

            // -----------------------------------------------------------------------------

            Dt::CEntity* pSubEntity = rSphere.GetHierarchyFacet()->GetFirstChild();

            Dt::CMeshComponent* pComponent = pSubEntity->GetComponentFacet()->GetComponent<Dt::CMeshComponent>();

            Dt::SMaterialDescriptor MaterialFileDesc;

            MaterialFileDesc.m_pFileName = 0;
            MaterialFileDesc.m_pMaterialName = "Red Sparrow";
            MaterialFileDesc.m_pColorMap = 0;
            MaterialFileDesc.m_pNormalMap = 0;
            MaterialFileDesc.m_pRoughnessMap = 0;
            MaterialFileDesc.m_pMetalMaskMap = 0;
            MaterialFileDesc.m_pAOMap = 0;
            MaterialFileDesc.m_pBumpMap = 0;
            MaterialFileDesc.m_Roughness = 1.0f;
            MaterialFileDesc.m_Reflectance = 0.0f;
            MaterialFileDesc.m_MetalMask = 0.0f;
            MaterialFileDesc.m_Displacement = 0.0f;
            MaterialFileDesc.m_AlbedoColor = glm::vec3(1.0f, 0.0f, 0.0f);
            MaterialFileDesc.m_TilingOffset = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);

            Dt::CMaterial& rMaterial = Dt::MaterialManager::CreateMaterial(MaterialFileDesc);

            pComponent->SetMaterial(0, &rMaterial);

            Dt::MaterialManager::MarkMaterialAsDirty(rMaterial, Dt::CMaterial::DirtyCreate);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pComponent, Dt::CMeshComponent::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rSphere, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }
#else
void CLgLoadMapState::CreateDefaultScene()
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

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

            auto Component = Dt::CComponentManager::GetInstance().Allocate<Dt::CCameraComponent>();

            Component->SetMainCamera(true);
            Component->SetProjectionType(Dt::CCameraComponent::Perspective);
            Component->SetClearFlag(Dt::CCameraComponent::Skybox);

            rEntity.AttachComponent(Component);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(Component, Dt::CCameraComponent::DirtyCreate);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup light
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rLightingEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rLightingEntity.SetName("Local light probe");

            Dt::CTransformationFacet* pTransformationFacet = rLightingEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            pTransformationFacet->SetScale   (glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            {
                auto LightProbeComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CLightProbeComponent>();

                LightProbeComponent->SetType(Dt::CLightProbeComponent::Sky);
                LightProbeComponent->SetQuality(Dt::CLightProbeComponent::PX256);
                LightProbeComponent->SetIntensity(1.0f);
                LightProbeComponent->SetRefreshMode(Dt::CLightProbeComponent::Static);
                LightProbeComponent->SetNear(0.01f);
                LightProbeComponent->SetFar(1024.0f);
                LightProbeComponent->SetParallaxCorrection(false);
                LightProbeComponent->SetBoxSize(glm::vec3(1024.0f));

                rLightingEntity.AttachComponent(LightProbeComponent);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(LightProbeComponent, Dt::CLightProbeComponent::DirtyCreate);
            }

            Dt::EntityManager::MarkEntityAsDirty(rLightingEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

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

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(SunComponent, Dt::CSunComponent::DirtyCreate);
            }

            {
                auto Component = Dt::CComponentManager::GetInstance().Allocate<Dt::CSkyComponent>();

                Component->SetRefreshMode(Dt::CSkyComponent::Static);
                Component->SetType(Dt::CSkyComponent::Procedural);
                Component->SetIntensity(40000.0f);

                rEnvironmentEntity.AttachComponent(Component);

                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(Component, Dt::CSkyComponent::DirtyCreate);
            }

            Dt::EntityManager::MarkEntityAsDirty(rEnvironmentEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("SSAO");

            // -----------------------------------------------------------------------------
            // Transformation
            // -----------------------------------------------------------------------------
            auto pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            pTransformationFacet->SetScale   (glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            auto Component = Dt::CComponentManager::GetInstance().Allocate<Dt::CSSAOComponent>();

            rEntity.AttachComponent(Component);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(Component, Dt::CSSAOComponent::DirtyCreate);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

    }
#endif
} // namespace

namespace Lg
{
namespace LoadMap
{
    int OnEnter(Base::CTextReader& _rSerializer)
    {
        return CLgLoadMapState::GetInstance().OnEnter(_rSerializer);
    }
    
    // -----------------------------------------------------------------------------
    
    int OnLeave()
    {
        return CLgLoadMapState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnRun()
    {
        return CLgLoadMapState::GetInstance().OnRun();
    }
} // namespace LoadMap
} // namespace Lg