
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_ar_controller_component.h"
#include "data/data_ar_tracked_object_component.h"
#include "data/data_camera_component.h"
#include "data/data_component_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
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

        rCurrentEntity.SetCategory(Dt::SEntityCategory::Dynamic);

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

        // -----------------------------------------------------------------------------

        Dt::CARControllerPluginComponent* pFacet = Dt::CComponentManager::GetInstance().Allocate<Dt::CARControllerPluginComponent>();

        pFacet->SetCameraEntity       (0);
        pFacet->SetConfiguration      ("-device=WinDS -flipV");
        pFacet->SetCameraParameterFile("ar/configurations/logitech_para.dat");
        pFacet->SetOutputBackground   (pBackgroundTexture);
        pFacet->SetDeviceType         (Dt::CARControllerPluginComponent::Webcam);
        pFacet->SetNumberOfMarker     (1);
        pFacet->SetFreezeOutput       (false);
            
        Dt::CARControllerPluginComponent::SMarker& rMarkerOne = pFacet->GetMarker(0);

        rMarkerOne.m_UID          = 0;
        rMarkerOne.m_Type         = Dt::CARControllerPluginComponent::SMarker::Square;
        rMarkerOne.m_WidthInMeter = 0.08f;
        rMarkerOne.m_PatternFile  = "ar/patterns/patt.hiro";

        rCurrentEntity.AddComponent(pFacet);

        Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pFacet, Dt::CARControllerPluginComponent::DirtyCreate);
    }

    // -----------------------------------------------------------------------------

    void CPluginHelper::OnRequestPluginInfoARController(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CARControllerPluginComponent* pFacet = rCurrentEntity.GetComponent<Dt::CARControllerPluginComponent>();

        if (pFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read value
            // -----------------------------------------------------------------------------
            int Device = pFacet->GetDeviceType();

            bool FreezeOutput = pFacet->GetFreezeLastFrame();

            const char* pConfiguration = pFacet->GetConfiguration().c_str();

            const char* pParameterFile = pFacet->GetCameraParameterFile().c_str();

            Base::ID CameraEntityID = static_cast<Base::ID>(-1);

            if (pFacet->GetCameraEntity() != nullptr)
            {
                CameraEntityID = pFacet->GetCameraEntity()->GetID();
            }

            unsigned int OutputBackground = pFacet->GetOutputBackground()->GetHash();

            unsigned int NumberOfMarker = pFacet->GetNumberOfMarker();

            // -----------------------------------------------------------------------------
            // Send message
            // -----------------------------------------------------------------------------
            Edit::CMessage NewMessage;

            NewMessage.PutInt(static_cast<int>(rCurrentEntity.GetID()));

            NewMessage.PutInt(Device);

            NewMessage.PutBool(FreezeOutput);

            NewMessage.PutString(pConfiguration);
           
            NewMessage.PutString(pParameterFile);

            NewMessage.PutInt(static_cast<int>(CameraEntityID));

            NewMessage.PutInt(OutputBackground);

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

        Dt::CARControllerPluginComponent* pFacet = rCurrentEntity.GetComponent<Dt::CARControllerPluginComponent>();

        if (pFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read value
            // -----------------------------------------------------------------------------
            int MarkerID = pFacet->GetDeviceType();

            Dt::CARControllerPluginComponent::SMarker& rMarker = pFacet->GetMarker(MarkerID);

            unsigned int UID = rMarker.m_UID;

            unsigned int Type = rMarker.m_Type;

            const char* pPatternFile = rMarker.m_PatternFile.c_str();

            float Width = rMarker.m_WidthInMeter;

            // -----------------------------------------------------------------------------
            // Send message
            // -----------------------------------------------------------------------------
            Edit::CMessage NewMessage;

            NewMessage.PutInt(static_cast<int>(rCurrentEntity.GetID()));

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
        
        Dt::CARControllerPluginComponent* pFacet = rCurrentEntity.GetComponent<Dt::CARControllerPluginComponent>();

        if (pFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            int Device = _rMessage.GetInt();

            bool FreezeOutput = _rMessage.GetBool();

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
            pFacet->SetDeviceType(static_cast<Dt::CARControllerPluginComponent::EType>(Device));

            pFacet->SetFreezeOutput(FreezeOutput);

            pFacet->SetConfiguration(Configuration);

            pFacet->SetCameraParameterFile(ParameterFile);

            if (rCameraEntity.HasComponent<Dt::CCameraComponent>())
            {
                pFacet->SetCameraEntity(&rCameraEntity);
            }

            pFacet->SetNumberOfMarker(NumberOfMarker);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pFacet, Dt::CARControllerPluginComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CPluginHelper::OnPluginInfoARControllerMarker(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CARControllerPluginComponent* pFacet = rCurrentEntity.GetComponent<Dt::CARControllerPluginComponent>();

        if (pFacet != nullptr)
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
            Dt::CARControllerPluginComponent::SMarker& rMarker = pFacet->GetMarker(MarkerID);

            rMarker.m_UID = UID;

            rMarker.m_Type = static_cast<Dt::CARControllerPluginComponent::SMarker::EMarkerType>(Type);

            rMarker.m_PatternFile = Text;

            rMarker.m_WidthInMeter = Width;

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pFacet, Dt::CARControllerPluginComponent::DirtyInfo);
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