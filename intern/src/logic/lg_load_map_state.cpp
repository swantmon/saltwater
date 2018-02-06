
#include "logic/lg_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "camera/cam_control_manager.h"
#include "camera/cam_game_control.h"

#include "data/data_actor_type.h"
#include "data/data_ar_controller_manager.h"
#include "data/data_area_light_manager.h"
#include "data/data_bloom_manager.h"
#include "data/data_dof_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_fx_type.h"
#include "data/data_post_aa_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_light_type.h"
#include "data/data_map.h"
#include "data/data_material_manager.h"
#include "data/data_mesh_manager.h"
#include "data/data_camera_actor_manager.h"
#include "data/data_model_manager.h"
#include "data/data_plugin_type.h"
#include "data/data_point_light_manager.h"
#include "data/data_light_probe_manager.h"
#include "data/data_sun_manager.h"
#include "data/data_sky_manager.h"
#include "data/data_ssao_manager.h"
#include "data/data_ssr_manager.h"
#include "data/data_texture_manager.h"
#include "data/data_transformation_facet.h"
#include "data/data_volume_fog_manager.h"

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

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Actor;
            EntityDesc.m_EntityType = Dt::SActorType::Camera;
            EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetName("Main Camera");

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
            pTransformationFacet->SetScale(glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

            Dt::CCameraActorFacet* pFacet = Dt::CameraActorManager::CreateCameraActor();

            pFacet->SetMainCamera(true);

            rEntity.SetDetailFacet(Dt::SFacetCategory::Data, pFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

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

            Dt::CCameraActorFacet* pFacet = Dt::CameraActorManager::CreateCameraActor();

            pFacet->SetMainCamera(true);
            pFacet->SetProjectionType(Dt::CCameraActorFacet::Perspective);
            pFacet->SetClearFlag(Dt::CCameraActorFacet::Skybox);

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

            Dt::CSkyFacet* pSkyboxFacet = Dt::SkyManager::CreateSky();

            pSkyboxFacet->SetRefreshMode(Dt::CSkyFacet::Static);
            pSkyboxFacet->SetType(Dt::CSkyFacet::Procedural);
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
            EntityDesc.m_EntityType     = Dt::SLightType::LightProbe;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rGlobalProbeLight = Dt::EntityManager::CreateEntity(EntityDesc);

            rGlobalProbeLight.SetName("Local light probe");

            Dt::CTransformationFacet* pTransformationFacet = rGlobalProbeLight.GetTransformationFacet();

            pTransformationFacet->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            pTransformationFacet->SetScale   (glm::vec3(1.0f));
            pTransformationFacet->SetRotation(glm::vec3(0.0f));

            Dt::CLightProbeFacet* pProbeLightFacet = Dt::LightProbeManager::CreateLightProbe();

            pProbeLightFacet->SetType(Dt::CLightProbeFacet::Sky);
            pProbeLightFacet->SetQuality(Dt::CLightProbeFacet::PX256);
            pProbeLightFacet->SetIntensity(1.0f);
            pProbeLightFacet->SetRefreshMode(Dt::CLightProbeFacet::Static);
            pProbeLightFacet->SetNear(0.01f);
            pProbeLightFacet->SetFar(1024.0f);
            pProbeLightFacet->SetParallaxCorrection(false);
            pProbeLightFacet->SetBoxSize(glm::vec3(1024.0f));

            rGlobalProbeLight.SetDetailFacet(Dt::SFacetCategory::Data, pProbeLightFacet);

            Dt::EntityManager::MarkEntityAsDirty(rGlobalProbeLight, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

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

            Dt::CMeshActorFacet* pModelActorFacet = static_cast<Dt::CMeshActorFacet*>(pSubEntity->GetDetailFacet(Dt::SFacetCategory::Data));

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

            Dt::CMeshActorFacet* pModelActorFacet = static_cast<Dt::CMeshActorFacet*>(pSubEntity->GetDetailFacet(Dt::SFacetCategory::Data));

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

            pModelActorFacet->SetMaterial(0, &rMaterial);

            Dt::MaterialManager::MarkMaterialAsDirty(rMaterial, Dt::CMaterial::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rSphere, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }
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