
#include "editor/edit_precompiled.h"

#include "base/base_aabb2.h"
#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_actor_type.h"
#include "data/data_camera_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"
#include "data/data_material_manager.h"
#include "data/data_mesh_facet.h"
#include "data/data_model_manager.h"
#include "data/data_texture_manager.h"
#include "data/data_transformation_facet.h"

#include "editor/edit_actor_helper.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <windows.h>
#undef SendMessage

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

        std::string CopyFileToAssets(const char* _pAssetFolder, const char* _pPathToFile);
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
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewActorModel, EDIT_RECEIVE_MESSAGE(&CActorHelper::OnNewActorModel));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewActorCamera, EDIT_RECEIVE_MESSAGE(&CActorHelper::OnNewActorCamera));
        
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestActorInfoMaterial, EDIT_RECEIVE_MESSAGE(&CActorHelper::OnRequestActorInfoMaterial));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestActorInfoCamera, EDIT_RECEIVE_MESSAGE(&CActorHelper::OnRequestActorInfoCamera));

        Edit::MessageManager::Register(Edit::SGUIMessageType::ActorInfoMaterial, EDIT_RECEIVE_MESSAGE(&CActorHelper::OnActorInfoMaterial));
        Edit::MessageManager::Register(Edit::SGUIMessageType::ActorInfoCamera, EDIT_RECEIVE_MESSAGE(&CActorHelper::OnActorInfoCamera));
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnNewActorModel(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Create new entity
        // -----------------------------------------------------------------------------
        char        pTmp[512];
        std::string PathToFile;

        // -----------------------------------------------------------------------------
        // Model
        // -----------------------------------------------------------------------------
        Dt::SModelFileDescriptor ModelFileDesc;

        const char* pPathToFile = _rMessage.GetString(pTmp, 512);

        PathToFile = "models/" + CopyFileToAssets("../assets/models/", pPathToFile);

        ModelFileDesc.m_pFileName = PathToFile.c_str();
        ModelFileDesc.m_GenFlag   = Dt::SGeneratorFlag::Default;

        Dt::CModel& rModel = Dt::ModelManager::CreateModel(ModelFileDesc);

        Dt::CEntity& rNewEntity = Dt::EntityManager::CreateEntityFromModel(rModel);

        rNewEntity.SetName("New Model");

        // -----------------------------------------------------------------------------
        // Add model to map
        // -----------------------------------------------------------------------------
        Dt::EntityManager::MarkEntityAsDirty(rNewEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnNewActorCamera(Edit::CMessage& _rMessage)
    {
        Dt::SEntityDescriptor EntityDesc;

        EntityDesc.m_EntityCategory = Dt::SEntityCategory::Actor;
        EntityDesc.m_EntityType = Dt::SActorType::Camera;
        EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

        Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

        rEntity.SetName("New Camera");

        Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

        pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 10.0f));
        pTransformationFacet->SetScale   (Base::Float3(1.0f));
        pTransformationFacet->SetRotation(Base::Float3(0.0f, 0.0f, 0.0f));

        Dt::CCameraActorFacet* pFacet = Dt::CameraManager::CreateCameraActor();

        rEntity.SetDetailFacet(Dt::SFacetCategory::Data, pFacet);

        Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnRequestActorInfoMaterial(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CMeshActorFacet* pFacet = static_cast<Dt::CMeshActorFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Actor && rCurrentEntity.GetType() == Dt::SActorType::Mesh && pFacet != nullptr)
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

            NewMessage.PutInt(rCurrentEntity.GetID());

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

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::ActorInfoMaterial, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnRequestActorInfoCamera(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CCameraActorFacet* pFacet = static_cast<Dt::CCameraActorFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Actor && rCurrentEntity.GetType() == Dt::SActorType::Camera && pFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(rCurrentEntity.GetID());

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

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::ActorInfoCamera, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnActorInfoMaterial(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CMeshActorFacet* pFacet = static_cast<Dt::CMeshActorFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Actor && rCurrentEntity.GetType() == Dt::SActorType::Mesh && pFacet != nullptr)
        {
            
        }
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnActorInfoCamera(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CCameraActorFacet* pFacet = static_cast<Dt::CCameraActorFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Actor && rCurrentEntity.GetType() == Dt::SActorType::Camera && pFacet != nullptr)
        {
            float R, G, B;
            float X, Y, W, H;

            // -----------------------------------------------------------------------------
            // Get values
            // -----------------------------------------------------------------------------
            bool IsMainCamera = _rMessage.GetBool();

            Dt::CCameraActorFacet::EClearFlag ClearFlag = static_cast<Dt::CCameraActorFacet::EClearFlag >(_rMessage.GetInt());

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

            Dt::CCameraActorFacet::EProjectionType ProjectionType = static_cast<Dt::CCameraActorFacet::EProjectionType>(_rMessage.GetInt());

            float Size = _rMessage.GetFloat();

            float FOV = _rMessage.GetFloat();

            float Near = _rMessage.GetFloat();

            float Far = _rMessage.GetFloat();

            X = _rMessage.GetFloat();
            Y = _rMessage.GetFloat();
            W = _rMessage.GetFloat();
            H = _rMessage.GetFloat();

            float Depth = _rMessage.GetFloat();

            Dt::CCameraActorFacet::ECameraMode CameraMode = static_cast<Dt::CCameraActorFacet::ECameraMode>(_rMessage.GetInt());

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

            pFacet->SetBackgroundColor(Base::Float3(R, G, B));

            pFacet->SetCullingMask(CullingMask);

            pFacet->SetProjectionType(ProjectionType);

            pFacet->SetSize(Size);

            pFacet->SetFoV(FOV);

            pFacet->SetNear(Near);

            pFacet->SetFar(Far);

            pFacet->SetViewportRect(Base::AABB2Float(Base::Float2(X, Y), Base::Float2(W, H)));
            
            pFacet->SetDepth(Depth);

            pFacet->SetCameraMode(CameraMode);

            pFacet->SetShutterSpeed(ShutterSpeed);

            pFacet->SetAperture(Aperture);

            pFacet->SetISO(ISO);

            pFacet->SetEC(EC);

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
        }
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        if ((_pEntity->GetDirtyFlags() & Dt::CEntity::DirtyAdd) == Dt::CEntity::DirtyAdd)
        {
            
        }
    }

    // -----------------------------------------------------------------------------

    std::string CActorHelper::CopyFileToAssets(const char* _pAssetFolder, const char* _pPathToFile)
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