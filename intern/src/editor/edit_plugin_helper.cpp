
#include "editor/edit_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_console.h"

#include "data/data_ar_controller_component.h"
#include "data/data_ar_tracked_object_component.h"
#include "data/data_camera_component.h"
#include "data/data_component.h"
#include "data/data_component_facet.h"
#include "data/data_component_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"

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
        BASE_UNUSED(_rMessage);
    }

    // -----------------------------------------------------------------------------

    void CPluginHelper::OnRequestPluginInfoARController(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CARControllerPluginComponent* pFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CARControllerPluginComponent>();

        if (pFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read value
            // -----------------------------------------------------------------------------
            int Device = pFacet->GetDeviceType();

            bool FreezeOutput = pFacet->GetFreezeLastFrame();

            std::string Configuration = pFacet->GetConfiguration();

            std::string ParameterFile = pFacet->GetCameraParameterFile();

            Base::ID CameraEntityID = static_cast<Base::ID>(-1);

            if (pFacet->GetCameraEntity() != nullptr)
            {
                CameraEntityID = pFacet->GetCameraEntity()->GetID();
            }

            unsigned int NumberOfMarker = pFacet->GetNumberOfMarker();

            // -----------------------------------------------------------------------------
            // Send message
            // -----------------------------------------------------------------------------
            Edit::CMessage NewMessage;

            NewMessage.Put(pCurrentEntity->GetID());

            NewMessage.Put(Device);

            NewMessage.Put(FreezeOutput);

            NewMessage.Put(Configuration);
           
            NewMessage.Put(ParameterFile);

            NewMessage.Put(static_cast<int>(CameraEntityID));

            NewMessage.Put(NumberOfMarker);

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Plugin_ARController_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CPluginHelper::OnRequestPluginInfoARControllerMarker(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CARControllerPluginComponent* pFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CARControllerPluginComponent>();

        if (pFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read value
            // -----------------------------------------------------------------------------
            int MarkerID = pFacet->GetDeviceType();

            Dt::CARControllerPluginComponent::SMarker& rMarker = pFacet->GetMarker(MarkerID);

            unsigned int UID = rMarker.m_UID;

            unsigned int Type = rMarker.m_Type;

            std::string PatternFile = rMarker.m_PatternFile;

            float Width = rMarker.m_WidthInMeter;

            // -----------------------------------------------------------------------------
            // Send message
            // -----------------------------------------------------------------------------
            Edit::CMessage NewMessage;

            NewMessage.Put(pCurrentEntity->GetID());

            NewMessage.Put(MarkerID);

            NewMessage.Put(UID);

            NewMessage.Put(Type);

            NewMessage.Put(PatternFile);

            NewMessage.Put(Width);

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Plugin_ARController_Marker_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CPluginHelper::OnPluginInfoARController(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);
        
        Dt::CARControllerPluginComponent* pFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CARControllerPluginComponent>();

        if (pFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            int Device = _rMessage.Get<int>();

            bool FreezeOutput = _rMessage.Get<bool>();

            std::string Configuration = _rMessage.Get<std::string>();

            std::string ParameterFile = _rMessage.Get<std::string>();

            unsigned int CameraEntityID = _rMessage.Get<int>();

            Dt::CEntity* pCameraEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(CameraEntityID));

            unsigned int NumberOfMarker = _rMessage.Get<int>();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pFacet->SetDeviceType(static_cast<Dt::CARControllerPluginComponent::EType>(Device));

            pFacet->SetFreezeOutput(FreezeOutput);

            pFacet->SetConfiguration(Configuration);

            pFacet->SetCameraParameterFile(ParameterFile);

            if (pCameraEntity->GetComponentFacet()->HasComponent<Dt::CCameraComponent>())
            {
                pFacet->SetCameraEntity(pCameraEntity);
            }

            pFacet->SetNumberOfMarker(NumberOfMarker);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pFacet, Dt::CARControllerPluginComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CPluginHelper::OnPluginInfoARControllerMarker(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CARControllerPluginComponent* pFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CARControllerPluginComponent>();

        if (pFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            int MarkerID = pFacet->GetDeviceType();

            unsigned int UID = _rMessage.Get<int>();

            unsigned int Type = _rMessage.Get<int>();

            std::string Text = _rMessage.Get<std::string>();

            float Width = _rMessage.Get<float>();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            Dt::CARControllerPluginComponent::SMarker& rMarker = pFacet->GetMarker(MarkerID);

            rMarker.m_UID = UID;

            rMarker.m_Type = static_cast<Dt::CARControllerPluginComponent::SMarker::EMarkerType>(Type);

            rMarker.m_PatternFile = Text;

            rMarker.m_WidthInMeter = Width;

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pFacet, Dt::CARControllerPluginComponent::DirtyInfo);
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