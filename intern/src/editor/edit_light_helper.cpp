
#include "editor/edit_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_console.h"

#include "engine/data/data_area_light_component.h"
#include "engine/data/data_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_hierarchy_facet.h"
#include "engine/data/data_light_probe_component.h"
#include "engine/data/data_map.h"
#include "engine/data/data_point_light_component.h"
#include "engine/data/data_sky_component.h"
#include "engine/data/data_sun_component.h"
#include "engine/data/data_transformation_facet.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <string>

namespace
{
    class CLightHelper : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CLightHelper)

    public:

        CLightHelper();
        ~CLightHelper();

    public:

        void OnStart();
        void OnExit();

    private:

        void OnNewPointlight(Edit::CMessage& _rMessage);
        void OnNewSun(Edit::CMessage& _rMessage);
        void OnNewEnvironment(Edit::CMessage& _rMessage);
        void OnNewLightProbe(Edit::CMessage& _rMessage);
        void OnNewArealight(Edit::CMessage& _rMessage);

        void OnRequestInfoPointlight(Edit::CMessage& _rMessage);
        void OnRequestInfoSun(Edit::CMessage& _rMessage);
        void OnRequestInfoEnvironment(Edit::CMessage& _rMessage);
        void OnRequestInfoLightProbe(Edit::CMessage& _rMessage);
        void OnRequestInfoArealight(Edit::CMessage& _rMessage);

        void OnInfoPointlight(Edit::CMessage& _rMessage);
        void OnInfoSun(Edit::CMessage& _rMessage);
        void OnInfoEnvironment(Edit::CMessage& _rMessage);
        void OnInfoLightProbe(Edit::CMessage& _rMessage);
        void OnInfoArealight(Edit::CMessage& _rMessage);

        void OnDirtyEntity(Dt::CEntity* _pEntity);
    };
} // namespace

namespace
{
    CLightHelper::CLightHelper()
    {
        
    }

    // -----------------------------------------------------------------------------

    CLightHelper::~CLightHelper()
    {

    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Entity
        // -----------------------------------------------------------------------------
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CLightHelper::OnDirtyEntity));

        // -----------------------------------------------------------------------------
        // Edit
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SGUIMessageType::Light_Pointlight_New , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnNewPointlight));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Light_Sun_New        , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnNewSun));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Light_Environment_New, EDIT_RECEIVE_MESSAGE(&CLightHelper::OnNewEnvironment));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Light_Probe_New      , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnNewLightProbe));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Light_Arealight_New  , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnNewArealight));
        
        Edit::MessageManager::Register(Edit::SGUIMessageType::Light_Pointlight_Info , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnRequestInfoPointlight));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Light_Sun_Info        , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnRequestInfoSun));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Light_Environment_Info, EDIT_RECEIVE_MESSAGE(&CLightHelper::OnRequestInfoEnvironment));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Light_Probe_Info      , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnRequestInfoLightProbe));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Light_Arealight_Info  , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnRequestInfoArealight));
       
        Edit::MessageManager::Register(Edit::SGUIMessageType::Light_Pointlight_Update , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnInfoPointlight));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Light_Sun_Update        , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnInfoSun));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Light_Environment_Update, EDIT_RECEIVE_MESSAGE(&CLightHelper::OnInfoEnvironment));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Light_Probe_Update      , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnInfoLightProbe));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Light_Arealight_Update  , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnInfoArealight));
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnNewPointlight(Edit::CMessage& _rMessage)
    {
        {
            // -----------------------------------------------------------------------------
            // Get entity and set type + category
            // -----------------------------------------------------------------------------
            Base::ID EntityID = _rMessage.Get<Base::ID>();

            Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            // -----------------------------------------------------------------------------
            // Create facet and set it
            // -----------------------------------------------------------------------------
            Dt::CPointLightComponent* pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CPointLightComponent>();

            pComponent->SetRefreshMode      (Dt::CPointLightComponent::Static);
            pComponent->SetShadowType       (Dt::CPointLightComponent::HardShadows);
            pComponent->SetShadowQuality    (Dt::CPointLightComponent::High);
            pComponent->EnableTemperature   (false);
            pComponent->SetColor            (glm::vec3(1.0f, 1.0f, 1.0f));
            pComponent->SetAttenuationRadius(10.0f);
            pComponent->SetInnerConeAngle   (glm::radians(45.0f));
            pComponent->SetOuterConeAngle   (glm::radians(90.0f));
            pComponent->SetDirection        (glm::vec3(-1.0f, -1.0f, -1.0f));
            pComponent->SetIntensity        (1200.0f);
            pComponent->SetTemperature      (0);

            pComponent->UpdateLightness();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CPointLightComponent::DirtyCreate);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnNewSun(Edit::CMessage& _rMessage)
    {
        {
            // -----------------------------------------------------------------------------
            // Get entity and set type + category
            // -----------------------------------------------------------------------------
            Base::ID EntityID = _rMessage.Get<Base::ID>();

            Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            // -----------------------------------------------------------------------------
            // Create facet and set it
            // -----------------------------------------------------------------------------
            Dt::CSunComponent* pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CSunComponent>();

            pComponent->EnableTemperature(false);
            pComponent->SetColor         (glm::vec3(1.0f, 1.0f, 1.0f));
            pComponent->SetDirection     (glm::vec3(0.01f, 0.01f, -1.0f));
            pComponent->SetIntensity     (90600.0f);
            pComponent->SetTemperature   (0);
            pComponent->SetRefreshMode   (Dt::CSunComponent::Dynamic);

            pComponent->UpdateLightness();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CSunComponent::DirtyCreate);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnNewEnvironment(Edit::CMessage& _rMessage)
    {
        {
            // -----------------------------------------------------------------------------
            // Get entity and set type + category
            // -----------------------------------------------------------------------------
            Base::ID EntityID = _rMessage.Get<Base::ID>();

            Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            // -----------------------------------------------------------------------------

            Dt::CSkyComponent* pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CSkyComponent>();

            pComponent->SetRefreshMode(Dt::CSkyComponent::Static);
            pComponent->SetType       (Dt::CSkyComponent::Panorama);
            pComponent->SetTexture    ("environments/PaperMill_E_3k.hdr");
            pComponent->SetIntensity  (5000.0f);

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CSkyComponent::DirtyCreate);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnNewLightProbe(Edit::CMessage& _rMessage)
    {
        {
            // -----------------------------------------------------------------------------
            // Get entity and set type + category
            // -----------------------------------------------------------------------------
            Base::ID EntityID = _rMessage.Get<Base::ID>();

            Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            // -----------------------------------------------------------------------------
            // Create facet and set it
            // -----------------------------------------------------------------------------
            Dt::CLightProbeComponent* pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CLightProbeComponent>();

            pComponent->SetRefreshMode       (Dt::CLightProbeComponent::Static);
            pComponent->SetType              (Dt::CLightProbeComponent::Local);
            pComponent->SetQuality           (Dt::CLightProbeComponent::PX256);
            pComponent->SetClearFlag         (Dt::CLightProbeComponent::Skybox);
            pComponent->SetIntensity         (1.0f);
            pComponent->SetNear              (0.1f);
            pComponent->SetFar               (10.0f);
            pComponent->SetParallaxCorrection(true);
            pComponent->SetBoxSize           (glm::vec3(10.0f));

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CLightProbeComponent::DirtyCreate);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnNewArealight(Edit::CMessage& _rMessage)
    {
        {
            // -----------------------------------------------------------------------------
            // Get entity and set type + category
            // -----------------------------------------------------------------------------
            Base::ID EntityID = _rMessage.Get<Base::ID>();

            Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            // -----------------------------------------------------------------------------
            // Create facet and set it
            // -----------------------------------------------------------------------------
            Dt::CAreaLightComponent* pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CAreaLightComponent>();

            pComponent->EnableTemperature   (false);
            pComponent->SetColor            (glm::vec3(1.0f, 1.0f, 1.0f));
            pComponent->SetRotation         (0.0f);
            pComponent->SetWidth            (8.0f);
            pComponent->SetHeight           (8.0f);
            pComponent->SetDirection        (glm::vec3(-0.01f, 0.01f, -1.0f));
            pComponent->SetIntensity        (1200.0f);
            pComponent->SetTemperature      (0);
            pComponent->SetIsTwoSided       (false);

            pComponent->UpdateLightness();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CAreaLightComponent::DirtyCreate);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnRequestInfoPointlight(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CPointLightComponent* pPointLightFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CPointLightComponent>();

        if (pPointLightFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.Put(pCurrentEntity->GetID());
            NewMessage.Put(static_cast<int>(pPointLightFacet->HasTemperature()));
            NewMessage.Put(pPointLightFacet->GetColor()[0]);
            NewMessage.Put(pPointLightFacet->GetColor()[1]);
            NewMessage.Put(pPointLightFacet->GetColor()[2]);
            NewMessage.Put(pPointLightFacet->GetTemperature());
            NewMessage.Put(pPointLightFacet->GetIntensity());
            NewMessage.Put(pPointLightFacet->GetAttenuationRadius());
            NewMessage.Put(glm::degrees(pPointLightFacet->GetInnerConeAngle()));
            NewMessage.Put(glm::degrees(pPointLightFacet->GetOuterConeAngle()));
            NewMessage.Put(pPointLightFacet->GetDirection()[0]);
            NewMessage.Put(pPointLightFacet->GetDirection()[1]);
            NewMessage.Put(pPointLightFacet->GetDirection()[2]);
            NewMessage.Put(static_cast<int>(pPointLightFacet->GetShadowType()));
            NewMessage.Put(static_cast<int>(pPointLightFacet->GetShadowQuality()));
            NewMessage.Put(static_cast<int>(pPointLightFacet->GetRefreshMode()));

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Light_Pointlight_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnRequestInfoSun(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CSunComponent* pLightFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CSunComponent>();

        if (pLightFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.Put(pCurrentEntity->GetID());
            NewMessage.Put(static_cast<int>(pLightFacet->HasTemperature()));
            NewMessage.Put(pLightFacet->GetColor()[0]);
            NewMessage.Put(pLightFacet->GetColor()[1]);
            NewMessage.Put(pLightFacet->GetColor()[2]);
            NewMessage.Put(pLightFacet->GetTemperature());
            NewMessage.Put(pLightFacet->GetIntensity());
            NewMessage.Put(pLightFacet->GetDirection()[0]);
            NewMessage.Put(pLightFacet->GetDirection()[1]);
            NewMessage.Put(pLightFacet->GetDirection()[2]);
            NewMessage.Put(static_cast<int>(pLightFacet->GetRefreshMode()));

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Light_Sun_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnRequestInfoEnvironment(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CSkyComponent* pLightFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CSkyComponent>();

        if (pLightFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.Put(pCurrentEntity->GetID());
            NewMessage.Put(static_cast<int>(pLightFacet->GetRefreshMode()));
            NewMessage.Put(static_cast<int>(pLightFacet->GetType()));

            if (pLightFacet->GetType() == Dt::CSkyComponent::Procedural)
            {
                NewMessage.Put(false);
            }
            else
            {
                NewMessage.Put(true);

                NewMessage.Put(pLightFacet->GetTexture());
            }

            NewMessage.Put(pLightFacet->GetIntensity());
                
            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Light_Environment_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------
    
    void CLightHelper::OnRequestInfoLightProbe(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CLightProbeComponent* pLightFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CLightProbeComponent>();

        if (pLightFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.Put(pCurrentEntity->GetID());
            NewMessage.Put(pLightFacet->GetRefreshMode());
            NewMessage.Put(pLightFacet->GetType());
            NewMessage.Put(pLightFacet->GetQuality());
            NewMessage.Put(pLightFacet->GetClearFlag());
            NewMessage.Put(pLightFacet->GetIntensity());
            NewMessage.Put(pLightFacet->GetNear());
            NewMessage.Put(pLightFacet->GetFar());
            NewMessage.Put(pLightFacet->GetParallaxCorrection());
            NewMessage.Put(pLightFacet->GetBoxSize()[0]);
            NewMessage.Put(pLightFacet->GetBoxSize()[1]);
            NewMessage.Put(pLightFacet->GetBoxSize()[2]);

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Light_Probe_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnRequestInfoArealight(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CAreaLightComponent* pLightFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CAreaLightComponent>();

        if (pLightFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.Put(pCurrentEntity->GetID());
            NewMessage.Put(static_cast<int>(pLightFacet->HasTemperature()));
            NewMessage.Put(pLightFacet->GetColor()[0]);
            NewMessage.Put(pLightFacet->GetColor()[1]);
            NewMessage.Put(pLightFacet->GetColor()[2]);
            NewMessage.Put(pLightFacet->GetTemperature());
            NewMessage.Put(pLightFacet->GetIntensity());
            NewMessage.Put(glm::degrees(pLightFacet->GetRotation()));
            NewMessage.Put(pLightFacet->GetWidth());
            NewMessage.Put(pLightFacet->GetHeight());
            NewMessage.Put(pLightFacet->GetIsTwoSided());
            NewMessage.Put(pLightFacet->GetDirection()[0]);
            NewMessage.Put(pLightFacet->GetDirection()[1]);
            NewMessage.Put(pLightFacet->GetDirection()[2]);

            if (pLightFacet->GetHasTexture())
            {
                NewMessage.Put(true);

                NewMessage.Put(pLightFacet->GetTexture());
            }
            else
            {
                NewMessage.Put(false);
            }

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Light_Arealight_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnInfoPointlight(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CPointLightComponent* pLightFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CPointLightComponent>();

        if (pLightFacet != nullptr)
        {
            float R, G, B;
            float X, Y, Z;

            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            int ColorMode = _rMessage.Get<int>();

            R = _rMessage.Get<float>();
            G = _rMessage.Get<float>();
            B = _rMessage.Get<float>();

            glm::vec3 Color = glm::vec3(R, G, B);

            float Temperature       = _rMessage.Get<float>();
            float Intensity         = _rMessage.Get<float>();
            float AttenuationRadius = _rMessage.Get<float>();
            float InnerConeAngle    = glm::radians(_rMessage.Get<float>());
            float OuterConeAngle    = glm::radians(_rMessage.Get<float>());

            X = _rMessage.Get<float>();
            Y = _rMessage.Get<float>();
            Z = _rMessage.Get<float>();

            glm::vec3 Direction = glm::vec3(X, Y, Z);

            int ShadowType    = _rMessage.Get<int>();
            int ShadowQuality = _rMessage.Get<int>();
            int ShadowRefresh = _rMessage.Get<int>();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pLightFacet->EnableTemperature   (ColorMode == 1);
            pLightFacet->SetColor            (Color);
            pLightFacet->SetTemperature      (Temperature);
            pLightFacet->SetIntensity        (Intensity);
            pLightFacet->SetAttenuationRadius(AttenuationRadius);
            pLightFacet->SetInnerConeAngle   (InnerConeAngle);
            pLightFacet->SetOuterConeAngle   (OuterConeAngle);
            pLightFacet->SetDirection        (Direction);
            pLightFacet->SetShadowType       (static_cast<Dt::CPointLightComponent::EShadowType>(ShadowType));
            pLightFacet->SetShadowQuality    (static_cast<Dt::CPointLightComponent::EShadowQuality>(ShadowQuality));
            pLightFacet->SetRefreshMode      (static_cast<Dt::CPointLightComponent::ERefreshMode>(ShadowRefresh));
            
            pLightFacet->UpdateLightness();

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pLightFacet, Dt::CPointLightComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnInfoSun(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CSunComponent* pLightFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CSunComponent>();

        if (pLightFacet != nullptr)
        {
            float R, G, B;
            float X, Y, Z;

            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            int ColorMode = _rMessage.Get<int>();

            R = _rMessage.Get<float>();
            G = _rMessage.Get<float>();
            B = _rMessage.Get<float>();

            glm::vec3 Color = glm::vec3(R, G, B);

            float Temperature = _rMessage.Get<float>();
            float Intensity = _rMessage.Get<float>();

            X = _rMessage.Get<float>();
            Y = _rMessage.Get<float>();
            Z = _rMessage.Get<float>();

            glm::vec3 Direction = glm::vec3(X, Y, Z);

            int ShadowRefresh = _rMessage.Get<int>();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pLightFacet->EnableTemperature(ColorMode == 1);
            pLightFacet->SetColor(Color);
            pLightFacet->SetTemperature(Temperature);
            pLightFacet->SetIntensity(Intensity);
            pLightFacet->SetDirection(Direction);
            pLightFacet->SetRefreshMode(static_cast<Dt::CSunComponent::ERefreshMode>(ShadowRefresh));

            pLightFacet->UpdateLightness();

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pLightFacet, Dt::CSunComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnInfoEnvironment(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CSkyComponent* pLightFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CSkyComponent>();

        if (pLightFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            int RefreshMode = _rMessage.Get<int>();

            int Type = _rMessage.Get<int>();

            std::string Texture = _rMessage.Get<std::string>();

            float Intensity = _rMessage.Get<float>();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pLightFacet->SetRefreshMode(static_cast<Dt::CSkyComponent::ERefreshMode>(RefreshMode));
            pLightFacet->SetType       (static_cast<Dt::CSkyComponent::EType>(Type));
            pLightFacet->SetIntensity  (Intensity);

            if (pLightFacet->GetType() != Dt::CSkyComponent::Procedural)
            {
                pLightFacet->SetTexture(Texture);
            }

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pLightFacet, Dt::CSkyComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnInfoLightProbe(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CLightProbeComponent* pLightFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CLightProbeComponent>();

        if (pLightFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            int RefreshMode = _rMessage.Get<int>();

            int Type = _rMessage.Get<int>();

            int Quality = _rMessage.Get<int>();

            int ClearFlag = _rMessage.Get<int>();

            float Intensity = _rMessage.Get<float>();

            float Near = _rMessage.Get<float>();

            float Far = _rMessage.Get<float>();

            bool ParallaxCorrection = _rMessage.Get<bool>();

            float BoxSizeX = _rMessage.Get<float>();

            float BoxSizeY = _rMessage.Get<float>();

            float BoxSizeZ = _rMessage.Get<float>();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pLightFacet->SetRefreshMode(static_cast<Dt::CLightProbeComponent::ERefreshMode>(RefreshMode));

            pLightFacet->SetType(static_cast<Dt::CLightProbeComponent::EType>(Type));

            pLightFacet->SetQuality(static_cast<Dt::CLightProbeComponent::EQuality>(Quality));

            pLightFacet->SetClearFlag(static_cast<Dt::CLightProbeComponent::EClearFlag>(ClearFlag));

            pLightFacet->SetIntensity(Intensity);

            pLightFacet->SetNear(Near);

            pLightFacet->SetFar(Far);

            pLightFacet->SetParallaxCorrection(ParallaxCorrection);

            pLightFacet->SetBoxSize(glm::vec3(BoxSizeX, BoxSizeY, BoxSizeZ));

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pLightFacet, Dt::CLightProbeComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnInfoArealight(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CAreaLightComponent* pLightFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CAreaLightComponent>();

        if (pLightFacet != nullptr)
        {
            float R, G, B;
            float X, Y, Z;
            std::string TextureFile;

            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            int ColorMode = _rMessage.Get<int>();

            R = _rMessage.Get<float>();
            G = _rMessage.Get<float>();
            B = _rMessage.Get<float>();

            glm::vec3 Color = glm::vec3(R, G, B);

            float Temperature = _rMessage.Get<float>();
            float Intensity   = _rMessage.Get<float>();
            float Rotation    = glm::radians(_rMessage.Get<float>());
            float Width       = _rMessage.Get<float>();
            float Height      = _rMessage.Get<float>();
            bool  IsTwoSided  = _rMessage.Get<bool>();

            X = _rMessage.Get<float>();
            Y = _rMessage.Get<float>();
            Z = _rMessage.Get<float>();

            glm::vec3 Direction = glm::vec3(X, Y, Z);

            bool HasTexture = _rMessage.Get<bool>();

            if (HasTexture)
            {
                TextureFile = _rMessage.Get<std::string>();
            }

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pLightFacet->EnableTemperature(ColorMode == 1);
            pLightFacet->SetColor         (Color);
            pLightFacet->SetTemperature   (Temperature);
            pLightFacet->SetIntensity     (Intensity);
            pLightFacet->SetRotation      (Rotation);
            pLightFacet->SetWidth         (Width);
            pLightFacet->SetHeight        (Height);
            pLightFacet->SetIsTwoSided    (IsTwoSided);
            pLightFacet->SetDirection     (Direction);

            if (HasTexture)
            {
                pLightFacet->SetTexture(TextureFile);
            }
            else
            {
                pLightFacet->SetTexture("");
            }
            
            pLightFacet->UpdateLightness();

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pLightFacet, Dt::CAreaLightComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnDirtyEntity(Dt::CEntity* _pEntity)
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
namespace Light
{
    void OnStart()
    {
        CLightHelper::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CLightHelper::GetInstance().OnExit();
    }
} // namespace Light
} // namespace Helper
} // namespace Edit