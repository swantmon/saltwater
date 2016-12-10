
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_actor_type.h"
#include "data/data_ar_controller_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_plugin_type.h"
#include "data/data_texture_manager.h"

#include "editor/edit_plugin_helper.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

namespace
{
    class CPluginHelper : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CPluginHelper)

    public:

        CPluginHelper();
        ~CPluginHelper();

    public:

        void OnStart();
        void OnExit();

    private:

        Dt::CEntity* m_pLastRequestedEntity;

    private:

        void OnNewPluginARController(Edit::CMessage& _rMessage);

        void OnRequestPluginInfoARController(Edit::CMessage& _rMessage);
        void OnRequestPluginInfoARControllerMarker(Edit::CMessage& _rMessage);

        void OnPluginInfoARController(Edit::CMessage& _rMessage);
        void OnPluginInfoARControllerMarker(Edit::CMessage& _rMessage);

        void OnDirtyEntity(Dt::CEntity* _pEntity);
    };
} // namespace

namespace
{
    CPluginHelper::CPluginHelper()
        : m_pLastRequestedEntity(nullptr)
    {

    }

    // -----------------------------------------------------------------------------

    CPluginHelper::~CPluginHelper()
    {

    }

    // -----------------------------------------------------------------------------

    void CPluginHelper::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Entity
        // -----------------------------------------------------------------------------
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CPluginHelper::OnDirtyEntity));

        // -----------------------------------------------------------------------------
        // Edit
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SGUIMessageType::Plugin_ARConroller_New, EDIT_RECEIVE_MESSAGE(&CPluginHelper::OnNewPluginARController));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Plugin_ARConroller_Info       , EDIT_RECEIVE_MESSAGE(&CPluginHelper::OnRequestPluginInfoARController));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Plugin_ARConroller_Marker_Info, EDIT_RECEIVE_MESSAGE(&CPluginHelper::OnRequestPluginInfoARControllerMarker));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Plugin_ARConroller_Update       , EDIT_RECEIVE_MESSAGE(&CPluginHelper::OnPluginInfoARController));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Plugin_ARConroller_Marker_Update, EDIT_RECEIVE_MESSAGE(&CPluginHelper::OnPluginInfoARControllerMarker));
    }

    // -----------------------------------------------------------------------------

    void CPluginHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CPluginHelper::OnNewPluginARController(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Get entity and set type + category
        // -----------------------------------------------------------------------------
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        rCurrentEntity.SetCategory(Dt::SEntityCategory::Plugin);
        rCurrentEntity.SetType(Dt::SPluginType::ARControlManager);

        // -----------------------------------------------------------------------------
        // Create facet and set it
        // -----------------------------------------------------------------------------
        Dt::STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = 1280;
        TextureDescriptor.m_NumberOfPixelsV  = 720;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_Format           = Dt::CTextureBase::R8G8B8_UBYTE;
        TextureDescriptor.m_Semantic         = Dt::CTextureBase::Diffuse;
        TextureDescriptor.m_Binding          = Dt::CTextureBase::ShaderResource;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pIdentifier      = "AR_BACKGROUND_TEXTURE";

        Dt::CTexture2D* pBackgroundTexture = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

        Dt::TextureManager::MarkTextureAsDirty(pBackgroundTexture, Dt::CTextureBase::DirtyCreate);

        TextureDescriptor.m_NumberOfPixelsU = 512;
        TextureDescriptor.m_NumberOfPixelsV = 512;
        TextureDescriptor.m_Binding         = Dt::CTextureBase::ShaderResource | Dt::CTextureBase::RenderTarget;
        TextureDescriptor.m_pIdentifier     = "AR_ENV_CUBEMAP_TEXTURE";

        Dt::CTextureCube* pTextureCubemap = Dt::TextureManager::CreateCubeTexture(TextureDescriptor);

        Dt::TextureManager::MarkTextureAsDirty(pTextureCubemap, Dt::CTextureBase::DirtyCreate);

        // -----------------------------------------------------------------------------

        Dt::CARControllerPluginFacet* pFacet = Dt::ARControllerManager::CreateARControllerPlugin();

        pFacet->SetCameraEntity       (0);
        pFacet->SetConfiguration      ("-device=WinDS -flipV");
        pFacet->SetCameraParameterFile("ar/configurations/logitech_para.dat");
        pFacet->SetOutputBackground   (pBackgroundTexture);
        pFacet->SetOutputCubemap      (pTextureCubemap);
        pFacet->SetDeviceType         (Dt::CARControllerPluginFacet::Webcam);
        pFacet->SetNumberOfMarker     (1);
            
        Dt::CARControllerPluginFacet::SMarker& rMarkerOne = pFacet->GetMarker(0);

        rMarkerOne.m_UID          = 0;
        rMarkerOne.m_Type         = Dt::CARControllerPluginFacet::SMarker::Square;
        rMarkerOne.m_WidthInMeter = 0.08f;
        rMarkerOne.m_PatternFile  = "ar/patterns/patt.hiro";

        rCurrentEntity.SetDetailFacet(Dt::SFacetCategory::Data, pFacet);
    }

    // -----------------------------------------------------------------------------

    void CPluginHelper::OnRequestPluginInfoARController(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CARControllerPluginFacet* pFacet = static_cast<Dt::CARControllerPluginFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Plugin && rCurrentEntity.GetType() == Dt::SPluginType::ARControlManager && pFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read value
            // -----------------------------------------------------------------------------
            int Device = pFacet->GetDeviceType();

            const char* pConfiguration = pFacet->GetConfiguration();

            const char* pParameterFile = pFacet->GetCameraParameterFile();

            int CameraEntityID = -1;

            if (pFacet->GetCameraEntity() != nullptr)
            {
                CameraEntityID = pFacet->GetCameraEntity()->GetID();
            }

            unsigned int OutputBackground = pFacet->GetOutputBackground()->GetHash();

            unsigned int OutputCubemap = pFacet->GetOutputCubemap()->GetHash();

            unsigned int NumberOfMarker = pFacet->GetNumberOfMarker();

            // -----------------------------------------------------------------------------
            // Send message
            // -----------------------------------------------------------------------------
            Edit::CMessage NewMessage;

            NewMessage.PutInt(rCurrentEntity.GetID());

            NewMessage.PutInt(Device);

            NewMessage.PutString(pConfiguration);
           
            NewMessage.PutString(pParameterFile);

            NewMessage.PutInt(CameraEntityID);

            NewMessage.PutInt(OutputBackground);

            NewMessage.PutInt(OutputCubemap);

            NewMessage.PutInt(NumberOfMarker);

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Plugin_ARController_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CPluginHelper::OnRequestPluginInfoARControllerMarker(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CARControllerPluginFacet* pFacet = static_cast<Dt::CARControllerPluginFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Plugin && rCurrentEntity.GetType() == Dt::SPluginType::ARControlManager && pFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read value
            // -----------------------------------------------------------------------------
            int MarkerID = pFacet->GetDeviceType();

            Dt::CARControllerPluginFacet::SMarker& rMarker = pFacet->GetMarker(MarkerID);

            unsigned int UID = rMarker.m_UID;

            unsigned int Type = rMarker.m_Type;

            const char* pPatternFile = rMarker.m_PatternFile.GetConst();

            float Width = rMarker.m_WidthInMeter;

            // -----------------------------------------------------------------------------
            // Send message
            // -----------------------------------------------------------------------------
            Edit::CMessage NewMessage;

            NewMessage.PutInt(rCurrentEntity.GetID());

            NewMessage.PutInt(MarkerID);

            NewMessage.PutInt(UID);

            NewMessage.PutInt(Type);

            NewMessage.PutString(pPatternFile);

            NewMessage.PutFloat(Width);

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Plugin_ARController_Marker_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CPluginHelper::OnPluginInfoARController(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));
        Dt::CARControllerPluginFacet* pFacet = static_cast<Dt::CARControllerPluginFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Plugin && rCurrentEntity.GetType() == Dt::SPluginType::ARControlManager && pFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            int Device = _rMessage.GetInt();

            char Configuration[256];

            _rMessage.GetString(Configuration, 256);

            char ParameterFile[256];

            _rMessage.GetString(ParameterFile, 256);

            unsigned int CameraEntityID = _rMessage.GetInt();

            Dt::CEntity& rCameraEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(CameraEntityID));

            unsigned int NumberOfMarker = _rMessage.GetInt();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pFacet->SetDeviceType(static_cast<Dt::CARControllerPluginFacet::EType>(Device));

            pFacet->SetConfiguration(Configuration);

            pFacet->SetCameraParameterFile(ParameterFile);

            if (rCameraEntity.GetCategory() == Dt::SEntityCategory::Actor && rCameraEntity.GetType() == Dt::SActorType::Camera)
            {
                pFacet->SetCameraEntity(&rCameraEntity);
            }

            pFacet->SetNumberOfMarker(NumberOfMarker);
            

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
        }
    }

    // -----------------------------------------------------------------------------

    void CPluginHelper::OnPluginInfoARControllerMarker(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));
        Dt::CARControllerPluginFacet* pFacet = static_cast<Dt::CARControllerPluginFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Plugin && rCurrentEntity.GetType() == Dt::SPluginType::ARControlManager && pFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            int MarkerID = pFacet->GetDeviceType();

            unsigned int UID = _rMessage.GetInt();

            unsigned int Type = _rMessage.GetInt();

            char Text[256];

            _rMessage.GetString(Text, 256);

            float Width = _rMessage.GetFloat();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            Dt::CARControllerPluginFacet::SMarker& rMarker = pFacet->GetMarker(MarkerID);

            rMarker.m_UID = UID;

            rMarker.m_Type = static_cast<Dt::CARControllerPluginFacet::SMarker::EMarkerType>(Type);

            rMarker.m_PatternFile = Text;

            rMarker.m_WidthInMeter = Width;

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
        }
    }

    // -----------------------------------------------------------------------------

    void CPluginHelper::OnDirtyEntity(Dt::CEntity* _pEntity)
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
namespace Plugin
{
    void OnStart()
    {
        CPluginHelper::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CPluginHelper::GetInstance().OnExit();
    }
} // namespace Plugin
} // namespace Helper
} // namespace Edit