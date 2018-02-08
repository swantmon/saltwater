
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_actor_type.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_map.h"
#include "data/data_model_manager.h"
#include "data/data_transformation_facet.h"

#include "editor/edit_entity_helper.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

namespace
{
    class CEntityHelper : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CEntityHelper)

    public:

        CEntityHelper();
        ~CEntityHelper();

    public:

        void OnStart();
        void OnExit();

    private:

        void OnNewEntity(Edit::CMessage& _rMessage);
        void OnLoadEntity(Edit::CMessage& _rMessage);
        void OnCreateEntity(Edit::CMessage& _rMessage);
        void OnAddEntity(Edit::CMessage& _rMessage);
        void OnRemoveEntity(Edit::CMessage& _rMessage);
        void OnDestroyEntity(Edit::CMessage& _rMessage);

        void OnRequestInfoFacets(Edit::CMessage& _rMessage);
        void OnRequestInfoEntity(Edit::CMessage& _rMessage);
        void OnRequestInfoTransformation(Edit::CMessage& _rMessage);

        void OnInfoEntity(Edit::CMessage& _rMessage);
        void OnInfoHierarchie(Edit::CMessage& _rMessage);
        void OnInfoTransformation(Edit::CMessage& _rMessage);

        void OnDirtyEntity(Dt::CEntity* _pEntity);
    };
} // namespace

namespace
{
    CEntityHelper::CEntityHelper()
    {
        
    }

    // -----------------------------------------------------------------------------

    CEntityHelper::~CEntityHelper()
    {

    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Entity
        // -----------------------------------------------------------------------------
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CEntityHelper::OnDirtyEntity));

        // -----------------------------------------------------------------------------
        // Edit
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SGUIMessageType::Entity_New    , EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnNewEntity));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Entity_Load   , EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnLoadEntity));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Entity_Create , EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnCreateEntity));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Entity_Add    , EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnAddEntity));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Entity_Remove , EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnRemoveEntity));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Entity_Destroy, EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnDestroyEntity));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Entity_Facets_Info        , EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnRequestInfoFacets));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Entity_Info               , EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnRequestInfoEntity));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Entity_Transformation_Info, EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnRequestInfoTransformation));
        
        Edit::MessageManager::Register(Edit::SGUIMessageType::Entity_Info_Update          , EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnInfoEntity));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Entity_Hierarchy_Update     , EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnInfoHierarchie));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Entity_Transformation_Update, EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnInfoTransformation));
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnNewEntity(Edit::CMessage& _rMessage)
    {
        Dt::SEntityDescriptor EntityDesc;

        EntityDesc.m_EntityCategory = 0;
        EntityDesc.m_EntityType     = 0;
        EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

        Dt::CEntity& rNewEntity = Dt::EntityManager::CreateEntity(EntityDesc);

        Dt::CTransformationFacet* pTransformationFacet = rNewEntity.GetTransformationFacet();

        pTransformationFacet->SetPosition(glm::vec3(0.0f));
        pTransformationFacet->SetScale(glm::vec3(1.0f));
        pTransformationFacet->SetRotation(glm::vec3(0.0f));

        _rMessage.SetResult(rNewEntity.GetID());
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnLoadEntity(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Create new entity
        // -----------------------------------------------------------------------------
        char pTmp[512];

        // -----------------------------------------------------------------------------
        // Model
        // -----------------------------------------------------------------------------
        Dt::SModelFileDescriptor ModelFileDesc;

        const char* pPathToFile = _rMessage.GetString(pTmp, 512);

        ModelFileDesc.m_pFileName = pPathToFile;
        ModelFileDesc.m_GenFlag = Dt::SGeneratorFlag::Default;

        Dt::CModel& rModel = Dt::ModelManager::CreateModel(ModelFileDesc);

        Dt::CEntity& rNewEntity = Dt::EntityManager::CreateEntityFromModel(rModel);

        rNewEntity.SetName("New Prefab");

        Dt::EntityManager::MarkEntityAsDirty(rNewEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnCreateEntity(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyCreate);

        _rMessage.SetResult(1);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnAddEntity(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyAdd);

        _rMessage.SetResult(1);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnRemoveEntity(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyRemove);

        _rMessage.SetResult(1);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnDestroyEntity(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyRemove | Dt::CEntity::DirtyDestroy);

        _rMessage.SetResult(1);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnRequestInfoFacets(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Edit::CMessage NewMessage;

        NewMessage.PutInt(rCurrentEntity.GetID());

        NewMessage.PutInt(rCurrentEntity.GetCategory());
        NewMessage.PutInt(rCurrentEntity.GetType());

        NewMessage.PutBool(rCurrentEntity.GetTransformationFacet() != nullptr);
        NewMessage.PutBool(rCurrentEntity.GetHierarchyFacet()      != nullptr);

        NewMessage.PutBool(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data)    != nullptr);
        NewMessage.PutBool(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Graphic) != nullptr);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Entity_Facets_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnRequestInfoEntity(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));
           
        Edit::CMessage NewMessage;

        NewMessage.PutInt(rCurrentEntity.GetID());

        NewMessage.PutBool(rCurrentEntity.IsInMap());

        NewMessage.PutInt(rCurrentEntity.GetLayer());

        NewMessage.PutInt(rCurrentEntity.GetCategory());

        if (rCurrentEntity.GetName().length() > 0)
        {
            NewMessage.PutBool(true);

            NewMessage.PutString(rCurrentEntity.GetName().c_str());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Entity_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnRequestInfoTransformation(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Edit::CMessage NewMessage;

        Dt::CTransformationFacet* pTransformationFacet = rCurrentEntity.GetTransformationFacet();

        NewMessage.PutInt(rCurrentEntity.GetID());

        if (pTransformationFacet)
        {
            NewMessage.PutBool(true);

            NewMessage.PutFloat(pTransformationFacet->GetPosition()[0]);
            NewMessage.PutFloat(pTransformationFacet->GetPosition()[1]);
            NewMessage.PutFloat(pTransformationFacet->GetPosition()[2]);

            NewMessage.PutFloat(glm::degrees(pTransformationFacet->GetRotation()[0]));
            NewMessage.PutFloat(glm::degrees(pTransformationFacet->GetRotation()[1]));
            NewMessage.PutFloat(glm::degrees(pTransformationFacet->GetRotation()[2]));

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

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Entity_Transformation_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnInfoEntity(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        int Layer = _rMessage.GetInt();

        rCurrentEntity.SetLayer(Layer);

        int Category = _rMessage.GetInt();

        if(Category != static_cast<int>(rCurrentEntity.GetCategory())) return;

        bool HasName = _rMessage.GetBool();

        if (HasName)
        {
            char NewEntityName[256];

            _rMessage.GetString(NewEntityName, 256);

            rCurrentEntity.SetName(NewEntityName);
        }
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnInfoHierarchie(Edit::CMessage& _rMessage)
    {
        int EntityIDSource = _rMessage.GetInt();
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

    void CEntityHelper::OnInfoTransformation(Edit::CMessage& _rMessage)
    {
        float TranslationX;
        float TranslationY;
        float TranslationZ;
        float RotationX;
        float RotationY;
        float RotationZ;
        float ScaleX;
        float ScaleY;
        float ScaleZ;

        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CTransformationFacet* pTransformationFacet = rCurrentEntity.GetTransformationFacet();

        TranslationX = _rMessage.GetFloat();
        TranslationY = _rMessage.GetFloat();
        TranslationZ = _rMessage.GetFloat();

        if (pTransformationFacet)
        {
            RotationX = _rMessage.GetFloat();
            RotationY = _rMessage.GetFloat();
            RotationZ = _rMessage.GetFloat();

            ScaleX = _rMessage.GetFloat();
            ScaleY = _rMessage.GetFloat();
            ScaleZ = _rMessage.GetFloat();


            glm::vec3 Position(TranslationX, TranslationY, TranslationZ);
            glm::vec3 Rotation(glm::radians(RotationX), glm::radians(RotationY), glm::radians(RotationZ));
            glm::vec3 Scale(ScaleX, ScaleY, ScaleZ);

            pTransformationFacet->SetPosition(Position);
            pTransformationFacet->SetScale(Scale);
            pTransformationFacet->SetRotation(Rotation);
        }
        else
        {
            glm::vec3 Position(TranslationX, TranslationY, TranslationZ);

            rCurrentEntity.SetWorldPosition(Position);
        }

        Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyMove);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        if ((_pEntity->GetDirtyFlags() & Dt::CEntity::DirtyCreate) == Dt::CEntity::DirtyCreate)
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
            if (rCurrentEntity.GetName().length() > 0)
            {
                NewMessage.PutBool(true);
                NewMessage.PutString(rCurrentEntity.GetName().c_str());
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

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Entity_Hierarchy_Info, NewMessage);
        }
    }
} // namespace

namespace Edit
{
namespace Helper
{
namespace Entity
{
    void OnStart()
    {
        CEntityHelper::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CEntityHelper::GetInstance().OnExit();
    }
} // namespace Entity
} // namespace Helper
} // namespace Edit