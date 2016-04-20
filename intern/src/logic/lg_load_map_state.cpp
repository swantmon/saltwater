
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
#include "data/data_texture_manager.h"
#include "data/data_transformation_facet.h"

#include "logic/lg_load_map_state.h"

#include "mr/mr_control_manager.h"
#include "mr/mr_marker_manager.h"
#include "mr/mr_tracker_manager.h"
#include "mr/mr_webcam_control.h"

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
        
        CreatePBRTestScene();
//        CreatePBRARScene();
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
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            rEntity.SetWorldPosition(Base::Float3(0.0f, 0.0f, 10.0f));

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

            rPlane.SetWorldPosition(Base::Float3(0.0f, 0.0f, 0.1f));

            Dt::CTransformationFacet* pTransformationFacet = rPlane.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 0.0f));
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

            rSphere.SetWorldPosition(Base::Float3(0.0f, 0.0f, 2.0f));

            Dt::CTransformationFacet* pTransformationFacet = rSphere.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 0.0f));
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

            Dt::EntityManager::MarkEntityAsDirty(rSphere, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }
    
    void CLgLoadMapState::CreatePBRARScene()
    {
        // -----------------------------------------------------------------------------
        // Set augmented reality on
        // -----------------------------------------------------------------------------
        MR::ControlManager::CreateControl   (MR::CControl::Webcam);
        MR::ControlManager::SetActiveControl(MR::CControl::Webcam);

        MR::SControlDescription  ControlSettings;

        ControlSettings.m_pCameraParameterFile = "ar/configurations/logitech_para.dat";
        ControlSettings.m_DeviceNumber         = 1;
        ControlSettings.m_OutputSize           = Base::Int2(1280, 720);

        MR::ControlManager::Start(ControlSettings);

        // -----------------------------------------------------------------------------

        MR::MarkerManager::OnStart();

        MR::SMarkerDescription  MarkerDescription;

        MarkerDescription.m_UserID       = 0;
        MarkerDescription.m_Type         = MR::SMarkerDescription::Square;
        MarkerDescription.m_WidthInMeter = 0.08f;
        MarkerDescription.m_pPatternFile = "ar/patterns/patt.hiro";

        MR::CMarkerPtr MarkerPtr = MR::MarkerManager::CreateMarker(MarkerDescription);

        // -----------------------------------------------------------------------------

        MR::TrackerManager::OnStart();

        MR::TrackerManager::RegisterMarker(MarkerPtr);

        // -----------------------------------------------------------------------------
        // Set our camera and position
        // -----------------------------------------------------------------------------
        Cam::ControlManager::SetActiveControl(Cam::CControl::GameControl);

        Cam::CGameControl& rARControl = static_cast<Cam::CGameControl&>(Cam::ControlManager::GetActiveControl());

        rARControl.SetProjectionMatrix(MR::ControlManager::GetActiveControl().GetProjectionMatrix());

        // -----------------------------------------------------------------------------
        // Allocate a map
        // -----------------------------------------------------------------------------
        Dt::Map::AllocateMap(1, 1);

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
            MR::CControl& rMRControl = MR::ControlManager::GetActiveControl();

            MR::CWebcamControl& rWebcamControl = static_cast<MR::CWebcamControl&>(rMRControl);

            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Skybox;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEnvironment = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CSkyboxFacet* pSkyboxFacet = Dt::LightManager::CreateSkybox();

            pSkyboxFacet->SetType     (Dt::CSkyboxFacet::ImageBackground);
            pSkyboxFacet->SetTexture  (rMRControl.GetConvertedFrame());
            pSkyboxFacet->SetCubemap  (rWebcamControl.GetCubemap());
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

        // -----------------------------------------------------------------------------
        // AR
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Actor;
            EntityDesc.m_EntityType     = Dt::SActorType::AR;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rSphere = Dt::EntityManager::CreateEntity(EntityDesc);

            rSphere.SetWorldPosition(Base::Float3(0.0f, 0.0f, 0.0f));

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

            rSphere.SetWorldPosition(Base::Float3(0.0f, 0.0f, 0.1f));

            Dt::CTransformationFacet* pTransformationFacet = rSphere.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 0.0f));
            pTransformationFacet->SetScale   (Base::Float3(0.005f));
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
    }
    
    // -----------------------------------------------------------------------------
    
    void CLgLoadMapState::CreatePBRSponzaScene()
    {
        // -----------------------------------------------------------------------------
        // Allocate a map
        // -----------------------------------------------------------------------------
        Dt::Map::AllocateMap(1, 1);
        
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

            rSceneEntity.SetWorldPosition(Base::Float3(20.0f, 6.0f, 0.0f));

            Dt::CTransformationFacet* pTransformationFacet = rSceneEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 0.0f));
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