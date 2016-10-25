
#include "editor/edit_precompiled.h"

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

        void OnNewMap(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoTransformation(Edit::CMessage& _rMessage);
        void OnDirtyEntity(Dt::CEntity* _pEntity);
    };
} // namespace

namespace
{
    CMapHelper::CMapHelper()
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
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewMap, EDIT_RECEIVE_MESSAGE(&CMapHelper::OnNewMap));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoTransformation, EDIT_RECEIVE_MESSAGE(&CMapHelper::OnRequestEntityInfoTransformation));
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

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc, 0);

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
            Dt::STextureDescriptor TextureDescriptor;

            TextureDescriptor.m_NumberOfPixelsU = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
            TextureDescriptor.m_NumberOfPixelsV = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_Format          = Dt::CTextureBase::R16G16B16_FLOAT;
            TextureDescriptor.m_Semantic        = Dt::CTextureBase::HDR;
            TextureDescriptor.m_pPixels         = 0;
            TextureDescriptor.m_pFileName       = "environments/Ridgecrest_Road_Ref.hdr";
            TextureDescriptor.m_pIdentifier     = 0;

            Dt::CTexture2D* pPanoramaTexture = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

            // -----------------------------------------------------------------------------

            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Skybox;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEnvironment = Dt::EntityManager::CreateEntity(EntityDesc, 1);

            Dt::CSkyboxFacet* pSkyboxFacet = Dt::LightManager::CreateSkybox();

            pSkyboxFacet->SetType(Dt::CSkyboxFacet::Panorama);
            pSkyboxFacet->SetTexture(pPanoramaTexture);
            pSkyboxFacet->SetIntensity(10000.0f);

            rEnvironment.SetDetailFacet(Dt::SFacetCategory::Data, pSkyboxFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEnvironment, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup entities
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Actor;
            EntityDesc.m_EntityType     = Dt::SActorType::Model;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rSphere = Dt::EntityManager::CreateEntity(EntityDesc, 2);

            Dt::CTransformationFacet* pTransformationFacet = rSphere.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f));
            pTransformationFacet->SetScale   (Base::Float3(1.0f));
            pTransformationFacet->SetRotation(Base::Float3(0.0f));

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

    void CMapHelper::OnRequestEntityInfoTransformation(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin();
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities; CurrentEntity = CurrentEntity.Next())
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            if (rCurrentEntity.GetID() == EntityID)
            {
                Edit::CMessage NewMessage;

                Dt::CTransformationFacet* pTransformationFacet = rCurrentEntity.GetTransformationFacet();

                if (pTransformationFacet)
                {
                    NewMessage.PutBool(true);

                    NewMessage.PutFloat(pTransformationFacet->GetPosition()[0]);
                    NewMessage.PutFloat(pTransformationFacet->GetPosition()[1]);
                    NewMessage.PutFloat(pTransformationFacet->GetPosition()[2]);

                    NewMessage.PutFloat(pTransformationFacet->GetScale()[0]);
                    NewMessage.PutFloat(pTransformationFacet->GetScale()[1]);
                    NewMessage.PutFloat(pTransformationFacet->GetScale()[2]);

                    NewMessage.PutFloat(pTransformationFacet->GetRotation()[0]);
                    NewMessage.PutFloat(pTransformationFacet->GetRotation()[1]);
                    NewMessage.PutFloat(pTransformationFacet->GetRotation()[2]);
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
    }

    // -----------------------------------------------------------------------------

    void CMapHelper::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        Edit::CMessage NewMessage;

        Dt::CEntity& rCurrentEntity = *_pEntity;

        NewMessage.PutInt(rCurrentEntity.GetID());
        NewMessage.PutInt(rCurrentEntity.GetCategory());

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::SceneGraphChanged, NewMessage);
    }
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