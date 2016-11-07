
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_actor_facet.h"
#include "data/data_actor_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_light_facet.h"
#include "data/data_light_manager.h"
#include "data/data_fx_facet.h"
#include "data/data_fx_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_map.h"
#include "data/data_material_manager.h"
#include "data/data_model_manager.h"
#include "data/data_plugin_facet.h"
#include "data/data_plugin_manager.h"
#include "data/data_script_facet.h"
#include "data/data_script_manager.h"
#include "data/data_texture_manager.h"
#include "data/data_transformation_facet.h"

#include "editor/edit_map_helper.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <windows.h>
#undef SendMessage

namespace
{
    class CMapHelper : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CMapHelper)

    public:

        CMapHelper();
        ~CMapHelper();

    public:

        void OnStart();
        void OnExit();

    private:

        Dt::CEntity* m_pLastRequestedEntity;

    private:

        void OnNewMap(Edit::CMessage& _rMessage);
        void OnNewEntityActor(Edit::CMessage& _rMessage);
        void OnNewEntityPointlight(Edit::CMessage& _rMessage);
        void OnNewEntitySun(Edit::CMessage& _rMessage);
        void OnNewEntityEnvironment(Edit::CMessage& _rMessage);
        void OnNewEntityGlobalProbe(Edit::CMessage& _rMessage);
        void OnNewEntityBloom(Edit::CMessage& _rMessage);
        void OnNewEntityDOF(Edit::CMessage& _rMessage);
        void OnNewEntityFXAA(Edit::CMessage& _rMessage);
        void OnNewEntitySSR(Edit::CMessage& _rMessage);
        void OnNewEntityVolumeFog(Edit::CMessage& _rMessage);

        void OnRemoveEntity(Edit::CMessage& _rMessage);

        void OnRequestEntityInfoFacets(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoEntity(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoTransformation(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoPointlight(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoSun(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoEnvironment(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoGlobalProbe(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoBloom(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoDOF(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoFXAA(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoSSR(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoVolumeFog(Edit::CMessage& _rMessage);

        void OnEntityInfoEntity(Edit::CMessage& _rMessage);
        void OnEntityInfoHierarchie(Edit::CMessage& _rMessage);
        void OnEntityInfoTransformation(Edit::CMessage& _rMessage);
        void OnEntityInfoPointlight(Edit::CMessage& _rMessage);
        void OnEntityInfoSun(Edit::CMessage& _rMessage);
        void OnEntityInfoEnvironment(Edit::CMessage& _rMessage);
        void OnEntityInfoGlobalProbe(Edit::CMessage& _rMessage);
        void OnEntityInfoBloom(Edit::CMessage& _rMessage);
        void OnEntityInfoDOF(Edit::CMessage& _rMessage);
        void OnEntityInfoFXAA(Edit::CMessage& _rMessage);
        void OnEntityInfoSSR(Edit::CMessage& _rMessage);
        void OnEntityInfoVolumeFog(Edit::CMessage& _rMessage);

        void OnDirtyEntity(Dt::CEntity* _pEntity);

        std::string CopyFileToAssets(const char* _pAssetFolder, const char* _pPathToFile);
    };
} // namespace

namespace
{
    CMapHelper::CMapHelper()
        : m_pLastRequestedEntity(nullptr)
    {
        
    }

    // -----------------------------------------------------------------------------

    CMapHelper::~CMapHelper()
    {

    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Entity
        // -----------------------------------------------------------------------------
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CMapHelper::OnDirtyEntity));

        // -----------------------------------------------------------------------------
        // Edit
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewMap                         , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnNewMap));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewEntityActor                 , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnNewEntityActor));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewEntityPointlight            , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnNewEntityPointlight));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewEntitySun                   , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnNewEntitySun));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewEntityEnvironment           , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnNewEntityEnvironment));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewEntityGlobalProbe           , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnNewEntityGlobalProbe));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewEntityBloom                 , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnNewEntityBloom));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewEntityDOF                   , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnNewEntityDOF));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewEntityFXAA                  , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnNewEntityFXAA));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewEntitySSR                   , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnNewEntitySSR));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewEntityVolumeFog             , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnNewEntityVolumeFog));

        Edit::MessageManager::Register(Edit::SGUIMessageType::RemoveEntity                   , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnRemoveEntity));

        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoFacets        , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnRequestEntityInfoFacets));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoEntity        , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnRequestEntityInfoEntity));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoTransformation, EDIT_RECEIVE_MESSAGE(&CMapHelper::OnRequestEntityInfoTransformation));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoPointlight    , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnRequestEntityInfoPointlight));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoSun           , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnRequestEntityInfoSun));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoEnvironment   , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnRequestEntityInfoEnvironment));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoGlobalProbe   , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnRequestEntityInfoGlobalProbe));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoBloom         , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnRequestEntityInfoBloom));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoDOF           , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnRequestEntityInfoDOF));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoFXAA          , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnRequestEntityInfoFXAA));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoSSR           , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnRequestEntityInfoSSR));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoVolumeFog     , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnRequestEntityInfoVolumeFog));

        Edit::MessageManager::Register(Edit::SGUIMessageType::EntityInfoEntity               , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnEntityInfoEntity));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EntityInfoHierarchie           , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnEntityInfoHierarchie));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EntityInfoTransformation       , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnEntityInfoTransformation));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EntityInfoPointlight           , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnEntityInfoPointlight));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EntityInfoSun                  , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnEntityInfoSun));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EntityInfoEnvironment          , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnEntityInfoEnvironment));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EntityInfoGlobalProbe          , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnEntityInfoGlobalProbe));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EntityInfoBloom                , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnEntityInfoBloom));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EntityInfoDOF                  , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnEntityInfoDOF));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EntityInfoFXAA                 , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnEntityInfoFXAA));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EntityInfoSSR                  , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnEntityInfoSSR));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EntityInfoVolumeFog            , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnEntityInfoVolumeFog));
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnNewMap(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Allocate a map
        // -----------------------------------------------------------------------------
        int MapX = _rMessage.GetInt();
        int MapY = _rMessage.GetInt();

        Dt::Map::AllocateMap(MapX, MapY);

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
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnNewEntityActor(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Create new entity
        // -----------------------------------------------------------------------------
        char        pTmp[512];
        std::string PathToFile;
        bool        HasModel;

        HasModel = _rMessage.GetBool();

        if (HasModel)
        {
            // -----------------------------------------------------------------------------
            // Model
            // -----------------------------------------------------------------------------
            Dt::SAssimpDescriptor ModelFileDesc;

            const char* pPathToFile = _rMessage.GetString(pTmp, 512);

            PathToFile = "models/" + CopyFileToAssets("../assets/models/", pPathToFile);

            ModelFileDesc.m_pPathToFile = PathToFile.c_str();

            Dt::CEntity& rNewEntity = Dt::EntityManager::CreateEntityFromFile(ModelFileDesc);

            // -----------------------------------------------------------------------------
            // Add model to map
            // -----------------------------------------------------------------------------
            Dt::EntityManager::MarkEntityAsDirty(rNewEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnNewEntityPointlight(Edit::CMessage& _rMessage)
    {
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Point;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rPointLight = Dt::EntityManager::CreateEntity(EntityDesc);

            // -----------------------------------------------------------------------------
            // Transformation
            // -----------------------------------------------------------------------------
            Dt::CTransformationFacet* pTransformationFacet = rPointLight.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f));
            pTransformationFacet->SetScale   (Base::Float3(1.0f));
            pTransformationFacet->SetRotation(Base::Float3(0.0f));

            Dt::CPointLightFacet* pPointLightFacet = Dt::LightManager::CreatePointLight();

            pPointLightFacet->SetRefreshMode      (Dt::CPointLightFacet::Static);
            pPointLightFacet->SetShadowType       (Dt::CPointLightFacet::HardShadows);
            pPointLightFacet->SetShadowQuality    (Dt::CPointLightFacet::High);
            pPointLightFacet->EnableTemperature   (false);
            pPointLightFacet->SetColor            (Base::Float3(1.0f, 1.0f, 1.0f));
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
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnNewEntitySun(Edit::CMessage& _rMessage)
    {
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Sun;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rSunLight = Dt::EntityManager::CreateEntity(EntityDesc);

            // -----------------------------------------------------------------------------
            // Transformation
            // -----------------------------------------------------------------------------
            Dt::CTransformationFacet* pTransformationFacet = rSunLight.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f));
            pTransformationFacet->SetScale   (Base::Float3(1.0f));
            pTransformationFacet->SetRotation(Base::Float3(0.0f));

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
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnNewEntityEnvironment(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Setup environment
        // -----------------------------------------------------------------------------
        {
            Dt::STextureDescriptor TextureDescriptor;

            TextureDescriptor.m_NumberOfPixelsU = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
            TextureDescriptor.m_NumberOfPixelsV = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_Format          = Dt::CTextureBase::R16G16B16_FLOAT;
            TextureDescriptor.m_Semantic        = Dt::CTextureBase::HDR;
            TextureDescriptor.m_pPixels         = 0;
            TextureDescriptor.m_pFileName       = "environments/PaperMill_E_3k.hdr";
            TextureDescriptor.m_pIdentifier     = 0;

            Dt::CTexture2D* pPanoramaTexture = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

            // -----------------------------------------------------------------------------

            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Skybox;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rEnvironment = Dt::EntityManager::CreateEntity(EntityDesc);

            // -----------------------------------------------------------------------------
            // Transformation
            // -----------------------------------------------------------------------------
            Dt::CTransformationFacet* pTransformationFacet = rEnvironment.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f));
            pTransformationFacet->SetScale   (Base::Float3(1.0f));
            pTransformationFacet->SetRotation(Base::Float3(0.0f));

            Dt::CSkyboxFacet* pSkyboxFacet = Dt::LightManager::CreateSkybox();

            pSkyboxFacet->SetType     (Dt::CSkyboxFacet::Panorama);
            pSkyboxFacet->SetTexture  (pPanoramaTexture);
            pSkyboxFacet->SetIntensity(5000.0f);

            rEnvironment.SetDetailFacet(Dt::SFacetCategory::Data, pSkyboxFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEnvironment, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnNewEntityGlobalProbe(Edit::CMessage& _rMessage)
    {
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::GlobalProbe;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy;

            Dt::CEntity& rGlobalProbeLight = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CGlobalProbeLightFacet* pGlobalProbeLightFacet = Dt::LightManager::CreateGlobalProbeLight();

            pGlobalProbeLightFacet->SetRefreshMode(Dt::CGlobalProbeLightFacet::Static);
            pGlobalProbeLightFacet->SetType       (Dt::CGlobalProbeLightFacet::Sky);
            pGlobalProbeLightFacet->SetQuality    (Dt::CGlobalProbeLightFacet::PX512);
            pGlobalProbeLightFacet->SetIntensity  (1.0f);

            rGlobalProbeLight.SetDetailFacet(Dt::SFacetCategory::Data, pGlobalProbeLightFacet);

            Dt::EntityManager::MarkEntityAsDirty(rGlobalProbeLight, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnNewEntityBloom(Edit::CMessage& _rMessage)
    {
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
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnNewEntityDOF(Edit::CMessage& _rMessage)
    {
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::FX;
            EntityDesc.m_EntityType     = Dt::SFXType::DOF;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEffectEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CDOFFXFacet* pEffectFacet = Dt::FXManager::CreateDOFFX();

            rEffectEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEffectEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnNewEntityFXAA(Edit::CMessage& _rMessage)
    {
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
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnNewEntitySSR(Edit::CMessage& _rMessage)
    {
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
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnNewEntityVolumeFog(Edit::CMessage& _rMessage)
    {
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::FX;
            EntityDesc.m_EntityType     = Dt::SFXType::VolumeFog;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEffectEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CVolumeFogFXFacet* pEffectFacet = Dt::FXManager::CreateVolumeFogFX();

            rEffectEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEffectEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnRemoveEntity(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyRemove | Dt::CEntity::DirtyDestroy);

        _rMessage.SetResult(100);

        m_pLastRequestedEntity = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnRequestEntityInfoFacets(Edit::CMessage& _rMessage)
    {
        m_pLastRequestedEntity = nullptr;
        
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        m_pLastRequestedEntity = &rCurrentEntity;

        if (m_pLastRequestedEntity != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(rCurrentEntity.GetCategory());
            NewMessage.PutInt(rCurrentEntity.GetType());

            NewMessage.PutBool(rCurrentEntity.GetTransformationFacet() != nullptr);
            NewMessage.PutBool(rCurrentEntity.GetHierarchyFacet()      != nullptr);

            NewMessage.PutBool(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data)    != nullptr);
            NewMessage.PutBool(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic) != nullptr);
            NewMessage.PutBool(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Script)  != nullptr);

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::EntityInfoFacets, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnRequestEntityInfoEntity(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Edit::CMessage NewMessage;

            NewMessage.PutInt(rCurrentEntity.GetID());

            NewMessage.PutInt(rCurrentEntity.GetCategory());

            NewMessage.PutInt(rCurrentEntity.GetType());

            if (rCurrentEntity.GetName().GetLength() > 0)
            {
                NewMessage.PutBool(true);

                NewMessage.PutString(rCurrentEntity.GetName().GetConst());
            }
            else
            {
                NewMessage.PutBool(false);
            }

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::EntityInfoEntity, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnRequestEntityInfoTransformation(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Edit::CMessage NewMessage;

            Dt::CTransformationFacet* pTransformationFacet = rCurrentEntity.GetTransformationFacet();

            if (pTransformationFacet)
            {
                NewMessage.PutBool(true);

                NewMessage.PutFloat(pTransformationFacet->GetPosition()[0]);
                NewMessage.PutFloat(pTransformationFacet->GetPosition()[1]);
                NewMessage.PutFloat(pTransformationFacet->GetPosition()[2]);

                NewMessage.PutFloat(Base::RadiansToDegree(pTransformationFacet->GetRotation()[0]));
                NewMessage.PutFloat(Base::RadiansToDegree(pTransformationFacet->GetRotation()[1]));
                NewMessage.PutFloat(Base::RadiansToDegree(pTransformationFacet->GetRotation()[2]));

                NewMessage.PutFloat(pTransformationFacet->GetScale()[0]);
                NewMessage.PutFloat(pTransformationFacet->GetScale()[1]);
                NewMessage.PutFloat(pTransformationFacet->GetScale()[2]);
            }
            else
            {
                NewMessage.PutBool(false);

                NewMessage.PutFloat(rCurrentEntity.GetWorldPosition()[0]);
                NewMessage.PutFloat(rCurrentEntity.GetWorldPosition()[1]);
                NewMessage.PutFloat(rCurrentEntity.GetWorldPosition()[2]);
            }

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::EntityInfoTransformation, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnRequestEntityInfoPointlight(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CPointLightFacet* pPointLightFacet = static_cast<Dt::CPointLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::Point && pPointLightFacet != nullptr)
            {
                Edit::CMessage NewMessage;

                NewMessage.PutInt(static_cast<int>(pPointLightFacet->HasTemperature()));
                NewMessage.PutFloat(pPointLightFacet->GetColor()[0]);
                NewMessage.PutFloat(pPointLightFacet->GetColor()[1]);
                NewMessage.PutFloat(pPointLightFacet->GetColor()[2]);
                NewMessage.PutFloat(pPointLightFacet->GetTemperature());
                NewMessage.PutFloat(pPointLightFacet->GetIntensity());
                NewMessage.PutFloat(pPointLightFacet->GetAttenuationRadius());
                NewMessage.PutFloat(Base::RadiansToDegree(pPointLightFacet->GetInnerConeAngle()));
                NewMessage.PutFloat(Base::RadiansToDegree(pPointLightFacet->GetOuterConeAngle()));
                NewMessage.PutFloat(pPointLightFacet->GetDirection()[0]);
                NewMessage.PutFloat(pPointLightFacet->GetDirection()[1]);
                NewMessage.PutFloat(pPointLightFacet->GetDirection()[2]);
                NewMessage.PutInt(static_cast<int>(pPointLightFacet->GetShadowType()));
                NewMessage.PutInt(static_cast<int>(pPointLightFacet->GetShadowQuality()));
                NewMessage.PutInt(static_cast<int>(pPointLightFacet->GetRefreshMode()));

                NewMessage.Reset();

                Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::EntityInfoPointlight, NewMessage);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnRequestEntityInfoSun(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CSunLightFacet* pLightFacet = static_cast<Dt::CSunLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::Sun && pLightFacet != nullptr)
            {
                Edit::CMessage NewMessage;

                NewMessage.PutInt(static_cast<int>(pLightFacet->HasTemperature()));
                NewMessage.PutFloat(pLightFacet->GetColor()[0]);
                NewMessage.PutFloat(pLightFacet->GetColor()[1]);
                NewMessage.PutFloat(pLightFacet->GetColor()[2]);
                NewMessage.PutFloat(pLightFacet->GetTemperature());
                NewMessage.PutFloat(pLightFacet->GetIntensity());
                NewMessage.PutFloat(pLightFacet->GetDirection()[0]);
                NewMessage.PutFloat(pLightFacet->GetDirection()[1]);
                NewMessage.PutFloat(pLightFacet->GetDirection()[2]);
                NewMessage.PutInt(static_cast<int>(pLightFacet->GetRefreshMode()));

                NewMessage.Reset();

                Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::EntityInfoSun, NewMessage);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnRequestEntityInfoEnvironment(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CSkyboxFacet* pLightFacet = static_cast<Dt::CSkyboxFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::Skybox && pLightFacet != nullptr)
            {
                Edit::CMessage NewMessage;

                NewMessage.PutInt   (static_cast<int>(pLightFacet->GetType()));
                NewMessage.PutString(pLightFacet->GetTexture()->GetFileName());
                NewMessage.PutFloat (pLightFacet->GetIntensity());
                
                NewMessage.Reset();

                Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::EntityInfoEnvironment, NewMessage);
            }
        }
    }

    // -----------------------------------------------------------------------------
    
    void CMapHelper::OnRequestEntityInfoGlobalProbe(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CGlobalProbeLightFacet* pLightFacet = static_cast<Dt::CGlobalProbeLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::GlobalProbe && pLightFacet != nullptr)
            {
                Edit::CMessage NewMessage;

                NewMessage.PutInt(pLightFacet->GetRefreshMode());
                NewMessage.PutInt(pLightFacet->GetType());
                NewMessage.PutInt(pLightFacet->GetQuality());
                NewMessage.PutFloat(pLightFacet->GetIntensity());

                NewMessage.Reset();

                Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::EntityInfoGlobalProbe, NewMessage);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnRequestEntityInfoBloom(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CBloomFXFacet* pFXFacet = static_cast<Dt::CBloomFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::Bloom && pFXFacet != nullptr)
            {
                Edit::CMessage NewMessage;

                NewMessage.PutFloat(pFXFacet->GetTint()[0]);
                NewMessage.PutFloat(pFXFacet->GetTint()[1]);
                NewMessage.PutFloat(pFXFacet->GetTint()[2]);

                NewMessage.PutFloat(pFXFacet->GetIntensity());
                NewMessage.PutFloat(pFXFacet->GetTreshhold());
                NewMessage.PutFloat(pFXFacet->GetExposureScale());
                NewMessage.PutInt(pFXFacet->GetSize());

                NewMessage.Reset();

                Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::EntityInfoBloom, NewMessage);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnRequestEntityInfoDOF(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CDOFFXFacet* pFXFacet = static_cast<Dt::CDOFFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::DOF && pFXFacet != nullptr)
            {
                Edit::CMessage NewMessage;

                NewMessage.PutFloat(pFXFacet->GetNearDistance());
                NewMessage.PutFloat(pFXFacet->GetFarDistance());
                NewMessage.PutFloat(pFXFacet->GetNearToFarRatio());
                NewMessage.PutFloat(pFXFacet->GetFadeUnToSmallBlur());
                NewMessage.PutFloat(pFXFacet->GetFadeSmallToMediumBlur());

                NewMessage.Reset();

                Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::EntityInfoDOF, NewMessage);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnRequestEntityInfoFXAA(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CFXAAFXFacet* pFXFacet = static_cast<Dt::CFXAAFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::FXAA && pFXFacet != nullptr)
            {
                Edit::CMessage NewMessage;

                NewMessage.PutFloat(pFXFacet->GetLuma()[0]);
                NewMessage.PutFloat(pFXFacet->GetLuma()[1]);
                NewMessage.PutFloat(pFXFacet->GetLuma()[2]);

                NewMessage.Reset();

                Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::EntityInfoFXAA, NewMessage);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnRequestEntityInfoSSR(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CSSRFXFacet* pFXFacet = static_cast<Dt::CSSRFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::SSR && pFXFacet != nullptr)
            {
                Edit::CMessage NewMessage;

                NewMessage.PutFloat(pFXFacet->GetIntensity());
                NewMessage.PutFloat(pFXFacet->GetRoughnessMask());
                NewMessage.PutFloat(pFXFacet->GetDistance());
                NewMessage.PutBool (pFXFacet->GetUseLastFrame());

                NewMessage.Reset();

                Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::EntityInfoSSR, NewMessage);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnRequestEntityInfoVolumeFog(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CVolumeFogFXFacet* pFXFacet = static_cast<Dt::CVolumeFogFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::VolumeFog && pFXFacet != nullptr)
            {
                Edit::CMessage NewMessage;

                NewMessage.PutFloat(pFXFacet->GetWindDirection()[0]);
                NewMessage.PutFloat(pFXFacet->GetWindDirection()[1]);
                NewMessage.PutFloat(pFXFacet->GetWindDirection()[2]);

                NewMessage.PutFloat(pFXFacet->GetFogColor()[0]);
                NewMessage.PutFloat(pFXFacet->GetFogColor()[1]);
                NewMessage.PutFloat(pFXFacet->GetFogColor()[2]);

                NewMessage.PutFloat(pFXFacet->GetFrustumDepthInMeter());
                NewMessage.PutFloat(pFXFacet->GetShadowIntensity());
                NewMessage.PutFloat(pFXFacet->GetScatteringCoefficient());
                NewMessage.PutFloat(pFXFacet->GetAbsorptionCoefficient());
                NewMessage.PutFloat(pFXFacet->GetDensityLevel());
                NewMessage.PutFloat(pFXFacet->GetDensityAttenuation());

                NewMessage.Reset();

                Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::EntityInfoVolumeFog, NewMessage);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnEntityInfoEntity(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            bool HasName = _rMessage.GetBool();

            if (HasName)
            {
                char NewEntityName[256];

                _rMessage.GetString(NewEntityName, 256);

                rCurrentEntity.SetName(NewEntityName);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnEntityInfoHierarchie(Edit::CMessage& _rMessage)
    {
        int EntityIDSource      = _rMessage.GetInt();
        int EntityIDDestination = _rMessage.GetInt();

        assert(EntityIDSource != -1);

        // -----------------------------------------------------------------------------
        // Get source entity and hierarchy facet
        // -----------------------------------------------------------------------------
        Dt::CEntity& rSourceEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityIDSource));

        rSourceEntity.Detach();

        if (EntityIDDestination != -1)
        {
            Dt::CEntity& rDestinationEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityIDDestination));

            rDestinationEntity.Attach(rSourceEntity);
        }

        Dt::EntityManager::MarkEntityAsDirty(rSourceEntity, Dt::CEntity::DirtyMove);
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnEntityInfoTransformation(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CTransformationFacet* pTransformationFacet = rCurrentEntity.GetTransformationFacet();

            if (pTransformationFacet)
            {
                Base::Float3 Position(_rMessage.GetFloat(), _rMessage.GetFloat(), _rMessage.GetFloat());
                Base::Float3 Rotation(Base::DegreesToRadians(_rMessage.GetFloat()), Base::DegreesToRadians(_rMessage.GetFloat()), Base::DegreesToRadians(_rMessage.GetFloat()));
                Base::Float3 Scale   (_rMessage.GetFloat(), _rMessage.GetFloat(), _rMessage.GetFloat());

                pTransformationFacet->SetPosition(Position);
                pTransformationFacet->SetScale(Scale);
                pTransformationFacet->SetRotation(Rotation);
            }
            else
            {
                Base::Float3 Position(_rMessage.GetFloat(), _rMessage.GetFloat(), _rMessage.GetFloat());

                rCurrentEntity.SetWorldPosition(Position);
            }

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyMove);
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnEntityInfoPointlight(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CPointLightFacet* pPointLightFacet = static_cast<Dt::CPointLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::Point && pPointLightFacet != nullptr)
            {
                // -----------------------------------------------------------------------------
                // Read values
                // -----------------------------------------------------------------------------
                int ColorMode = _rMessage.GetInt();

                Base::Float3 Color = Base::Float3(_rMessage.GetFloat(), _rMessage.GetFloat(), _rMessage.GetFloat());

                float Temperature       = _rMessage.GetFloat();
                float Intensity         = _rMessage.GetFloat();
                float AttenuationRadius = _rMessage.GetFloat();
                float InnerConeAngle    = Base::DegreesToRadians(_rMessage.GetFloat());
                float OuterConeAngle    = Base::DegreesToRadians(_rMessage.GetFloat());

                Base::Float3 Direction = Base::Float3(_rMessage.GetFloat(), _rMessage.GetFloat(), _rMessage.GetFloat());

                int ShadowType    = _rMessage.GetInt();
                int ShadowQuality = _rMessage.GetInt();
                int ShadowRefresh = _rMessage.GetInt();

                // -----------------------------------------------------------------------------
                // Set values
                // -----------------------------------------------------------------------------
                pPointLightFacet->EnableTemperature   (ColorMode == 1);
                pPointLightFacet->SetColor            (Color);
                pPointLightFacet->SetTemperature      (Temperature);
                pPointLightFacet->SetIntensity        (Intensity);
                pPointLightFacet->SetAttenuationRadius(AttenuationRadius);
                pPointLightFacet->SetInnerConeAngle   (InnerConeAngle);
                pPointLightFacet->SetOuterConeAngle   (OuterConeAngle);
                pPointLightFacet->SetDirection        (Direction);
                pPointLightFacet->SetShadowType       (static_cast<Dt::CPointLightFacet::EShadowType>(ShadowType));
                pPointLightFacet->SetShadowQuality    (static_cast<Dt::CPointLightFacet::EShadowQuality>(ShadowQuality));
                pPointLightFacet->SetRefreshMode      (static_cast<Dt::CPointLightFacet::ERefreshMode>(ShadowRefresh));
            
                pPointLightFacet->UpdateLightness();

                Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnEntityInfoSun(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CSunLightFacet* pLightFacet = static_cast<Dt::CSunLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::Sun && pLightFacet != nullptr)
            {
                // -----------------------------------------------------------------------------
                // Read values
                // -----------------------------------------------------------------------------
                int ColorMode = _rMessage.GetInt();

                Base::Float3 Color = Base::Float3(_rMessage.GetFloat(), _rMessage.GetFloat(), _rMessage.GetFloat());

                float Temperature = _rMessage.GetFloat();
                float Intensity = _rMessage.GetFloat();

                Base::Float3 Direction = Base::Float3(_rMessage.GetFloat(), _rMessage.GetFloat(), _rMessage.GetFloat());

                int ShadowRefresh = _rMessage.GetInt();

                // -----------------------------------------------------------------------------
                // Set values
                // -----------------------------------------------------------------------------
                pLightFacet->EnableTemperature(ColorMode == 1);
                pLightFacet->SetColor(Color);
                pLightFacet->SetTemperature(Temperature);
                pLightFacet->SetIntensity(Intensity);
                pLightFacet->SetDirection(Direction);
                pLightFacet->SetRefreshMode(static_cast<Dt::CSunLightFacet::ERefreshMode>(ShadowRefresh));

                pLightFacet->UpdateLightness();

                Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnEntityInfoEnvironment(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CSkyboxFacet* pLightFacet = static_cast<Dt::CSkyboxFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::Skybox && pLightFacet != nullptr)
            {
                // -----------------------------------------------------------------------------
                // Read values
                // -----------------------------------------------------------------------------
                int Type = _rMessage.GetInt();

                char pTemp[256];

                const char* pTexture = _rMessage.GetString(pTemp, 256);

                float Intensity = _rMessage.GetFloat();

                // -----------------------------------------------------------------------------
                // Set values
                // -----------------------------------------------------------------------------
                pLightFacet->SetType     (static_cast<Dt::CSkyboxFacet::EType>(Type));
                pLightFacet->SetIntensity(Intensity);

                const char* pCurrentTexture = pLightFacet->GetTexture()->GetFileName();
                
                if (strcmp(pCurrentTexture, pTexture))
                {
                    Dt::STextureDescriptor TextureDescriptor;

                    TextureDescriptor.m_NumberOfPixelsU = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
                    TextureDescriptor.m_NumberOfPixelsV = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
                    TextureDescriptor.m_NumberOfPixelsW = 1;
                    TextureDescriptor.m_Format          = Dt::CTextureBase::R16G16B16_FLOAT;
                    TextureDescriptor.m_Semantic        = Dt::CTextureBase::HDR;
                    TextureDescriptor.m_pPixels         = 0;
                    TextureDescriptor.m_pFileName       = pTexture;
                    TextureDescriptor.m_pIdentifier     = 0;

                    Dt::CTexture2D* pPanoramaTexture = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

                    pLightFacet->SetTexture(pPanoramaTexture);
                }

                Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnEntityInfoGlobalProbe(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CGlobalProbeLightFacet* pLightFacet = static_cast<Dt::CGlobalProbeLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::GlobalProbe && pLightFacet != nullptr)
            {
                // -----------------------------------------------------------------------------
                // Read values
                // -----------------------------------------------------------------------------
                int RefreshMode = _rMessage.GetInt();

                int Type = _rMessage.GetInt();

                int Quality = _rMessage.GetInt();

                float Intensity = _rMessage.GetFloat();

                // -----------------------------------------------------------------------------
                // Set values
                // -----------------------------------------------------------------------------
                pLightFacet->SetRefreshMode(static_cast<Dt::CGlobalProbeLightFacet::ERefreshMode>(RefreshMode));

                pLightFacet->SetType(static_cast<Dt::CGlobalProbeLightFacet::EType>(Type));

                pLightFacet->SetQuality(static_cast<Dt::CGlobalProbeLightFacet::EQuality>(Quality));

                pLightFacet->SetIntensity(Intensity);

                Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
            }
        }
    }

    void CMapHelper::OnEntityInfoBloom(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CBloomFXFacet* pFXFacet = static_cast<Dt::CBloomFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::Bloom && pFXFacet != nullptr)
            {
                // -----------------------------------------------------------------------------
                // Read values
                // -----------------------------------------------------------------------------
                Base::Float4 Color = Base::Float4(_rMessage.GetFloat(), _rMessage.GetFloat(), _rMessage.GetFloat(), 1.0f);

                float Intensity     = _rMessage.GetFloat();
                float Treshhold     = _rMessage.GetFloat();
                float ExposureScale = _rMessage.GetFloat();
                int   Size          = _rMessage.GetInt();

                // -----------------------------------------------------------------------------
                // Set values
                // -----------------------------------------------------------------------------
                pFXFacet->SetTint(Color);

                pFXFacet->SetIntensity(Intensity);

                pFXFacet->SetTreshhold(Treshhold);

                pFXFacet->SetExposureScale(ExposureScale);

                pFXFacet->SetSize(Size);

                pFXFacet->UpdateEffect();

                Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnEntityInfoDOF(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CDOFFXFacet* pFXFacet = static_cast<Dt::CDOFFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::DOF && pFXFacet != nullptr)
            {
                // -----------------------------------------------------------------------------
                // Read values
                // -----------------------------------------------------------------------------               
                float Near            = _rMessage.GetFloat();
                float Far             = _rMessage.GetFloat();
                float NearToFarRatio  = _rMessage.GetFloat();
                float FadeUnSmall     = _rMessage.GetFloat();
                float FadeSmallMedium = _rMessage.GetFloat();

                // -----------------------------------------------------------------------------
                // Set values
                // -----------------------------------------------------------------------------
                pFXFacet->SetNearDistance(Near);

                pFXFacet->SetFarDistance(Far);

                pFXFacet->SetNearToFarRatio(NearToFarRatio);

                pFXFacet->SetFadeUnToSmallBlur(FadeUnSmall);

                pFXFacet->SetFadeSmallToMediumBlur(FadeSmallMedium);

                pFXFacet->UpdateEffect();

                Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnEntityInfoFXAA(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CFXAAFXFacet* pFXFacet = static_cast<Dt::CFXAAFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::FXAA && pFXFacet != nullptr)
            {
                // -----------------------------------------------------------------------------
                // Read values
                // -----------------------------------------------------------------------------               
                Base::Float3 Color = Base::Float3(_rMessage.GetFloat(), _rMessage.GetFloat(), _rMessage.GetFloat());

                // -----------------------------------------------------------------------------
                // Set values
                // -----------------------------------------------------------------------------
                pFXFacet->SetLuma(Color);

                pFXFacet->UpdateEffect();

                Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnEntityInfoSSR(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CSSRFXFacet* pFXFacet = static_cast<Dt::CSSRFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::SSR && pFXFacet != nullptr)
            {
                // -----------------------------------------------------------------------------
                // Read values
                // -----------------------------------------------------------------------------               
                float Intensity            = _rMessage.GetFloat();
                float RoughnessMask        = _rMessage.GetFloat();
                float Distance             = _rMessage.GetFloat();
                bool  UseDoubleReflections = _rMessage.GetBool();

                // -----------------------------------------------------------------------------
                // Set values
                // -----------------------------------------------------------------------------
                pFXFacet->SetIntensity(Intensity);

                pFXFacet->SetRoughnessMask(RoughnessMask);

                pFXFacet->SetDistance(Distance);

                pFXFacet->SetUseLastFrame(UseDoubleReflections);

                pFXFacet->UpdateEffect();

                Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnEntityInfoVolumeFog(Edit::CMessage& _rMessage)
    {
        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

            Dt::CVolumeFogFXFacet* pFXFacet = static_cast<Dt::CVolumeFogFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

            if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::VolumeFog && pFXFacet != nullptr)
            {
                // -----------------------------------------------------------------------------
                // Read values
                // -----------------------------------------------------------------------------
                Base::Float4 WindDirection = Base::Float4(_rMessage.GetFloat(), _rMessage.GetFloat(), _rMessage.GetFloat(), 0.0f);

                Base::Float4 Color = Base::Float4(_rMessage.GetFloat(), _rMessage.GetFloat(), _rMessage.GetFloat(), 1.0f);

                float FrustumDepth       = _rMessage.GetFloat();
                float ShadowIntensity    = _rMessage.GetFloat();
                float ScatteringCoeff    = _rMessage.GetFloat();
                float AbsorptionCoeff    = _rMessage.GetFloat();
                float DensityLevel       = _rMessage.GetFloat();
                float DensityAttenuation = _rMessage.GetFloat();

                // -----------------------------------------------------------------------------
                // Set values
                // -----------------------------------------------------------------------------
                pFXFacet->SetWindDirection(WindDirection);

                pFXFacet->SetFogColor(Color);

                pFXFacet->SetFrustumDepthInMeter(FrustumDepth);

                pFXFacet->SetScatteringCoefficient(ScatteringCoeff);

                pFXFacet->SetAbsorptionCoefficient(AbsorptionCoeff);

                pFXFacet->SetDensityLevel(DensityLevel);

                pFXFacet->SetDensityAttenuation(DensityAttenuation);

                Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        if ((_pEntity->GetDirtyFlags() & Dt::CEntity::DirtyAdd) == Dt::CEntity::DirtyAdd)
        {
            Edit::CMessage NewMessage;

            Dt::CEntity& rCurrentEntity = *_pEntity;

            // -----------------------------------------------------------------------------
            // ID
            // -----------------------------------------------------------------------------
            NewMessage.PutInt(rCurrentEntity.GetID());

            // -----------------------------------------------------------------------------
            // Name
            // -----------------------------------------------------------------------------
            Base::CharString& rEntityName = rCurrentEntity.GetName();

            if (rEntityName.GetLength() > 0)
            {
                NewMessage.PutBool(true);
                NewMessage.PutString(rEntityName.GetConst());
            }
            else
            {
                NewMessage.PutBool(false);
            }

            // -----------------------------------------------------------------------------
            // Hierarchy
            // -----------------------------------------------------------------------------
            Dt::CHierarchyFacet* pHierarchyFacet = rCurrentEntity.GetHierarchyFacet();

            if (pHierarchyFacet)
            {
                NewMessage.PutBool(true);

                Dt::CEntity* pParentEntity = pHierarchyFacet->GetParent();

                if (pParentEntity)
                {
                    NewMessage.PutBool(true);

                    NewMessage.PutInt(pParentEntity->GetID());
                }
                else
                {
                    NewMessage.PutBool(false);
                }
            }
            else
            {
                NewMessage.PutBool(false);
            }


            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::SceneGraphChanged, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    std::string CMapHelper::CopyFileToAssets(const char* _pAssetFolder, const char* _pPathToFile)
    {
        char pDrive[4];
        char pDirectory[512];
        char pFilename[32];
        char pExtension[12];

        std::string FileExtension;
        std::string RelativePathToModel;

        _splitpath_s(_pPathToFile, pDrive, 4, pDirectory, 512, pFilename, 32, pExtension, 12);

        FileExtension = std::string(pFilename) + std::string(pExtension);
        RelativePathToModel = std::string(_pAssetFolder) + FileExtension;

        CopyFileA(_pPathToFile, RelativePathToModel.c_str(), true);

        return FileExtension.c_str();
    };
} // namespace

namespace Edit
{
namespace Helper
{
namespace Map
{
    void OnStart()
    {
        CMapHelper::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CMapHelper::GetInstance().OnExit();
    }
} // namespace Map
} // namespace Helper
} // namespace Edit