
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_component_facet.h"
#include "data/data_component_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_map.h"
#include "data/data_mesh_manager.h"
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
        EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

        Dt::CEntity& rNewEntity = Dt::EntityManager::CreateEntity(EntityDesc);

        Dt::CTransformationFacet* pTransformationFacet = rNewEntity.GetTransformationFacet();

        pTransformationFacet->SetPosition(glm::vec3(0.0f));
        pTransformationFacet->SetScale(glm::vec3(1.0f));
        pTransformationFacet->SetRotation(glm::vec3(0.0f));

        _rMessage.SetResult(static_cast<int>(rNewEntity.GetID()));
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnLoadEntity(Edit::CMessage& _rMessage)
    {
        // TODO by tschwandt
        // Add loading map from scene file
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnCreateEntity(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyCreate);

        _rMessage.SetResult(1);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnAddEntity(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        rCurrentEntity.SetActive(true);

        Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyAdd);

        _rMessage.SetResult(1);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnRemoveEntity(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        rCurrentEntity.SetActive(false);

        Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyRemove);

        _rMessage.SetResult(1);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnDestroyEntity(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyRemove | Dt::CEntity::DirtyDestroy);

        _rMessage.SetResult(1);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnRequestInfoFacets(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Edit::CMessage NewMessage;

        NewMessage.Put(rCurrentEntity.GetID());

        NewMessage.Put(rCurrentEntity.GetTransformationFacet() != nullptr);

        auto Components = rCurrentEntity.GetComponentFacet()->GetComponents();

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

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);
           
        Edit::CMessage NewMessage;

        NewMessage.Put(rCurrentEntity.GetID());

        NewMessage.Put(rCurrentEntity.IsInMap());

        NewMessage.Put(rCurrentEntity.GetLayer());

        NewMessage.Put(rCurrentEntity.GetCategory());

        if (rCurrentEntity.GetName().length() > 0)
        {
            NewMessage.Put(true);

            NewMessage.Put(rCurrentEntity.GetName());
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

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Edit::CMessage NewMessage;

        Dt::CTransformationFacet* pTransformationFacet = rCurrentEntity.GetTransformationFacet();

        NewMessage.Put(rCurrentEntity.GetID());

        if (pTransformationFacet)
        {
            NewMessage.Put(true);

            NewMessage.Put(pTransformationFacet->GetPosition()[0]);
            NewMessage.Put(pTransformationFacet->GetPosition()[1]);
            NewMessage.Put(pTransformationFacet->GetPosition()[2]);

            NewMessage.Put(glm::degrees(pTransformationFacet->GetRotation()[0]));
            NewMessage.Put(glm::degrees(pTransformationFacet->GetRotation()[1]));
            NewMessage.Put(glm::degrees(pTransformationFacet->GetRotation()[2]));

            NewMessage.Put(pTransformationFacet->GetScale()[0]);
            NewMessage.Put(pTransformationFacet->GetScale()[1]);
            NewMessage.Put(pTransformationFacet->GetScale()[2]);
        }
        else
        {
            NewMessage.Put(false);

            NewMessage.Put(rCurrentEntity.GetWorldPosition()[0]);
            NewMessage.Put(rCurrentEntity.GetWorldPosition()[1]);
            NewMessage.Put(rCurrentEntity.GetWorldPosition()[2]);
        }

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Entity_Transformation_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnInfoEntity(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        int Layer = _rMessage.Get<int>();

        rCurrentEntity.SetLayer(Layer);

        int Category = _rMessage.Get<int>();

        if(Category != static_cast<int>(rCurrentEntity.GetCategory())) return;

        std::string NewEntityName = _rMessage.Get<std::string>();

        rCurrentEntity.SetName(NewEntityName);
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
        Dt::CEntity& rSourceEntity = Dt::EntityManager::GetEntityByID(EntityIDSource);

        rSourceEntity.Detach();

        if (EntityIDDestination != -1)
        {
            Dt::CEntity& rDestinationEntity = Dt::EntityManager::GetEntityByID(EntityIDDestination);

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

        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CTransformationFacet* pTransformationFacet = rCurrentEntity.GetTransformationFacet();

        TranslationX = _rMessage.Get<float>();
        TranslationY = _rMessage.Get<float>();
        TranslationZ = _rMessage.Get<float>();

        if (pTransformationFacet)
        {
            RotationX = _rMessage.Get<float>();
            RotationY = _rMessage.Get<float>();
            RotationZ = _rMessage.Get<float>();

            ScaleX = _rMessage.Get<float>();
            ScaleY = _rMessage.Get<float>();
            ScaleZ = _rMessage.Get<float>();


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
            NewMessage.Put(rCurrentEntity.GetID());

            // -----------------------------------------------------------------------------
            // Name
            // -----------------------------------------------------------------------------
            NewMessage.Put(rCurrentEntity.GetName());

            // -----------------------------------------------------------------------------
            // Hierarchy
            // -----------------------------------------------------------------------------
            Dt::CHierarchyFacet* pHierarchyFacet = rCurrentEntity.GetHierarchyFacet();

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