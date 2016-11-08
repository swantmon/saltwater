
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_actor_facet.h"
#include "data/data_actor_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_map.h"
#include "data/data_transformation_facet.h"

#include "editor/edit_entity_helper.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <windows.h>
#undef SendMessage

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

        Dt::CEntity* m_pLastRequestedEntity;

    private:

        void OnRemoveEntity(Edit::CMessage& _rMessage);

        void OnRequestEntityInfoFacets(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoEntity(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoTransformation(Edit::CMessage& _rMessage);

        void OnEntityInfoEntity(Edit::CMessage& _rMessage);
        void OnEntityInfoHierarchie(Edit::CMessage& _rMessage);
        void OnEntityInfoTransformation(Edit::CMessage& _rMessage);

        void OnDirtyEntity(Dt::CEntity* _pEntity);
    };
} // namespace

namespace
{
    CEntityHelper::CEntityHelper()
        : m_pLastRequestedEntity(nullptr)
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
        Edit::MessageManager::Register(Edit::SGUIMessageType::RemoveEntity                   , EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnRemoveEntity));

        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoFacets        , EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnRequestEntityInfoFacets));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoEntity        , EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnRequestEntityInfoEntity));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEntityInfoTransformation, EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnRequestEntityInfoTransformation));
        
        Edit::MessageManager::Register(Edit::SGUIMessageType::EntityInfoEntity               , EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnEntityInfoEntity));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EntityInfoHierarchie           , EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnEntityInfoHierarchie));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EntityInfoTransformation       , EDIT_RECEIVE_MESSAGE(&CEntityHelper::OnEntityInfoTransformation));
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnRemoveEntity(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyRemove | Dt::CEntity::DirtyDestroy);

        _rMessage.SetResult(100);

        m_pLastRequestedEntity = nullptr;
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnRequestEntityInfoFacets(Edit::CMessage& _rMessage)
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

    void CEntityHelper::OnRequestEntityInfoEntity(Edit::CMessage& _rMessage)
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

    void CEntityHelper::OnRequestEntityInfoTransformation(Edit::CMessage& _rMessage)
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

    void CEntityHelper::OnEntityInfoEntity(Edit::CMessage& _rMessage)
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

    void CEntityHelper::OnEntityInfoHierarchie(Edit::CMessage& _rMessage)
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

    void CEntityHelper::OnEntityInfoTransformation(Edit::CMessage& _rMessage)
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

        if (m_pLastRequestedEntity != nullptr)
        {
            Dt::CEntity& rCurrentEntity = *m_pLastRequestedEntity;

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


                Base::Float3 Position(TranslationX, TranslationY, TranslationZ);
                Base::Float3 Rotation(Base::DegreesToRadians(RotationX), Base::DegreesToRadians(RotationY), Base::DegreesToRadians(RotationZ));
                Base::Float3 Scale(ScaleX, ScaleY, ScaleZ);

                pTransformationFacet->SetPosition(Position);
                pTransformationFacet->SetScale(Scale);
                pTransformationFacet->SetRotation(Rotation);
            }
            else
            {
                Base::Float3 Position(TranslationX, TranslationY, TranslationZ);

                rCurrentEntity.SetWorldPosition(Position);
            }

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyMove);
        }
    }

    // -----------------------------------------------------------------------------

    void CEntityHelper::OnDirtyEntity(Dt::CEntity* _pEntity)
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