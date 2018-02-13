
#include "logic/lg_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "camera/cam_control_manager.h"
#include "camera/cam_game_control.h"

#include "data/data_camera_component.h"
#include "data/data_component_manager.h"
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
        BASE_CONSOLE_STREAMINFO("Logic> Loading level");
        
        // CreateEmptyScene();
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
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Main Camera");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

            auto Component = Dt::CComponentManager::GetInstance().Allocate<Dt::CCameraComponent>();

            Component.SetMainCamera(true);

            rEntity.AddComponent(Component);

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

        // -----------------------------------------------------------------------------
        // Setup cameras
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Actor;
            EntityDesc.m_EntityType     = Dt::SActorType::Camera;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Main Camera");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

            Dt::CCameraComponent* pFacet = Dt::CameraActorManager::CreateCameraActor();

            pFacet->SetMainCamera(true);
            pFacet->SetProjectionType(Dt::CCameraComponent::External);
            pFacet->SetClearFlag(Dt::CCameraComponent::Webcam);

            rEntity.SetDetailFacet(Dt::SFacetCategory::Data, pFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup environment
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Sky;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEnvironment = Dt::EntityManager::CreateEntity(EntityDesc);

            rEnvironment.SetName("Environment");

            Dt::CSkyComponent* pSkyboxFacet = Dt::SkyManager::CreateSky();

            pSkyboxFacet->SetRefreshMode(Dt::CSkyComponent::Static);
            pSkyboxFacet->SetType(Dt::CSkyComponent::Procedural);
            pSkyboxFacet->SetIntensity(40000.0f);

            rEnvironment.SetDetailFacet(Dt::SFacetCategory::Data, pSkyboxFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEnvironment, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup light
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Sun;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rSunLight = Dt::EntityManager::CreateEntity(EntityDesc);

            rSunLight.SetName("Sun");

            // -----------------------------------------------------------------------------
            // Transformation
            // -----------------------------------------------------------------------------
            Dt::CTransformationFacet* pTransformationFacet = rSunLight.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 20.0f));
            pTransformationFacet->SetScale   (glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            Dt::CSunLightFacet* pSunLightFacet = Dt::SunManager::CreateSunLight();

            pSunLightFacet->EnableTemperature(false);
            pSunLightFacet->SetColor         (glm::vec3(1.0f, 1.0f, 1.0f));
            pSunLightFacet->SetDirection     (glm::vec3(0.0f, 0.01f, -1.0f));
            pSunLightFacet->SetIntensity     (90600.0f);
            pSunLightFacet->SetTemperature   (0);
            pSunLightFacet->SetRefreshMode   (Dt::CSunLightFacet::Dynamic);

            pSunLightFacet->UpdateLightness();

            rSunLight.SetDetailFacet(Dt::SFacetCategory::Data, pSunLightFacet);

            Dt::EntityManager::MarkEntityAsDirty(rSunLight, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
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

            Dt::CMeshComponent* pModelActorFacet = static_cast<Dt::CMeshComponent*>(pSubEntity->GetDetailFacet(Dt::SFacetCategory::Data));

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

            pModelActorFacet->SetMaterial(0, &rMaterial);

            Dt::MaterialManager::MarkMaterialAsDirty(rMaterial, Dt::CMaterial::DirtyCreate);

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
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Main Camera");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

            auto CameraComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CCameraComponent>();

            CameraComponent.SetMainCamera(true);
            CameraComponent.SetProjectionType(Dt::CCameraComponent::Perspective);
            CameraComponent.SetClearFlag(Dt::CCameraComponent::Skybox);

            rEntity.AddComponent(CameraComponent);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup environment
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEnvironment = Dt::EntityManager::CreateEntity(EntityDesc);

            rEnvironment.SetName("Environment");

            auto SkyComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CSkyComponent>();

            SkyComponent.SetRefreshMode(Dt::CSkyComponent::Static);
            SkyComponent.SetType(Dt::CSkyComponent::Procedural);
            SkyComponent.SetIntensity(40000.0f);

            rEnvironment.AddComponent(SkyComponent);

            Dt::EntityManager::MarkEntityAsDirty(rEnvironment, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup light
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rGlobalProbeLight = Dt::EntityManager::CreateEntity(EntityDesc);

            rGlobalProbeLight.SetName("Local light probe");

            Dt::CTransformationFacet* pTransformationFacet = rGlobalProbeLight.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            pTransformationFacet->SetScale   (glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            auto LightProbeComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CLightProbeComponent>();

            LightProbeComponent.SetType(Dt::CLightProbeComponent::Sky);
            LightProbeComponent.SetQuality(Dt::CLightProbeComponent::PX256);
            LightProbeComponent.SetIntensity(1.0f);
            LightProbeComponent.SetRefreshMode(Dt::CLightProbeComponent::Static);
            LightProbeComponent.SetNear(0.01f);
            LightProbeComponent.SetFar(1024.0f);
            LightProbeComponent.SetParallaxCorrection(false);
            LightProbeComponent.SetBoxSize(glm::vec3(1024.0f));

            rGlobalProbeLight.AddComponent(LightProbeComponent);

            Dt::EntityManager::MarkEntityAsDirty(rGlobalProbeLight, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rSunLight = Dt::EntityManager::CreateEntity(EntityDesc);

            rSunLight.SetName("Sun");

            // -----------------------------------------------------------------------------
            // Transformation
            // -----------------------------------------------------------------------------
            Dt::CTransformationFacet* pTransformationFacet = rSunLight.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 20.0f));
            pTransformationFacet->SetScale   (glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            auto SunComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CSunComponent>();

            SunComponent.EnableTemperature(false);
            SunComponent.SetColor         (glm::vec3(1.0f, 1.0f, 1.0f));
            SunComponent.SetDirection     (glm::vec3(0.0f, 0.01f, -1.0f));
            SunComponent.SetIntensity     (90600.0f);
            SunComponent.SetTemperature   (0);
            SunComponent.SetRefreshMode   (Dt::CSunComponent::Dynamic);

            SunComponent.UpdateLightness();

            rSunLight.AddComponent(SunComponent);

            Dt::EntityManager::MarkEntityAsDirty(rSunLight, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Static;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("SSAO");

            // -----------------------------------------------------------------------------
            // Transformation
            // -----------------------------------------------------------------------------
            auto pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            pTransformationFacet->SetScale   (glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            rEntity.AddComponent(Dt::CComponentManager::GetInstance().Allocate<Dt::CSSAOComponent>());

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
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

            auto pMeshComponent = pSubEntity->GetComponent<Dt::CMeshComponent>();

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

            pMeshComponent->SetMaterial(0, &rMaterial);

            Dt::MaterialManager::MarkMaterialAsDirty(rMaterial, Dt::CMaterial::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rSphere, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SModelFileDescriptor ModelFileDesc;

            ModelFileDesc.m_pFileName = "models/plane.obj";
            ModelFileDesc.m_GenFlag = Dt::SGeneratorFlag::DefaultFlipUVs;

            Dt::CModel& rModel = Dt::ModelManager::CreateModel(ModelFileDesc);

            // -----------------------------------------------------------------------------

            Dt::CEntity& rSphere = Dt::EntityManager::CreateEntityFromModel(rModel);

            rSphere.SetName("Plane");

            Dt::CTransformationFacet* pTransformationFacet = rSphere.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            pTransformationFacet->SetScale(glm::vec3(0.01f));
            pTransformationFacet->SetRotation(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));

            // -----------------------------------------------------------------------------

            Dt::CEntity* pSubEntity = rSphere.GetHierarchyFacet()->GetFirstChild();

            auto pMeshComponent = pSubEntity->GetComponent<Dt::CMeshComponent>();

            Dt::SMaterialDescriptor MaterialFileDesc;

            MaterialFileDesc.m_pFileName = 0;
            MaterialFileDesc.m_pMaterialName = "Grey";
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
            MaterialFileDesc.m_AlbedoColor = glm::vec3(0.8f, 0.8f, 0.8f);
            MaterialFileDesc.m_TilingOffset = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);

            Dt::CMaterial& rMaterial = Dt::MaterialManager::CreateMaterial(MaterialFileDesc);

            pMeshComponent->SetMaterial(0, &rMaterial);

            Dt::MaterialManager::MarkMaterialAsDirty(rMaterial, Dt::CMaterial::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rSphere, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
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