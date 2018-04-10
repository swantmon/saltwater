
#include "editor/edit_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_asset_manager.h"
#include "core/core_console.h"

#include "data/data_component_facet.h"
#include "data/data_component.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_map.h"
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

        EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
        EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

        Dt::CEntity& rNewEntity = Dt::EntityManager::CreateEntity(EntityDesc);

        _rMessage.SetResult(static_cast<int>(rNewEntity.GetID()));
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnLoadEntity(Edit::CMessage& _rMessage)
    {
        std::string Modelfile = _rMessage.Get<std::string>();

        std::string PathToModel = Core::AssetManager::GetPathToAssets() + "/" + Modelfile;

        auto ListOfEntities = Dt::EntityManager::CreateEntitiesFromScene(PathToModel);

        if (ListOfEntities.size() > 0)
        {
            Dt::SEntityDescriptor EntityDescriptor;

            EntityDescriptor.m_EntityCategory = Dt::SEntityCategory::Dynamic;
            EntityDescriptor.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

            Dt::CEntity& rRootEntity = Dt::EntityManager::CreateEntity(EntityDescriptor);

            rRootEntity.SetName(Modelfile);

            for (auto SubEntity : ListOfEntities)
            {
                assert(SubEntity != nullptr);

                rRootEntity.Attach(*SubEntity);
            }

            Dt::EntityManager::MarkEntityAsDirty(rRootEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnCreateEntity(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        if (pCurrentEntity == nullptr) return;

        Dt::EntityManager::MarkEntityAsDirty(*pCurrentEntity, Dt::CEntity::DirtyCreate);

        _rMessage.SetResult(1);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnAddEntity(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        if (pCurrentEntity == nullptr) return;

        pCurrentEntity->SetActive(true);

        Dt::EntityManager::MarkEntityAsDirty(*pCurrentEntity, Dt::CEntity::DirtyAdd);

        _rMessage.SetResult(1);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnRemoveEntity(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        if (pCurrentEntity == nullptr) return;

        pCurrentEntity->SetActive(false);

        Dt::EntityManager::MarkEntityAsDirty(*pCurrentEntity, Dt::CEntity::DirtyRemove);

        _rMessage.SetResult(1);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnDestroyEntity(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        if (pCurrentEntity == nullptr) return;

        Dt::EntityManager::MarkEntityAsDirty(*pCurrentEntity, Dt::CEntity::DirtyRemove | Dt::CEntity::DirtyDestroy);

        _rMessage.SetResult(1);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnRequestInfoFacets(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        if (pCurrentEntity == nullptr) return;

        Edit::CMessage NewMessage;

        NewMessage.Put(pCurrentEntity->GetID());

        NewMessage.Put(pCurrentEntity->GetTransformationFacet() != nullptr);

        auto Components = pCurrentEntity->GetComponentFacet()->GetComponents();

        NewMessage.Put(Components.size());

        for (auto Component : Components)
        {
            NewMessage.Put(Component->GetTypeID());
        }

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Entity_Facets_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnRequestInfoEntity(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        if (pCurrentEntity == nullptr) return;
           
        Edit::CMessage NewMessage;

        NewMessage.Put(pCurrentEntity->GetID());

        NewMessage.Put(pCurrentEntity->IsInMap());

        NewMessage.Put(pCurrentEntity->GetLayer());

        NewMessage.Put(pCurrentEntity->GetCategory());

        if (pCurrentEntity->GetName().length() > 0)
        {
            NewMessage.Put(true);

            NewMessage.Put(pCurrentEntity->GetName());
        }
        else
        {
            NewMessage.Put(false);
        }

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Entity_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnRequestInfoTransformation(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        if (pCurrentEntity == nullptr) return;

        Edit::CMessage NewMessage;

        Dt::CTransformationFacet* pTransformationFacet = pCurrentEntity->GetTransformationFacet();

        NewMessage.Put(pCurrentEntity->GetID());

        if (pTransformationFacet)
        {
            NewMessage.Put(true);

            NewMessage.Put(pTransformationFacet->GetPosition());

            NewMessage.Put(pTransformationFacet->GetRotation());

            NewMessage.Put(pTransformationFacet->GetScale());
        }
        else
        {
            NewMessage.Put(false);

            NewMessage.Put(pCurrentEntity->GetWorldPosition());
        }

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Entity_Transformation_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnInfoEntity(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        if (pCurrentEntity == nullptr) return;

        int Layer = _rMessage.Get<int>();

        pCurrentEntity->SetLayer(Layer);

        int Category = _rMessage.Get<int>();

        if(Category != static_cast<int>(pCurrentEntity->GetCategory())) return;

        std::string NewEntityName = _rMessage.Get<std::string>();

        pCurrentEntity->SetName(NewEntityName);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnInfoHierarchie(Edit::CMessage& _rMessage)
    {
        Base::ID EntityIDSource      = _rMessage.Get<Base::ID>();
        Base::ID EntityIDDestination = _rMessage.Get<Base::ID>();

        assert(EntityIDSource != -1);

        // ----------------------------------------------------------------------------- 
        // Get source entity and hierarchy facet 
        // ----------------------------------------------------------------------------- 
        Dt::CEntity* pSourceEntity = Dt::EntityManager::GetEntityByID(EntityIDSource);

        if (pSourceEntity == nullptr) return;

        pSourceEntity->Detach();

        if (EntityIDDestination != -1)
        {
            Dt::CEntity* pDestinationEntity = Dt::EntityManager::GetEntityByID(EntityIDDestination);

            pDestinationEntity->Attach(*pSourceEntity);
        }

        Dt::EntityManager::MarkEntityAsDirty(*pSourceEntity, Dt::CEntity::DirtyMove);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnInfoTransformation(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        if (pCurrentEntity == nullptr) return;

        Dt::CTransformationFacet* pTransformationFacet = pCurrentEntity->GetTransformationFacet();

        glm::vec3 Translation = _rMessage.Get<glm::vec3>();

        if (pTransformationFacet)
        {
            glm::quat Rotation = _rMessage.Get<glm::quat>();
            glm::vec3 Scale    = _rMessage.Get<glm::vec3>();

            pTransformationFacet->SetPosition(Translation);
            pTransformationFacet->SetScale(Scale);
            pTransformationFacet->SetRotation(Rotation);
        }
        else
        {
            pCurrentEntity->SetWorldPosition(Translation);
        }

        Dt::EntityManager::MarkEntityAsDirty(*pCurrentEntity, Dt::CEntity::DirtyMove);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        if ((_pEntity->GetDirtyFlags() & Dt::CEntity::DirtyCreate) == Dt::CEntity::DirtyCreate)
        {
            Edit::CMessage NewMessage;

            Dt::CEntity* pCurrentEntity = _pEntity;

            if (pCurrentEntity == nullptr) return;

            // -----------------------------------------------------------------------------
            // ID
            // -----------------------------------------------------------------------------
            NewMessage.Put(pCurrentEntity->GetID());

            // -----------------------------------------------------------------------------
            // Name
            // -----------------------------------------------------------------------------
            NewMessage.Put(pCurrentEntity->GetName());

            // -----------------------------------------------------------------------------
            // Hierarchy
            // -----------------------------------------------------------------------------
            Dt::CHierarchyFacet* pHierarchyFacet = pCurrentEntity->GetHierarchyFacet();

            if (pHierarchyFacet)
            {
                NewMessage.Put(true);

                Dt::CEntity* pParentEntity = pHierarchyFacet->GetParent();

                if (pParentEntity)
                {
                    NewMessage.Put(true);

                    NewMessage.Put(pParentEntity->GetID());
                }
                else
                {
                    NewMessage.Put(false);
                }
            }
            else
            {
                NewMessage.Put(false);
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