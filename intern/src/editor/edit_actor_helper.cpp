
#include "editor/edit_precompiled.h"

#include "base/base_aabb2.h"
#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_camera_component.h"
#include "data/data_component.h"
#include "data/data_component_facet.h"
#include "data/data_component_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"
#include "data/data_material.h"
#include "data/data_material_component.h"
#include "data/data_material_manager.h"
#include "data/data_mesh_component.h"
#include "data/data_transformation_facet.h"

#include "editor/edit_actor_helper.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

namespace
{
    class CActorHelper : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CActorHelper)

    public:

        CActorHelper();
        ~CActorHelper();

    public:

        void OnStart();
        void OnExit();

    private:

        void OnNewActorModel(Edit::CMessage& _rMessage);
        void OnNewActorCamera(Edit::CMessage& _rMessage);

        void OnRequestActorInfoMaterial(Edit::CMessage& _rMessage);
        void OnRequestActorInfoCamera(Edit::CMessage& _rMessage);

        void OnActorInfoMaterial(Edit::CMessage& _rMessage);
        void OnActorInfoCamera(Edit::CMessage& _rMessage);

        void OnDirtyEntity(Dt::CEntity* _pEntity);
    };
} // namespace

namespace
{
    CActorHelper::CActorHelper()
    {
        
    }

    // -----------------------------------------------------------------------------

    CActorHelper::~CActorHelper()
    {

    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Entity
        // -----------------------------------------------------------------------------
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CActorHelper::OnDirtyEntity));

        // -----------------------------------------------------------------------------
        // Edit
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SGUIMessageType::Actor_Mesh_New, EDIT_RECEIVE_MESSAGE(&CActorHelper::OnNewActorModel));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Actor_Camera_New, EDIT_RECEIVE_MESSAGE(&CActorHelper::OnNewActorCamera));
        
        Edit::MessageManager::Register(Edit::SGUIMessageType::Actor_Material_Info, EDIT_RECEIVE_MESSAGE(&CActorHelper::OnRequestActorInfoMaterial));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Actor_Camera_Info, EDIT_RECEIVE_MESSAGE(&CActorHelper::OnRequestActorInfoCamera));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Actor_Material_Update, EDIT_RECEIVE_MESSAGE(&CActorHelper::OnActorInfoMaterial));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Actor_Camera_Update, EDIT_RECEIVE_MESSAGE(&CActorHelper::OnActorInfoCamera));
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnNewActorModel(Edit::CMessage& _rMessage)
    {
        BASE_UNUSED(_rMessage);
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnNewActorCamera(Edit::CMessage& _rMessage)
    {
        {
            // -----------------------------------------------------------------------------
            // Get entity and set type + category
            // -----------------------------------------------------------------------------
            Base::ID EntityID = _rMessage.Get<Base::ID>();

            Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

            rCurrentEntity.SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CCameraComponent>();

            rCurrentEntity.AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pComponent, Dt::CCameraComponent::DirtyCreate);
        }
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnRequestActorInfoMaterial(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CMaterialComponent* pComponent = rCurrentEntity.GetComponentFacet()->GetComponent<Dt::CMaterialComponent>();

        if (pComponent == nullptr || pComponent->GetMaterial() == nullptr) return;

        Edit::CMessage NewMessage;

        NewMessage.Put(rCurrentEntity.GetID());

        NewMessage.Put(pComponent->GetMaterial()->GetHash());

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Actor_Material_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnRequestActorInfoCamera(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CCameraComponent* pFacet = rCurrentEntity.GetComponentFacet()->GetComponent<Dt::CCameraComponent>();

        if (pFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.Put(rCurrentEntity.GetID());

            NewMessage.Put(pFacet->IsMainCamera());

            NewMessage.Put(pFacet->GetClearFlag());

            NewMessage.Put(pFacet->GetBackgroundColor()[0]);
            NewMessage.Put(pFacet->GetBackgroundColor()[1]);
            NewMessage.Put(pFacet->GetBackgroundColor()[2]);

            NewMessage.Put(pFacet->GetCullingMask());

            NewMessage.Put(pFacet->GetProjectionType());

            NewMessage.Put(pFacet->GetSize());

            NewMessage.Put(pFacet->GetFoV());

            NewMessage.Put(pFacet->GetNear());

            NewMessage.Put(pFacet->GetFar());

            NewMessage.Put(pFacet->GetViewportRect()[0][0]);
            NewMessage.Put(pFacet->GetViewportRect()[0][1]);
            NewMessage.Put(pFacet->GetViewportRect()[1][0]);
            NewMessage.Put(pFacet->GetViewportRect()[1][1]);

            NewMessage.Put(pFacet->GetDepth());

            NewMessage.Put(pFacet->GetCameraMode());

            NewMessage.Put(pFacet->GetShutterSpeed());

            NewMessage.Put(pFacet->GetAperture());

            NewMessage.Put(pFacet->GetISO());

            NewMessage.Put(pFacet->GetEC());
            
            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Actor_Camera_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnActorInfoMaterial(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Base::BHash MaterialHash = _rMessage.Get<Base::BHash>();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        auto pComponent = rCurrentEntity.GetComponentFacet()->GetComponent<Dt::CMaterialComponent>();

        if (pComponent != nullptr)
        {
            pComponent->SetMaterial(Dt::MaterialManager::GetMaterialByHash(MaterialHash));

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pComponent, Dt::CMaterialComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnActorInfoCamera(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CCameraComponent* pFacet = rCurrentEntity.GetComponentFacet()->GetComponent<Dt::CCameraComponent>();

        if (pFacet != nullptr)
        {
            float R, G, B;
            float X, Y, W, H;

            // -----------------------------------------------------------------------------
            // Get values
            // -----------------------------------------------------------------------------
            bool IsMainCamera = _rMessage.Get<bool>();

            Dt::CCameraComponent::EClearFlag ClearFlag = static_cast<Dt::CCameraComponent::EClearFlag >(_rMessage.Get<int>());

            R = _rMessage.Get<float>();
            G = _rMessage.Get<float>();
            B = _rMessage.Get<float>();

            int CullingMask = _rMessage.Get<int>();

            Dt::CCameraComponent::EProjectionType ProjectionType = static_cast<Dt::CCameraComponent::EProjectionType>(_rMessage.Get<int>());

            float Size = _rMessage.Get<float>();

            float FOV = _rMessage.Get<float>();

            float Near = _rMessage.Get<float>();

            float Far = _rMessage.Get<float>();

            X = _rMessage.Get<float>();
            Y = _rMessage.Get<float>();
            W = _rMessage.Get<float>();
            H = _rMessage.Get<float>();

            float Depth = _rMessage.Get<float>();

            Dt::CCameraComponent::ECameraMode CameraMode = static_cast<Dt::CCameraComponent::ECameraMode>(_rMessage.Get<int>());

            float ShutterSpeed = _rMessage.Get<float>();

            float Aperture = _rMessage.Get<float>();

            float ISO = _rMessage.Get<float>();

            float EC = _rMessage.Get<float>();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pFacet->SetMainCamera(IsMainCamera);

            pFacet->SetClearFlag(ClearFlag);

            pFacet->SetBackgroundColor(glm::vec3(R, G, B));

            pFacet->SetCullingMask(CullingMask);

            pFacet->SetProjectionType(ProjectionType);

            pFacet->SetSize(Size);

            pFacet->SetFoV(FOV);

            pFacet->SetNear(Near);

            pFacet->SetFar(Far);

            pFacet->SetViewportRect(Base::AABB2Float(glm::vec2(X, Y), glm::vec2(W, H)));
            
            pFacet->SetDepth(Depth);

            pFacet->SetCameraMode(CameraMode);

            pFacet->SetShutterSpeed(ShutterSpeed);

            pFacet->SetAperture(Aperture);

            pFacet->SetISO(ISO);

            pFacet->SetEC(EC);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pFacet, Dt::CCameraComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        if ((_pEntity->GetDirtyFlags() & Dt::CEntity::DirtyAdd) == Dt::CEntity::DirtyAdd)
        {
            
        }
    }
} // namespace

namespace Edit
{
namespace Helper
{
namespace Actor
{
    void OnStart()
    {
        CActorHelper::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CActorHelper::GetInstance().OnExit();
    }
} // namespace Actor
} // namespace Helper
} // namespace Edit