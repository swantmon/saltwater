
#include "logic/lg_precompiled.h"

#include "base/base_console.h"
#include "base/base_serialize_text_reader.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"
#include "base/base_vector3.h"

#include "camera/cam_control_manager.h"
#include "camera/cam_game_control.h"

#include "data/data_actor_facet.h"
#include "data/data_actor_manager.h"
#include "data/data_ar_controller_manager.h"
#include "data/data_bloom_manager.h"
#include "data/data_dof_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_fx_type.h"
#include "data/data_fxaa_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_light_type.h"
#include "data/data_map.h"
#include "data/data_material_manager.h"
#include "data/data_model_manager.h"
#include "data/data_plugin_type.h"
#include "data/data_point_light_manager.h"
#include "data/data_light_probe_manager.h"
#include "data/data_sun_manager.h"
#include "data/data_sky_manager.h"
#include "data/data_script_facet.h"
#include "data/data_script_manager.h"
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
        
        void CreateDefaultScene();
        void CreatePBRARScene();        
    };
} // namespace

namespace
{
    int CLgLoadMapState::OnEnter(Base::CTextReader& _rSerializer)
    {
        // -----------------------------------------------------------------------------
        // Give serialize module to all data items that is needed for loading a
        // certain map
        // -----------------------------------------------------------------------------

        int LevelIndexDebug;
        
        _rSerializer >> LevelIndexDebug;

        BASE_CONSOLE_STREAMINFO("Logic> Loading level number " << LevelIndexDebug);
        
        CreateDefaultScene();
//        CreatePBRARScene();

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

            pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 10.0f));
            pTransformationFacet->SetScale(Base::Float3(1.0f));
            pTransformationFacet->SetRotation(Base::Float3(0.0f, 0.0f, 0.0f));

            Dt::CCameraActorFacet* pFacet = Dt::ActorManager::CreateCameraActor();

            pFacet->SetMainCamera(true);

            rEntity.SetDetailFacet(Dt::SFacetCategory::Data, pFacet);

            Dt::CScriptFacet* pScriptFacet = Dt::ScriptManager::CreateScript();

            pScriptFacet->SetScriptFile("scripts/camera_behavior.lua");

            rEntity.SetDetailFacet(Dt::SFacetCategory::Script, pScriptFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup environment
        // -----------------------------------------------------------------------------
        {
//             Dt::STextureDescriptor TextureDescriptor;
// 
//             TextureDescriptor.m_NumberOfPixelsU = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
//             TextureDescriptor.m_NumberOfPixelsV = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
//             TextureDescriptor.m_NumberOfPixelsW = 1;
//             TextureDescriptor.m_Format          = Dt::CTextureBase::R16G16B16_FLOAT;
//             TextureDescriptor.m_Semantic        = Dt::CTextureBase::HDR;
//             TextureDescriptor.m_Binding         = Dt::CTextureBase::ShaderResource;
//             TextureDescriptor.m_pPixels         = 0;
//             TextureDescriptor.m_pFileName       = "environments/OutputCube.dds";
//             TextureDescriptor.m_pIdentifier     = 0;
// 
//             Dt::CTextureCube* pPanoramaTexture = Dt::TextureManager::CreateCubeTexture(TextureDescriptor);
// 
//             Dt::TextureManager::MarkTextureAsDirty(pPanoramaTexture, Dt::CTextureBase::DirtyCreate);
// 
//             // -----------------------------------------------------------------------------
// 
//             Dt::SEntityDescriptor EntityDesc;
// 
//             EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
//             EntityDesc.m_EntityType     = Dt::SLightType::Sky;
//             EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;
// 
//             Dt::CEntity& rEnvironment = Dt::EntityManager::CreateEntity(EntityDesc);
// 
//             rEnvironment.SetName("Environment");
// 
//             // -----------------------------------------------------------------------------
//             // Transformation
//             // -----------------------------------------------------------------------------
//             Dt::CTransformationFacet* pTransformationFacet = rEnvironment.GetTransformationFacet();
// 
//             pTransformationFacet->SetPosition(Base::Float3(0.0f));
//             pTransformationFacet->SetScale   (Base::Float3(1.0f));
//             pTransformationFacet->SetRotation(Base::Float3(0.0f));
// 
//             Dt::CSkyFacet* pSkyboxFacet = Dt::LightManager::CreateSky();
// 
//             pSkyboxFacet->SetType     (Dt::CSkyFacet::Cubemap);
//             pSkyboxFacet->SetCubemap  (pPanoramaTexture);
//             pSkyboxFacet->SetIntensity(10000.0f);
// 
//             rEnvironment.SetDetailFacet(Dt::SFacetCategory::Data, pSkyboxFacet);
// 
//             Dt::EntityManager::MarkEntityAsDirty(rEnvironment, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);


            Dt::STextureDescriptor TextureDescriptor;

            TextureDescriptor.m_NumberOfPixelsU = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
            TextureDescriptor.m_NumberOfPixelsV = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_Format          = Dt::CTextureBase::R16G16B16_FLOAT;
            TextureDescriptor.m_Semantic        = Dt::CTextureBase::HDR;
            TextureDescriptor.m_Binding         = Dt::CTextureBase::ShaderResource;
            TextureDescriptor.m_pPixels         = 0;
            TextureDescriptor.m_pFileName       = "environments/Ridgecrest_Road_Ref.hdr";
            TextureDescriptor.m_pIdentifier     = 0;
            

            Dt::CTexture2D* pPanoramaTexture = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

            Dt::TextureManager::MarkTextureAsDirty(pPanoramaTexture, Dt::CTextureBase::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Sky;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEnvironment = Dt::EntityManager::CreateEntity(EntityDesc);

            rEnvironment.SetName("Environment");

            Dt::CSkyFacet* pSkyboxFacet = Dt::SkyManager::CreateSky();

            pSkyboxFacet->SetType(Dt::CSkyFacet::Panorama);
            pSkyboxFacet->SetPanorama(pPanoramaTexture);
            pSkyboxFacet->SetIntensity(10000.0f);

            rEnvironment.SetDetailFacet(Dt::SFacetCategory::Data, pSkyboxFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEnvironment, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup effects
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::FX;
            EntityDesc.m_EntityType     = Dt::SFXType::FXAA;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEffectEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEffectEntity.SetName("FXAA");

            Dt::CFXAAFXFacet* pEffectFacet = Dt::FXAAManager::CreateFXAAFX();

            rEffectEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEffectEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::FX;
            EntityDesc.m_EntityType     = Dt::SFXType::SSR;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEffectEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEffectEntity.SetName("SSR");

            Dt::CSSRFXFacet* pEffectFacet = Dt::SSRFXManager::CreateSSRFX();

            rEffectEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEffectEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::FX;
            EntityDesc.m_EntityType     = Dt::SFXType::Bloom;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEffectEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEffectEntity.SetName("Bloom");

            Dt::CBloomFXFacet* pEffectFacet = Dt::BloomManager::CreateBloomFX();

            pEffectFacet->SetSize(1);
            pEffectFacet->SetIntensity(0.5f);

            rEffectEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEffectEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::FX;
            EntityDesc.m_EntityType     = Dt::SFXType::SSAO;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEffectEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEffectEntity.SetName("SSAO");

            Dt::CSSAOFXFacet* pEffectFacet = Dt::SSAOManager::CreateSSAOFX();

            rEffectEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEffectEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup light
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::LightProbe;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rGlobalProbeLight = Dt::EntityManager::CreateEntity(EntityDesc);

            rGlobalProbeLight.SetName("Light probe");

            Dt::CLightProbeFacet* pProbeLightFacet = Dt::LightProbeManager::CreateLightProbe();

            pProbeLightFacet->SetType(Dt::CLightProbeFacet::Sky);
            pProbeLightFacet->SetQuality(Dt::CLightProbeFacet::PX512);
            pProbeLightFacet->SetIntensity(1.0f);

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

            pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 20.0f));
            pTransformationFacet->SetScale   (Base::Float3(1.0f));
            pTransformationFacet->SetRotation(Base::Float3(0.0f));

            Dt::CSunLightFacet* pSunLightFacet = Dt::SunManager::CreateSunLight();

            pSunLightFacet->EnableTemperature(false);
            pSunLightFacet->SetColor         (Base::Float3(1.0f, 1.0f, 1.0f));
            pSunLightFacet->SetDirection     (Base::Float3(0.0f, 0.0f, -1.0f));
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

            ModelFileDesc.m_pFileName = "models/plane.obj";
            ModelFileDesc.m_GenFlag   = Dt::SGeneratorFlag::DefaultFlipUVs;

            Dt::CModel& rModel = Dt::ModelManager::CreateModel(ModelFileDesc);

            // -----------------------------------------------------------------------------

            Dt::CEntity& rPlane = Dt::EntityManager::CreateEntityFromModel(rModel);

            rPlane.SetName("Plane");

            Dt::CTransformationFacet* pTransformationFacet = rPlane.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 0.1f));
            pTransformationFacet->SetScale(Base::Float3(0.01f));
            pTransformationFacet->SetRotation(Base::Float3(Base::DegreesToRadians(-90.0f), 0.0f, 0.0f));

            Dt::EntityManager::MarkEntityAsDirty(rPlane, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }


        // -----------------------------------------------------------------------------

        {
            Dt::SModelFileDescriptor ModelFileDesc;

            ModelFileDesc.m_pFileName = "models/sphere.obj";
            ModelFileDesc.m_GenFlag = Dt::SGeneratorFlag::DefaultFlipUVs;

            Dt::CModel& rModel = Dt::ModelManager::CreateModel(ModelFileDesc);

            // -----------------------------------------------------------------------------

            Dt::CEntity& rSphere = Dt::EntityManager::CreateEntityFromModel(rModel);

            rSphere.SetName("Sphere");

            Dt::CTransformationFacet* pTransformationFacet = rSphere.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(4.0f, 4.0f, 1.0f));
            pTransformationFacet->SetScale(Base::Float3(0.1f));
            pTransformationFacet->SetRotation(Base::Float3(Base::DegreesToRadians(-90.0f), 0.0f, 0.0f));

            // -----------------------------------------------------------------------------

            Dt::CEntity* pSubEntity = rSphere.GetHierarchyFacet()->GetFirstChild();

            Dt::CMeshActorFacet* pModelActorFacet = static_cast<Dt::CMeshActorFacet*>(pSubEntity->GetDetailFacet(Dt::SFacetCategory::Data));

            Dt::SMaterialDescriptor MaterialFileDesc;

            MaterialFileDesc.m_pFileName = "materials/naturals/metals/Gold_Worn_00.mat";

            Dt::CMaterial& rMaterial = Dt::MaterialManager::CreateMaterial(MaterialFileDesc);

            pModelActorFacet->SetMaterial(0, &rMaterial);

            Dt::MaterialManager::MarkMaterialAsDirty(rMaterial, Dt::CMaterial::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rSphere, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);


//             Dt::CScriptFacet* pScriptFacet = Dt::ScriptManager::CreateScript();
// 
//             pScriptFacet->SetScriptFile("scripts/move_circle.lua");
// 
//             rSphere.SetDetailFacet(Dt::SFacetCategory::Script, pScriptFacet);
        }

        // -----------------------------------------------------------------------------

        {
            Dt::SModelFileDescriptor ModelFileDesc;

            ModelFileDesc.m_pFileName = "models/plane.obj";
            ModelFileDesc.m_GenFlag = Dt::SGeneratorFlag::DefaultFlipUVs;

            Dt::CModel& rModel = Dt::ModelManager::CreateModel(ModelFileDesc);

            // -----------------------------------------------------------------------------

            Dt::CEntity& rPlane = Dt::EntityManager::CreateEntityFromModel(rModel);

            Dt::CTransformationFacet* pTransformationFacet = rPlane.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 0.0f));
            pTransformationFacet->SetScale(Base::Float3(0.1f));
            pTransformationFacet->SetRotation(Base::Float3(Base::DegreesToRadians(-90.0f), 0.0f, 0.0f));

            // -----------------------------------------------------------------------------

            Dt::CEntity* pSubEntity = rPlane.GetHierarchyFacet()->GetFirstChild();

            Dt::CMeshActorFacet* pModelActorFacet = static_cast<Dt::CMeshActorFacet*>(pSubEntity->GetDetailFacet(Dt::SFacetCategory::Data));

            Dt::SMaterialDescriptor MaterialFileDesc;

            MaterialFileDesc.m_pFileName = "materials/tests/checker.mat";

            Dt::CMaterial& rMaterial = Dt::MaterialManager::CreateMaterial(MaterialFileDesc);

            pModelActorFacet->SetMaterial(0, &rMaterial);

            Dt::MaterialManager::MarkMaterialAsDirty(rMaterial, Dt::CMaterial::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::EntityManager::MarkEntityAsDirty(rPlane, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------
    
    void CLgLoadMapState::CreatePBRARScene()
    {
        {
//             Dt::SEntityDescriptor EntityDesc;
// 
//             EntityDesc.m_EntityCategory = Dt::SEntityCategory::Actor;
//             EntityDesc.m_EntityType     = Dt::SActorType::AR;
//             EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;
// 
//             Dt::CEntity& rSphere = Dt::EntityManager::CreateEntity(EntityDesc);
// 
//             Dt::CTransformationFacet* pTransformationFacet = rSphere.GetTransformationFacet();
// 
//             pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 0.0f));
//             pTransformationFacet->SetScale   (Base::Float3(1.0f));
//             pTransformationFacet->SetRotation(Base::Float3(Base::DegreesToRadians(-90.0f), 0.0f, 0.0f));
// 
//             Dt::CARActorFacet* pModelActorFacet = Dt::ActorManager::CreateARActor();
// 
//             Dt::SModelFileDescriptor ModelFileDesc;
// 
//             ModelFileDesc.m_pFileName = "models/plane.obj";
//             ModelFileDesc.m_GenFlag   = Dt::SGeneratorFlag::DefaultFlipUVs;
// 
//             Dt::SMaterialFileDescriptor MaterialFileDesc;
// 
//             MaterialFileDesc.m_pFileName = "materials/tests/background.mat";
// 
//             pModelActorFacet->SetModel(&Dt::ModelManager::CreateModel(ModelFileDesc));
//             pModelActorFacet->SetMaterial(0, &Dt::MaterialManager::CreateMaterial(MaterialFileDesc));
// 
//             rSphere.SetDetailFacet(Dt::SFacetCategory::Data, pModelActorFacet);
// 
//             Dt::EntityManager::MarkEntityAsDirty(rSphere, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
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