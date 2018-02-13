
#include "editor/edit_precompiled.h"

#include "base/base_aabb2.h"
#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_camera_component.h"
#include "data/data_component_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"
#include "data/data_material_manager.h"
#include "data/data_mesh.h"
#include "data/data_mesh_component.h"
#include "data/data_texture_manager.h"
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
            int EntityID = _rMessage.GetInt();

            Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

            rCurrentEntity.SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CCameraComponent>();

            rCurrentEntity.AddComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pComponent, Dt::CCameraComponent::DirtyCreate);
        }
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnRequestActorInfoMaterial(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CMeshComponent* pFacet = rCurrentEntity.GetComponent<Dt::CMeshComponent>();

        if (pFacet != nullptr)
        {
            // TODO by tschwandt
            // different surfaces necessary?

            Dt::CMaterial* pMaterial = pFacet->GetMaterial(0);

            // TODO by tschwandt
            // default material necessary?

            if (pMaterial == nullptr)
            {
                pMaterial = pFacet->GetMesh()->GetLOD(0)->GetSurface(0)->GetMaterial();
            }

            Edit::CMessage NewMessage;

            NewMessage.PutInt(static_cast<int>(rCurrentEntity.GetID()));

            if (pMaterial)
            {
                NewMessage.PutBool(true);

                NewMessage.PutInt(pMaterial->GetHash());
            }
            else
            {
                NewMessage.PutBool(false);
            }

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Actor_Material_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnRequestActorInfoCamera(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CCameraComponent* pFacet = rCurrentEntity.GetComponent<Dt::CCameraComponent>();

        if (pFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(static_cast<int>(rCurrentEntity.GetID()));

            NewMessage.PutBool(pFacet->IsMainCamera());

            NewMessage.PutInt(pFacet->GetClearFlag());

            if (pFacet->GetHasTexture())
            {
                NewMessage.PutBool(true);

                NewMessage.PutInt(pFacet->GetTexture()->GetHash());
            }
            else
            {
                NewMessage.PutBool(false);
            }

            NewMessage.PutFloat(pFacet->GetBackgroundColor()[0]);
            NewMessage.PutFloat(pFacet->GetBackgroundColor()[1]);
            NewMessage.PutFloat(pFacet->GetBackgroundColor()[2]);

            NewMessage.PutInt(pFacet->GetCullingMask());

            NewMessage.PutInt(pFacet->GetProjectionType());

            NewMessage.PutFloat(pFacet->GetSize());

            NewMessage.PutFloat(pFacet->GetFoV());

            NewMessage.PutFloat(pFacet->GetNear());

            NewMessage.PutFloat(pFacet->GetFar());

            NewMessage.PutFloat(pFacet->GetViewportRect()[0][0]);
            NewMessage.PutFloat(pFacet->GetViewportRect()[0][1]);
            NewMessage.PutFloat(pFacet->GetViewportRect()[1][0]);
            NewMessage.PutFloat(pFacet->GetViewportRect()[1][1]);

            NewMessage.PutFloat(pFacet->GetDepth());

            NewMessage.PutInt(pFacet->GetCameraMode());

            NewMessage.PutFloat(pFacet->GetShutterSpeed());

            NewMessage.PutFloat(pFacet->GetAperture());

            NewMessage.PutFloat(pFacet->GetISO());

            NewMessage.PutFloat(pFacet->GetEC());
            
            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Actor_Camera_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnActorInfoMaterial(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        int MaterialHash = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CMeshComponent* pFacet = rCurrentEntity.GetComponent<Dt::CMeshComponent>();

        if (pFacet != nullptr)
        {
            Dt::CMaterial& rDtMaterial = Dt::MaterialManager::GetMaterialByHash(MaterialHash);

            pFacet->SetMaterial(0, &rDtMaterial);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pFacet, Dt::CMeshComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnActorInfoCamera(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CCameraComponent* pFacet = rCurrentEntity.GetComponent<Dt::CCameraComponent>();

        if (pFacet != nullptr)
        {
            float R, G, B;
            float X, Y, W, H;

            // -----------------------------------------------------------------------------
            // Get values
            // -----------------------------------------------------------------------------
            bool IsMainCamera = _rMessage.GetBool();

            Dt::CCameraComponent::EClearFlag ClearFlag = static_cast<Dt::CCameraComponent::EClearFlag >(_rMessage.GetInt());

            bool HasTexture = _rMessage.GetBool();

            int TextureHash = -1;

            if (HasTexture)
            {
                TextureHash = _rMessage.GetInt();
            }

            R = _rMessage.GetFloat();
            G = _rMessage.GetFloat();
            B = _rMessage.GetFloat();

            int CullingMask = _rMessage.GetInt();

            Dt::CCameraComponent::EProjectionType ProjectionType = static_cast<Dt::CCameraComponent::EProjectionType>(_rMessage.GetInt());

            float Size = _rMessage.GetFloat();

            float FOV = _rMessage.GetFloat();

            float Near = _rMessage.GetFloat();

            float Far = _rMessage.GetFloat();

            X = _rMessage.GetFloat();
            Y = _rMessage.GetFloat();
            W = _rMessage.GetFloat();
            H = _rMessage.GetFloat();

            float Depth = _rMessage.GetFloat();

            Dt::CCameraComponent::ECameraMode CameraMode = static_cast<Dt::CCameraComponent::ECameraMode>(_rMessage.GetInt());

            float ShutterSpeed = _rMessage.GetFloat();

            float Aperture = _rMessage.GetFloat();

            float ISO = _rMessage.GetFloat();

            float EC = _rMessage.GetFloat();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pFacet->SetMainCamera(IsMainCamera);

            pFacet->SetClearFlag(ClearFlag);

            if (HasTexture)
            {
                Dt::CTexture2D* pBackgroundTexture = Dt::TextureManager::GetTexture2DByHash(TextureHash);

                pFacet->SetTexture(pBackgroundTexture);
            }
            else
            {
                pFacet->SetTexture(nullptr);
            }

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