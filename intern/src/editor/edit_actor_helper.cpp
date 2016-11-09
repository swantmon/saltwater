
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_actor_facet.h"
#include "data/data_actor_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_map.h"
#include "data/data_material_manager.h"
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
        Dt::SAssimpDescriptor ModelFileDesc;

        const char* pPathToFile = _rMessage.GetString(pTmp, 512);

        PathToFile = "models/" + CopyFileToAssets("../assets/models/", pPathToFile);

        ModelFileDesc.m_pPathToFile = PathToFile.c_str();

        Dt::CEntity& rNewEntity = Dt::EntityManager::CreateEntityFromFile(ModelFileDesc);

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

        Dt::CCameraActorFacet* pFacet = Dt::ActorManager::CreateCameraActor();

        rEntity.SetDetailFacet(Dt::SFacetCategory::Data, pFacet);

        Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnRequestActorInfoMaterial(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CModelActorFacet* pFacet = static_cast<Dt::CModelActorFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Actor && rCurrentEntity.GetType() == Dt::SActorType::Model && pFacet != nullptr)
        {
            // TODO by tschwandt
            // different surfaces necessary?

            Dt::CMaterial* pMaterial = pFacet->GetMaterial(0);

            // TODO by tschwandt
            // default material necessary?

            if (pMaterial == nullptr)
            {
                pMaterial = pFacet->GetModel()->GetLOD(0)->GetSurface(0)->GetMaterial();
            }

            Edit::CMessage NewMessage;

            NewMessage.PutInt(rCurrentEntity.GetID());

            if (pMaterial)
            {
                NewMessage.PutBool(true);

                NewMessage.PutFloat(pMaterial->GetColor()[0]);
                NewMessage.PutFloat(pMaterial->GetColor()[1]);
                NewMessage.PutFloat(pMaterial->GetColor()[2]);

                NewMessage.PutFloat(pMaterial->GetTilingOffset()[0]);
                NewMessage.PutFloat(pMaterial->GetTilingOffset()[1]);
                NewMessage.PutFloat(pMaterial->GetTilingOffset()[2]);
                NewMessage.PutFloat(pMaterial->GetTilingOffset()[3]);

                NewMessage.PutFloat(pMaterial->GetRoughness());
                NewMessage.PutFloat(pMaterial->GetReflectance());
                NewMessage.PutFloat(pMaterial->GetMetalness());

                if (pMaterial->GetColorTexture())
                {
                    NewMessage.PutBool(true);

                    NewMessage.PutString(pMaterial->GetColorTexture()->GetFileName());
                }
                else
                {
                    NewMessage.PutBool(false);
                }

                if (pMaterial->GetNormalTexture())
                {
                    NewMessage.PutBool(true);

                    NewMessage.PutString(pMaterial->GetNormalTexture()->GetFileName());
                }
                else
                {
                    NewMessage.PutBool(false);
                }

                if (pMaterial->GetRoughnessTexture())
                {
                    NewMessage.PutBool(true);

                    NewMessage.PutString(pMaterial->GetRoughnessTexture()->GetFileName());
                }
                else
                {
                    NewMessage.PutBool(false);
                }

                if (pMaterial->GetMetalTexture())
                {
                    NewMessage.PutBool(true);

                    NewMessage.PutString(pMaterial->GetMetalTexture()->GetFileName());
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

            
            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::ActorInfoCamera, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CActorHelper::OnActorInfoMaterial(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CModelActorFacet* pFacet = static_cast<Dt::CModelActorFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Actor && rCurrentEntity.GetType() == Dt::SActorType::Model && pFacet != nullptr)
        {
            float X, Y, Z, W;

            bool HasColorMap = false;
            bool HasNormalMap = false;
            bool HasRoughnessMap = false;
            bool HasMetalnessMap = false;

            char ColorMapName[256];
            char NormalMapName[256];
            char RoughnessMapName[256];
            char MetalMapName[256];

            // TODO by tschwandt
            // different surfaces necessary?

            Dt::CMaterial* pMaterial = pFacet->GetMaterial(0);

            // TODO by tschwandt
            // default material necessary?

            if (pMaterial == nullptr)
            {
                pMaterial = pFacet->GetModel()->GetLOD(0)->GetSurface(0)->GetMaterial();
            }

            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            X = _rMessage.GetFloat();
            Y = _rMessage.GetFloat();
            Z = _rMessage.GetFloat();

            Base::Float3 Color = Base::Float3(X, Y, Z);

            X = _rMessage.GetFloat();
            Y = _rMessage.GetFloat();
            Z = _rMessage.GetFloat();
            W = _rMessage.GetFloat();

            Base::Float4 TilingOffset = Base::Float4(X, Y, Z, W);

            float Roughness = _rMessage.GetFloat();

            float Reflectance = _rMessage.GetFloat();

            float Metalness = _rMessage.GetFloat();

            HasColorMap = _rMessage.GetBool();

            if (HasColorMap)
            {
                _rMessage.GetString(ColorMapName, 256);
            }

            HasNormalMap = _rMessage.GetBool();

            if (HasNormalMap)
            {
                _rMessage.GetString(NormalMapName, 256);
            }

            HasRoughnessMap = _rMessage.GetBool();

            if (HasRoughnessMap)
            {
                _rMessage.GetString(RoughnessMapName, 256);
            }

            HasMetalnessMap = _rMessage.GetBool();

            if (HasMetalnessMap)
            {
                _rMessage.GetString(MetalMapName, 256);
            }
                
            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            if (!pMaterial)
            {
                return;
            }

            pMaterial->SetColor       (Color);
            pMaterial->SetTilingOffset(TilingOffset);
            pMaterial->SetRoughness   (Roughness);
            pMaterial->SetReflectance (Reflectance);
            pMaterial->SetMetalness   (Metalness);

            Dt::STextureDescriptor TextureDescriptor;

            TextureDescriptor.m_NumberOfPixelsU  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
            TextureDescriptor.m_NumberOfPixelsV  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
            TextureDescriptor.m_NumberOfPixelsW  = 1;
            TextureDescriptor.m_Format           = Dt::CTextureBase::R8G8B8_UBYTE;
            TextureDescriptor.m_Semantic         = Dt::CTextureBase::Diffuse;
            TextureDescriptor.m_pPixels          = 0;
            TextureDescriptor.m_pFileName        = 0;
            TextureDescriptor.m_pIdentifier      = 0;

            if (HasColorMap && (pMaterial->GetColorTexture() == nullptr || strcmp(pMaterial->GetColorTexture()->GetFileName(), ColorMapName)))
            {
                TextureDescriptor.m_pFileName = ColorMapName;

                pMaterial->SetColorTexture(Dt::TextureManager::CreateTexture2D(TextureDescriptor));
            }
            else if (HasColorMap == false)
            {
                pMaterial->SetColorTexture(0);
            }

            if (HasNormalMap && (pMaterial->GetNormalTexture() == nullptr || strcmp(pMaterial->GetNormalTexture()->GetFileName(), NormalMapName)))
            {
                TextureDescriptor.m_pFileName = NormalMapName;

                pMaterial->SetNormalTexture(Dt::TextureManager::CreateTexture2D(TextureDescriptor));
            }
            else if (HasNormalMap == false)
            {
                pMaterial->SetNormalTexture(0);
            }

            if (HasRoughnessMap && (pMaterial->GetRoughnessTexture() == nullptr || strcmp(pMaterial->GetRoughnessTexture()->GetFileName(), RoughnessMapName)))
            {
                TextureDescriptor.m_pFileName = RoughnessMapName;

                pMaterial->SetRoughnessTexture(Dt::TextureManager::CreateTexture2D(TextureDescriptor));
            }
            else if (HasRoughnessMap == false)
            {
                pMaterial->SetRoughnessTexture(0);
            }

            if (HasMetalnessMap && (pMaterial->GetMetalTexture() == nullptr || strcmp(pMaterial->GetMetalTexture()->GetFileName(), MetalMapName)))
            {
                TextureDescriptor.m_pFileName = MetalMapName;

                pMaterial->SetMetalTexture(Dt::TextureManager::CreateTexture2D(TextureDescriptor));
            }
            else if (HasMetalnessMap == false)
            {
                pMaterial->SetMetalTexture(0);
            }

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
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