
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
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_light_facet.h"
#include "data/data_light_manager.h"
#include "data/data_fx_facet.h"
#include "data/data_fx_manager.h"
#include "data/data_map.h"
#include "data/data_material_manager.h"
#include "data/data_model_manager.h"
#include "data/data_plugin_facet.h"
#include "data/data_plugin_manager.h"
#include "data/data_script_facet.h"
#include "data/data_script_manager.h"
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
        
        void CreatePBRTestScene();
        void CreatePBRARScene();
        void CreatePBRSponzaScene();
        
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
        
//        CreatePBRTestScene();
        CreatePBRARScene();
//        CreatePBRSponzaScene();

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
    
    void CLgLoadMapState::CreatePBRTestScene()
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

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 10.0f));
            pTransformationFacet->SetScale   (Base::Float3(1.0f));
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
            Dt::STextureDescriptor TextureDescriptor;

            TextureDescriptor.m_NumberOfPixelsU  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
            TextureDescriptor.m_NumberOfPixelsV  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
            TextureDescriptor.m_NumberOfPixelsW  = 1;
            TextureDescriptor.m_Format           = Dt::CTextureBase::R16G16B16_FLOAT;
            TextureDescriptor.m_Semantic         = Dt::CTextureBase::HDR;
            TextureDescriptor.m_pPixels          = 0;
            TextureDescriptor.m_pFileName        = "environments/PaperMill_E_3k.hdr";
            TextureDescriptor.m_pIdentifier      = 0;

            Dt::CTexture2D* pPanoramaTexture = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

            // -----------------------------------------------------------------------------

            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Skybox;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEnvironment = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CSkyboxFacet* pSkyboxFacet = Dt::LightManager::CreateSkybox();

            pSkyboxFacet->SetType     (Dt::CSkyboxFacet::Panorama);
            pSkyboxFacet->SetTexture  (pPanoramaTexture);
            pSkyboxFacet->SetIntensity(5000.0f);

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

            Dt::CFXAAFXFacet* pEffectFacet = Dt::FXManager::CreateFXAAFX();

            rEffectEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEffectEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::FX;
            EntityDesc.m_EntityType     = Dt::SFXType::SSR;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEffectEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CSSRFXFacet* pEffectFacet = Dt::FXManager::CreateSSRFX();

            rEffectEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEffectEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::FX;
            EntityDesc.m_EntityType     = Dt::SFXType::Bloom;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEffectEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CBloomFXFacet* pEffectFacet = Dt::FXManager::CreateBloomFX();

            rEffectEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEffectEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::FX;
            EntityDesc.m_EntityType     = Dt::SFXType::SSAO;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEffectEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CSSAOFXFacet* pEffectFacet = Dt::FXManager::CreateSSAOFX();

            rEffectEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEffectEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup light
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Sun;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rSunLight = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CSunLightFacet* pSunLightFacet = Dt::LightManager::CreateSunLight();

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

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::GlobalProbe;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rGlobalProbeLight = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CGlobalProbeLightFacet* pGlobalProbeLightFacet = Dt::LightManager::CreateGlobalProbeLight();

            pGlobalProbeLightFacet->SetType     (Dt::CGlobalProbeLightFacet::Sky);
            pGlobalProbeLightFacet->SetQuality  (Dt::CGlobalProbeLightFacet::PX512);
            pGlobalProbeLightFacet->SetIntensity(1.0f);

            rGlobalProbeLight.SetDetailFacet(Dt::SFacetCategory::Data, pGlobalProbeLightFacet);

            Dt::EntityManager::MarkEntityAsDirty(rGlobalProbeLight, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup entities
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Actor;
            EntityDesc.m_EntityType     = Dt::SActorType::Model;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rPlane = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CTransformationFacet* pTransformationFacet = rPlane.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 0.1f));
            pTransformationFacet->SetScale   (Base::Float3(0.01f));
            pTransformationFacet->SetRotation(Base::Float3(Base::DegreesToRadians(-90.0f), 0.0f, 0.0f));

            Dt::CModelActorFacet* pModelActorFacet = Dt::ActorManager::CreateModelActor();

            Dt::SModelFileDescriptor ModelFileDesc;

            ModelFileDesc.m_pFileName = "models/plane.obj";
            ModelFileDesc.m_GenFlag   = Dt::SGeneratorFlag::DefaultFlipUVs;

            pModelActorFacet->SetModel(&Dt::ModelManager::CreateModel(ModelFileDesc));

            rPlane.SetDetailFacet(Dt::SFacetCategory::Data, pModelActorFacet);

            Dt::EntityManager::MarkEntityAsDirty(rPlane, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Actor;
            EntityDesc.m_EntityType     = Dt::SActorType::Model;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rPlane = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CTransformationFacet* pTransformationFacet = rPlane.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 0.0f));
            pTransformationFacet->SetScale   (Base::Float3(0.1f));
            pTransformationFacet->SetRotation(Base::Float3(Base::DegreesToRadians(-90.0f), 0.0f, 0.0f));

            Dt::CModelActorFacet* pModelActorFacet = Dt::ActorManager::CreateModelActor();

            Dt::SModelFileDescriptor ModelFileDesc;

            ModelFileDesc.m_pFileName = "models/plane.obj";
            ModelFileDesc.m_GenFlag   = Dt::SGeneratorFlag::DefaultFlipUVs;

            Dt::SMaterialFileDescriptor MaterialFileDesc;

            MaterialFileDesc.m_pFileName = "materials/tests/checker.mat";

            pModelActorFacet->SetModel(&Dt::ModelManager::CreateModel(ModelFileDesc));
            pModelActorFacet->SetMaterial(0, &Dt::MaterialManager::CreateMaterial(MaterialFileDesc));

            rPlane.SetDetailFacet(Dt::SFacetCategory::Data, pModelActorFacet);

            Dt::EntityManager::MarkEntityAsDirty(rPlane, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Actor;
            EntityDesc.m_EntityType     = Dt::SActorType::Model;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rSphere = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CTransformationFacet* pTransformationFacet = rSphere.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 2.0f));
            pTransformationFacet->SetScale   (Base::Float3(0.1f));
            pTransformationFacet->SetRotation(Base::Float3(Base::DegreesToRadians(-90.0f), 0.0f, 0.0f));

            Dt::CModelActorFacet* pModelActorFacet = Dt::ActorManager::CreateModelActor();

            Dt::SModelFileDescriptor ModelFileDesc;

            ModelFileDesc.m_pFileName = "models/sphere.obj";
            ModelFileDesc.m_GenFlag   = Dt::SGeneratorFlag::DefaultFlipUVs;

            Dt::SMaterialFileDescriptor MaterialFileDesc;

            MaterialFileDesc.m_pFileName = "materials/naturals/metals/Chrome_Glossy_00.mat";

            pModelActorFacet->SetModel(&Dt::ModelManager::CreateModel(ModelFileDesc));
            pModelActorFacet->SetMaterial(0, &Dt::MaterialManager::CreateMaterial(MaterialFileDesc));

            rSphere.SetDetailFacet(Dt::SFacetCategory::Data, pModelActorFacet);

//             Dt::CScriptFacet* pScriptFacet = Dt::ScriptManager::CreateScript();
// 
//             pScriptFacet->SetScriptFile("scripts/move_circle.lua");
// 
//             rSphere.SetDetailFacet(Dt::SFacetCategory::Script, pScriptFacet);

            Dt::EntityManager::MarkEntityAsDirty(rSphere, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }
    
    void CLgLoadMapState::CreatePBRARScene()
    {
        // -----------------------------------------------------------------------------
        // Allocate a map
        // -----------------------------------------------------------------------------
        Dt::Map::AllocateMap(1, 1);

        // -----------------------------------------------------------------------------
        // Setup cameras
        // -----------------------------------------------------------------------------
        Dt::CEntity* pCameraEntity = nullptr;

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Actor;
            EntityDesc.m_EntityType     = Dt::SActorType::Camera;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 0.5f));
            pTransformationFacet->SetScale   (Base::Float3(1.0f));
            pTransformationFacet->SetRotation(Base::Float3(0.0f, 0.0f, 0.0f));

            Dt::CCameraActorFacet* pFacet = Dt::ActorManager::CreateCameraActor();

            pFacet->SetMainCamera(true);
            pFacet->SetNear(0.001f);

            rEntity.SetDetailFacet(Dt::SFacetCategory::Data, pFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);

            pCameraEntity = &rEntity;
        }

        // -----------------------------------------------------------------------------
        // Setup AR
        // -----------------------------------------------------------------------------
        Dt::STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = 1280;
        TextureDescriptor.m_NumberOfPixelsV  = 720;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_Format           = Dt::CTextureBase::R8G8B8A8_UBYTE;
        TextureDescriptor.m_Semantic         = Dt::CTextureBase::Diffuse;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pIdentifier      = 0;

        Dt::CTexture2D* pBackgroundTexture = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

        TextureDescriptor.m_NumberOfPixelsU = 512;
        TextureDescriptor.m_NumberOfPixelsV = 512;

        Dt::CTextureCube* pTextureCubemap = Dt::TextureManager::CreateCubeTexture(TextureDescriptor);

        Dt::TextureManager::CopyToTextureCube(pTextureCubemap, Dt::CTextureCube::Right,  Dt::TextureManager::CreateTexture2D(TextureDescriptor));
        Dt::TextureManager::CopyToTextureCube(pTextureCubemap, Dt::CTextureCube::Left,   Dt::TextureManager::CreateTexture2D(TextureDescriptor));
        Dt::TextureManager::CopyToTextureCube(pTextureCubemap, Dt::CTextureCube::Top,    Dt::TextureManager::CreateTexture2D(TextureDescriptor));
        Dt::TextureManager::CopyToTextureCube(pTextureCubemap, Dt::CTextureCube::Bottom, Dt::TextureManager::CreateTexture2D(TextureDescriptor));
        Dt::TextureManager::CopyToTextureCube(pTextureCubemap, Dt::CTextureCube::Front,  Dt::TextureManager::CreateTexture2D(TextureDescriptor));
        Dt::TextureManager::CopyToTextureCube(pTextureCubemap, Dt::CTextureCube::Back,   Dt::TextureManager::CreateTexture2D(TextureDescriptor));

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Plugin;
            EntityDesc.m_EntityType     = Dt::SPluginType::ARControlManager;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetWorldPosition(Base::Float3(0.0f, 0.0f, 0.0f));

            Dt::CARControllerPluginFacet* pFacet = Dt::PluginManager::CreateARControllerPlugin();

            pFacet->SetCameraEntity       (pCameraEntity);
            pFacet->SetCameraParameterFile("ar/configurations/logitech_para.dat");
            pFacet->SetDeviceNumber       (1);
            pFacet->SetOutputBackground   (pBackgroundTexture);
            pFacet->SetOutputCubemap      (pTextureCubemap);
            pFacet->SetDeviceType         (Dt::CARControllerPluginFacet::Webcam);
            pFacet->SetNumberOfMarker     (1);
            
            Dt::CARControllerPluginFacet::SMarker& rMarkerOne = pFacet->GetMarker(0);

            rMarkerOne.m_UID          = 0;
            rMarkerOne.m_Type         = Dt::CARControllerPluginFacet::SMarker::Square;
            rMarkerOne.m_WidthInMeter = 0.08f;
            rMarkerOne.m_PatternFile  = "ar/patterns/patt.hiro";

            rEntity.SetDetailFacet(Dt::SFacetCategory::Data, pFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
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

            Dt::CFXAAFXFacet* pEffectFacet = Dt::FXManager::CreateFXAAFX();

            rEffectEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEffectEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::FX;
            EntityDesc.m_EntityType     = Dt::SFXType::SSR;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEffectEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CSSRFXFacet* pEffectFacet = Dt::FXManager::CreateSSRFX();

            rEffectEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEffectEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Environment
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Skybox;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEnvironment = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CSkyboxFacet* pSkyboxFacet = Dt::LightManager::CreateSkybox();

            pSkyboxFacet->SetType     (Dt::CSkyboxFacet::ImageBackground);
            pSkyboxFacet->SetTexture  (pBackgroundTexture);
            pSkyboxFacet->SetCubemap  (pTextureCubemap);
            pSkyboxFacet->SetIntensity(20000.0f);

            rEnvironment.SetDetailFacet(Dt::SFacetCategory::Data, pSkyboxFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEnvironment, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Lights
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Sun;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rSunLight = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CSunLightFacet* pSunLightFacet = Dt::LightManager::CreateSunLight();

            pSunLightFacet->EnableTemperature(false);
            pSunLightFacet->SetColor         (Base::Float3(1.0f, 1.0f, 1.0f));
            pSunLightFacet->SetDirection     (Base::Float3(0.0f, 0.0f, -1.0f));
            pSunLightFacet->SetIntensity     (90700.0f);
            pSunLightFacet->SetTemperature   (0);
            pSunLightFacet->SetRefreshMode   (Dt::CSunLightFacet::Dynamic);

            pSunLightFacet->UpdateLightness();

            rSunLight.SetDetailFacet(Dt::SFacetCategory::Data, pSunLightFacet);

            Dt::EntityManager::MarkEntityAsDirty(rSunLight, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
        
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::GlobalProbe;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rGlobalProbeLight = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CGlobalProbeLightFacet* pGlobalProbeLightFacet = Dt::LightManager::CreateGlobalProbeLight();

            pGlobalProbeLightFacet->SetType     (Dt::CGlobalProbeLightFacet::Sky);
            pGlobalProbeLightFacet->SetQuality  (Dt::CGlobalProbeLightFacet::PX256);
            pGlobalProbeLightFacet->SetIntensity(1.0f);

            rGlobalProbeLight.SetDetailFacet(Dt::SFacetCategory::Data, pGlobalProbeLightFacet);

            Dt::EntityManager::MarkEntityAsDirty(rGlobalProbeLight, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // AR
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Actor;
            EntityDesc.m_EntityType     = Dt::SActorType::AR;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rSphere = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CTransformationFacet* pTransformationFacet = rSphere.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 0.0f));
            pTransformationFacet->SetScale   (Base::Float3(1000.0f));
            pTransformationFacet->SetRotation(Base::Float3(Base::DegreesToRadians(-90.0f), 0.0f, 0.0f));

            Dt::CARActorFacet* pModelActorFacet = Dt::ActorManager::CreateARActor();

            Dt::SModelFileDescriptor ModelFileDesc;

            ModelFileDesc.m_pFileName = "models/plane.obj";
            ModelFileDesc.m_GenFlag   = Dt::SGeneratorFlag::DefaultFlipUVs;

            Dt::SMaterialFileDescriptor MaterialFileDesc;

            MaterialFileDesc.m_pFileName = "materials/tests/background.mat";

            pModelActorFacet->SetModel(&Dt::ModelManager::CreateModel(ModelFileDesc));
            pModelActorFacet->SetMaterial(0, &Dt::MaterialManager::CreateMaterial(MaterialFileDesc));

            rSphere.SetDetailFacet(Dt::SFacetCategory::Data, pModelActorFacet);

            Dt::EntityManager::MarkEntityAsDirty(rSphere, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Entities
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Actor;
            EntityDesc.m_EntityType     = Dt::SActorType::Model;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rSphere = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CTransformationFacet* pTransformationFacet = rSphere.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.05f, 0.1f, 0.1f));
            pTransformationFacet->SetScale   (Base::Float3(0.006f));
            pTransformationFacet->SetRotation(Base::Float3(Base::DegreesToRadians(-90.0f), 0.0f, 0.0f));

            Dt::CModelActorFacet* pModelActorFacet = Dt::ActorManager::CreateModelActor();

            Dt::SModelFileDescriptor ModelFileDesc;

            ModelFileDesc.m_pFileName = "models/sphere.obj";
            ModelFileDesc.m_GenFlag   = Dt::SGeneratorFlag::DefaultFlipUVs;

            Dt::SMaterialFileDescriptor MaterialFileDesc;

            MaterialFileDesc.m_pFileName = "materials/naturals/metals/Gold_Worn_00.mat";

            pModelActorFacet->SetModel(&Dt::ModelManager::CreateModel(ModelFileDesc));
            pModelActorFacet->SetMaterial(0, &Dt::MaterialManager::CreateMaterial(MaterialFileDesc));

            rSphere.SetDetailFacet(Dt::SFacetCategory::Data, pModelActorFacet);

            Dt::EntityManager::MarkEntityAsDirty(rSphere, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Actor;
            EntityDesc.m_EntityType     = Dt::SActorType::Model;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rSphere = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CTransformationFacet* pTransformationFacet = rSphere.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 0.1f));
            pTransformationFacet->SetScale   (Base::Float3(0.0005f));
            pTransformationFacet->SetRotation(Base::Float3(Base::DegreesToRadians(35.0f), Base::DegreesToRadians(0.0f), Base::DegreesToRadians(10.0f)));

            Dt::CModelActorFacet* pModelActorFacet = Dt::ActorManager::CreateModelActor();

            Dt::SModelFileDescriptor ModelFileDesc;

            ModelFileDesc.m_pFileName = "models/plane.obj";
            ModelFileDesc.m_GenFlag   = Dt::SGeneratorFlag::DefaultFlipUVs;

            Dt::SMaterialFileDescriptor MaterialFileDesc;

            MaterialFileDesc.m_pFileName = "materials/naturals/metals/Chrome_Glossy_00.mat";

            pModelActorFacet->SetModel(&Dt::ModelManager::CreateModel(ModelFileDesc));
            pModelActorFacet->SetMaterial(0, &Dt::MaterialManager::CreateMaterial(MaterialFileDesc));

            rSphere.SetDetailFacet(Dt::SFacetCategory::Data, pModelActorFacet);

            Dt::EntityManager::MarkEntityAsDirty(rSphere, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CLgLoadMapState::CreatePBRSponzaScene()
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

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(14.0f, 6.0f, 10.0f));
            pTransformationFacet->SetScale   (Base::Float3(1.0f));
            pTransformationFacet->SetRotation(Base::Float3(0.0f, 0.0f, 0.0f));

            Dt::CCameraActorFacet* pFacet = Dt::ActorManager::CreateCameraActor();

            pFacet->SetMainCamera(true);

            rEntity.SetDetailFacet(Dt::SFacetCategory::Data, pFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
        
        // -----------------------------------------------------------------------------
        // Setup environment
        // -----------------------------------------------------------------------------
        {
            Dt::STextureDescriptor TextureDescriptor;

            TextureDescriptor.m_NumberOfPixelsU  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
            TextureDescriptor.m_NumberOfPixelsV  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
            TextureDescriptor.m_NumberOfPixelsW  = 1;
            TextureDescriptor.m_Format           = Dt::CTextureBase::R16G16B16_FLOAT;
            TextureDescriptor.m_Semantic         = Dt::CTextureBase::HDR;
            TextureDescriptor.m_pPixels          = 0;
            TextureDescriptor.m_pFileName        = "environments/PaperMill_E_3k.hdr";
            TextureDescriptor.m_pIdentifier      = 0;

            Dt::CTexture2D* pPanoramaTexture = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

            // -----------------------------------------------------------------------------

            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Skybox;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEnvironment = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CSkyboxFacet* pSkyboxFacet = Dt::LightManager::CreateSkybox();

            pSkyboxFacet->SetType     (Dt::CSkyboxFacet::Panorama);
            pSkyboxFacet->SetTexture  (pPanoramaTexture);
            pSkyboxFacet->SetIntensity(5000.0f);

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
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rSunLight = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CSunLightFacet* pSunLightFacet = Dt::LightManager::CreateSunLight();

            pSunLightFacet->EnableTemperature(false);
            pSunLightFacet->SetColor         (Base::Float3(1.0f, 1.0f, 1.0f));
            pSunLightFacet->SetDirection     (Base::Float3(0.0f, 0.0f, -1.0f));
            pSunLightFacet->SetIntensity     (90600.0f);
            pSunLightFacet->SetTemperature   (0);
            pSunLightFacet->SetRefreshMode   (Dt::CSunLightFacet::Static);

            pSunLightFacet->UpdateLightness();

            rSunLight.SetDetailFacet(Dt::SFacetCategory::Data, pSunLightFacet);

            Dt::EntityManager::MarkEntityAsDirty(rSunLight, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::GlobalProbe;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rGlobalProbeLight = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CGlobalProbeLightFacet* pGlobalProbeLightFacet = Dt::LightManager::CreateGlobalProbeLight();

            pGlobalProbeLightFacet->SetType     (Dt::CGlobalProbeLightFacet::Sky);
            pGlobalProbeLightFacet->SetQuality  (Dt::CGlobalProbeLightFacet::PX256);
            pGlobalProbeLightFacet->SetIntensity(1.0f);

            rGlobalProbeLight.SetDetailFacet(Dt::SFacetCategory::Data, pGlobalProbeLightFacet);

            Dt::EntityManager::MarkEntityAsDirty(rGlobalProbeLight, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Point;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rPointLight = Dt::EntityManager::CreateEntity(EntityDesc);

            rPointLight.SetWorldPosition(Base::Float3(14.0f, 6.0f, 4.0f));

            Dt::CPointLightFacet* pPointLightFacet = Dt::LightManager::CreatePointLight();

            pPointLightFacet->SetRefreshMode      (Dt::CPointLightFacet::Static);
            pPointLightFacet->SetShadowType       (Dt::CPointLightFacet::HardShadows);
            pPointLightFacet->SetShadowQuality    (Dt::CPointLightFacet::High);
            pPointLightFacet->EnableTemperature   (false);
            pPointLightFacet->SetColor            (Base::Float3(1.0f, 0.0f, 0.0f));
            pPointLightFacet->SetAttenuationRadius(10.0f);
            pPointLightFacet->SetInnerConeAngle   (Base::DegreesToRadians(45.0f));
            pPointLightFacet->SetOuterConeAngle   (Base::DegreesToRadians(90.0f));
            pPointLightFacet->SetDirection        (Base::Float3(-1.0f, -1.0f, -1.0f));
            pPointLightFacet->SetIntensity        (1200.0f);
            pPointLightFacet->SetTemperature      (0);

            pPointLightFacet->UpdateLightness();

            rPointLight.SetDetailFacet(Dt::SFacetCategory::Data, pPointLightFacet);

            Dt::EntityManager::MarkEntityAsDirty(rPointLight, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
        
        // -----------------------------------------------------------------------------        
        
        {
            Dt::SSceneDescriptor SceneDesc;

            SceneDesc.m_pSceneName = "scenes/sponza_crytek_pbr.obj";

            Dt::CEntity& rSceneEntity = Dt::EntityManager::CreateEntities(SceneDesc);

            Dt::CTransformationFacet* pTransformationFacet = rSceneEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(20.0f, 6.0f, 0.0f));
            pTransformationFacet->SetScale(Base::Float3(0.01f));
            pTransformationFacet->SetRotation(Base::Float3(Base::DegreesToRadians(-90.0f), 0.0f, 0.0f));

            Dt::EntityManager::MarkEntityAsDirty(rSceneEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
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