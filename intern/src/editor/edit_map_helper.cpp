
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
        unsigned int m_EntityID;

    private:

        void OnNewMap(Edit::CMessage& _rMessage);
        void OnNewEntityActor(Edit::CMessage& _rMessage);
        void OnNewLightDirectional(Edit::CMessage& _rMessage);
        void OnNewLightPoint(Edit::CMessage& _rMessage);
        void OnNewLightEnvironment(Edit::CMessage& _rMessage);
        void OnNewLightReflection(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoFacets(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoTransformation(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoPointlight(Edit::CMessage& _rMessage);
        void OnEntityInfoTransformation(Edit::CMessage& _rMessage);
        void OnEntityInfoPointlight(Edit::CMessage& _rMessage);
        void OnDirtyEntity(Dt::CEntity* _pEntity);

        std::string CopyFileToAssets(const char* _pAssetFolder, const char* _pPathToFile);
    };
} // namespace

namespace
{
    CMapHelper::CMapHelper()
        : m_pLastRequestedEntity(nullptr)
        , m_EntityID            (0)
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
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewLightDirectional            , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnNewLightDirectional));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewLightPoint                  , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnNewLightPoint));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewLightEnvironment            , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnNewLightEnvironment));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewLightReflection             , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnNewLightReflection));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoFacets        , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnRequestEntityInfoFacets));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoTransformation, EDIT_RECEIVE_MESSAGE(&CMapHelper::OnRequestEntityInfoTransformation));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoPointlight    , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnRequestEntityInfoPointlight));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EntityInfoTransformation       , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnEntityInfoTransformation));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EntityInfoPointlight           , EDIT_RECEIVE_MESSAGE(&CMapHelper::OnEntityInfoPointlight));
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

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc, m_EntityID);

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

        ++m_EntityID;
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
            // Entity
            // -----------------------------------------------------------------------------
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Actor;
            EntityDesc.m_EntityType     = Dt::SActorType::Model;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rNewActorModel = Dt::EntityManager::CreateEntity(EntityDesc, m_EntityID);

            // -----------------------------------------------------------------------------
            // Transformation
            // -----------------------------------------------------------------------------
            Dt::CTransformationFacet* pTransformationFacet = rNewActorModel.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f));
            pTransformationFacet->SetScale   (Base::Float3(1.0f));
            pTransformationFacet->SetRotation(Base::Float3(0.0f));

            Dt::CModelActorFacet* pModelActorFacet = Dt::ActorManager::CreateModelActor();

            // -----------------------------------------------------------------------------
            // Model
            // -----------------------------------------------------------------------------
            Dt::SModelFileDescriptor ModelFileDesc;

            const char* pPathToFile = _rMessage.GetString(pTmp, 512);

            PathToFile = "models/" + CopyFileToAssets("../assets/models/", pPathToFile);

            ModelFileDesc.m_pFileName = PathToFile.c_str();
            ModelFileDesc.m_GenFlag   = Dt::SGeneratorFlag::DefaultFlipUVs;

            pModelActorFacet->SetModel(&Dt::ModelManager::CreateModel(ModelFileDesc));

            // -----------------------------------------------------------------------------
            // Material
            // -----------------------------------------------------------------------------
            bool HasMaterial = _rMessage.GetBool();

            if (HasMaterial)
            {
                Dt::SMaterialFileDescriptor MaterialFileDesc;

                const char* pPathToFile = _rMessage.GetString(pTmp, 512);

                PathToFile = "materials/" + CopyFileToAssets("../assets/materials/", pPathToFile);

                MaterialFileDesc.m_pFileName = PathToFile.c_str();

                pModelActorFacet->SetMaterial(0, &Dt::MaterialManager::CreateMaterial(MaterialFileDesc));
            }

            rNewActorModel.SetDetailFacet(Dt::SFacetCategory::Data, pModelActorFacet);

            // -----------------------------------------------------------------------------
            // Add model to map
            // -----------------------------------------------------------------------------
            Dt::EntityManager::MarkEntityAsDirty(rNewActorModel, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Prepare next entity
        // -----------------------------------------------------------------------------
        ++m_EntityID;
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnNewLightDirectional(Edit::CMessage& _rMessage)
    {
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Sun;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rSunLight = Dt::EntityManager::CreateEntity(EntityDesc, m_EntityID);

            // -----------------------------------------------------------------------------
            // Transformation
            // -----------------------------------------------------------------------------
            Dt::CTransformationFacet* pTransformationFacet = rSunLight.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f));
            pTransformationFacet->SetScale   (Base::Float3(1.0f));
            pTransformationFacet->SetRotation(Base::Float3(0.0f));

            Dt::CSunLightFacet* pSunLightFacet = Dt::LightManager::CreateSunLight();

            pSunLightFacet->EnableTemperature(false);
            pSunLightFacet->SetColor(Base::Float3(1.0f, 1.0f, 1.0f));
            pSunLightFacet->SetDirection(Base::Float3(0.0f, 0.0f, -1.0f));
            pSunLightFacet->SetIntensity(90600.0f);
            pSunLightFacet->SetTemperature(0);
            pSunLightFacet->SetRefreshMode(Dt::CSunLightFacet::Dynamic);

            pSunLightFacet->UpdateLightness();

            rSunLight.SetDetailFacet(Dt::SFacetCategory::Data, pSunLightFacet);

            Dt::EntityManager::MarkEntityAsDirty(rSunLight, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        ++m_EntityID;
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnNewLightPoint(Edit::CMessage& _rMessage)
    {
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Point;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rPointLight = Dt::EntityManager::CreateEntity(EntityDesc, m_EntityID);

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

        ++m_EntityID;
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnNewLightEnvironment(Edit::CMessage& _rMessage)
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

            Dt::CEntity& rEnvironment = Dt::EntityManager::CreateEntity(EntityDesc, m_EntityID);

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

        ++m_EntityID;
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnNewLightReflection(Edit::CMessage& _rMessage)
    {
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::GlobalProbe;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy;

            Dt::CEntity& rGlobalProbeLight = Dt::EntityManager::CreateEntity(EntityDesc, m_EntityID);

            Dt::CGlobalProbeLightFacet* pGlobalProbeLightFacet = Dt::LightManager::CreateGlobalProbeLight();

            pGlobalProbeLightFacet->SetType(Dt::CGlobalProbeLightFacet::Sky);
            pGlobalProbeLightFacet->SetQuality(Dt::CGlobalProbeLightFacet::PX512);
            pGlobalProbeLightFacet->SetIntensity(1.0f);

            rGlobalProbeLight.SetDetailFacet(Dt::SFacetCategory::Data, pGlobalProbeLightFacet);

            Dt::EntityManager::MarkEntityAsDirty(rGlobalProbeLight, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        ++m_EntityID;
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnRequestEntityInfoFacets(Edit::CMessage& _rMessage)
    {
        m_pLastRequestedEntity = nullptr;
        
        int EntityID = _rMessage.GetInt();

        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin();
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities; CurrentEntity = CurrentEntity.Next())
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            if (rCurrentEntity.GetID() == EntityID)
            {
                m_pLastRequestedEntity = &rCurrentEntity;

                break;
            }
        }

        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

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

    void CMapHelper::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        if ((_pEntity->GetDirtyFlags() & Dt::CEntity::DirtyAdd) == Dt::CEntity::DirtyAdd)
        {
            Edit::CMessage NewMessage;

            Dt::CEntity& rCurrentEntity = *_pEntity;

            NewMessage.PutInt(rCurrentEntity.GetID());
            NewMessage.PutInt(rCurrentEntity.GetCategory());

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