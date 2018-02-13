
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_area_light_component.h"
#include "data/data_component_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_light_probe_component.h"
#include "data/data_map.h"
#include "data/data_point_light_component.h"
#include "data/data_sky_component.h"
#include "data/data_sun_component.h"
#include "data/data_texture_manager.h"
#include "data/data_transformation_facet.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

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
            int EntityID = _rMessage.GetInt();

            Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

            rCurrentEntity.SetCategory(Dt::SEntityCategory::Dynamic);

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

            rCurrentEntity.AddComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pComponent, Dt::CPointLightComponent::DirtyCreate);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnNewSun(Edit::CMessage& _rMessage)
    {
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
            Dt::CSunComponent* pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CSunComponent>();

            pComponent->EnableTemperature(false);
            pComponent->SetColor         (glm::vec3(1.0f, 1.0f, 1.0f));
            pComponent->SetDirection     (glm::vec3(0.01f, 0.01f, -1.0f));
            pComponent->SetIntensity     (90600.0f);
            pComponent->SetTemperature   (0);
            pComponent->SetRefreshMode   (Dt::CSunComponent::Dynamic);

            pComponent->UpdateLightness();

            rCurrentEntity.AddComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pComponent, Dt::CSunComponent::DirtyCreate);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnNewEnvironment(Edit::CMessage& _rMessage)
    {
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

            TextureDescriptor.m_NumberOfPixelsU = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
            TextureDescriptor.m_NumberOfPixelsV = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_Format          = Dt::CTextureBase::R16G16B16_FLOAT;
            TextureDescriptor.m_Semantic        = Dt::CTextureBase::HDR;
            TextureDescriptor.m_Binding         = Dt::CTextureBase::ShaderResource;
            TextureDescriptor.m_pPixels         = 0;
            TextureDescriptor.m_pFileName       = "environments/PaperMill_E_3k.hdr";
            TextureDescriptor.m_pIdentifier     = 0;

            Dt::CTexture2D* pPanoramaTexture = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

            Dt::TextureManager::MarkTextureAsDirty(pPanoramaTexture, Dt::CTextureBase::DirtyCreate);

            // -----------------------------------------------------------------------------

            Dt::CSkyComponent* pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CSkyComponent>();

            pComponent->SetRefreshMode(Dt::CSkyComponent::Static);
            pComponent->SetType       (Dt::CSkyComponent::Panorama);
            pComponent->SetPanorama   (pPanoramaTexture);
            pComponent->SetIntensity  (5000.0f);

            rCurrentEntity.AddComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pComponent, Dt::CSkyComponent::DirtyCreate);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnNewLightProbe(Edit::CMessage& _rMessage)
    {
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

            rCurrentEntity.AddComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pComponent, Dt::CLightProbeComponent::DirtyCreate);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnNewArealight(Edit::CMessage& _rMessage)
    {
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

            rCurrentEntity.AddComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pComponent, Dt::CAreaLightComponent::DirtyCreate);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnRequestInfoPointlight(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CPointLightComponent* pPointLightFacet = rCurrentEntity.GetComponent<Dt::CPointLightComponent>();

        if (pPointLightFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(static_cast<int>(rCurrentEntity.GetID()));
            NewMessage.PutInt(static_cast<int>(pPointLightFacet->HasTemperature()));
            NewMessage.PutFloat(pPointLightFacet->GetColor()[0]);
            NewMessage.PutFloat(pPointLightFacet->GetColor()[1]);
            NewMessage.PutFloat(pPointLightFacet->GetColor()[2]);
            NewMessage.PutFloat(pPointLightFacet->GetTemperature());
            NewMessage.PutFloat(pPointLightFacet->GetIntensity());
            NewMessage.PutFloat(pPointLightFacet->GetAttenuationRadius());
            NewMessage.PutFloat(glm::degrees(pPointLightFacet->GetInnerConeAngle()));
            NewMessage.PutFloat(glm::degrees(pPointLightFacet->GetOuterConeAngle()));
            NewMessage.PutFloat(pPointLightFacet->GetDirection()[0]);
            NewMessage.PutFloat(pPointLightFacet->GetDirection()[1]);
            NewMessage.PutFloat(pPointLightFacet->GetDirection()[2]);
            NewMessage.PutInt(static_cast<int>(pPointLightFacet->GetShadowType()));
            NewMessage.PutInt(static_cast<int>(pPointLightFacet->GetShadowQuality()));
            NewMessage.PutInt(static_cast<int>(pPointLightFacet->GetRefreshMode()));

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Light_Pointlight_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnRequestInfoSun(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CSunComponent* pLightFacet = rCurrentEntity.GetComponent<Dt::CSunComponent>();

        if (pLightFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(static_cast<int>(rCurrentEntity.GetID()));
            NewMessage.PutInt(static_cast<int>(pLightFacet->HasTemperature()));
            NewMessage.PutFloat(pLightFacet->GetColor()[0]);
            NewMessage.PutFloat(pLightFacet->GetColor()[1]);
            NewMessage.PutFloat(pLightFacet->GetColor()[2]);
            NewMessage.PutFloat(pLightFacet->GetTemperature());
            NewMessage.PutFloat(pLightFacet->GetIntensity());
            NewMessage.PutFloat(pLightFacet->GetDirection()[0]);
            NewMessage.PutFloat(pLightFacet->GetDirection()[1]);
            NewMessage.PutFloat(pLightFacet->GetDirection()[2]);
            NewMessage.PutInt(static_cast<int>(pLightFacet->GetRefreshMode()));

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Light_Sun_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnRequestInfoEnvironment(Edit::CMessage& _rMessage)
    {
        auto AddTextureToMessage = [&](const Dt::CTextureBase* _pTextureBase, Edit::CMessage& _rMessage)
        {
            if (_pTextureBase != 0)
            {
                if (_pTextureBase->GetFileName().length() > 0)
                {
                    _rMessage.PutBool(true);

                    _rMessage.PutString(_pTextureBase->GetFileName().c_str());
                }
                else
                {
                    _rMessage.PutBool(false);
                }

                _rMessage.PutInt(_pTextureBase->GetHash());
            }
            else
            {
                _rMessage.PutBool(false);

                _rMessage.PutInt(0);
            }
        };

        // -----------------------------------------------------------------------------

        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CSkyComponent* pLightFacet = rCurrentEntity.GetComponent<Dt::CSkyComponent>();

        if (pLightFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(static_cast<int>(rCurrentEntity.GetID()));
            NewMessage.PutInt(static_cast<int>(pLightFacet->GetRefreshMode()));
            NewMessage.PutInt(static_cast<int>(pLightFacet->GetType()));

            if (pLightFacet->GetType() == Dt::CSkyComponent::Procedural)
            {
                NewMessage.PutBool(false);

                NewMessage.PutInt(0);
            }
            else if (pLightFacet->GetType() == Dt::CSkyComponent::Panorama)
            {
                NewMessage.PutBool(true);

                AddTextureToMessage(pLightFacet->GetPanorama(), NewMessage);
            }
            else if (pLightFacet->GetType() == Dt::CSkyComponent::Cubemap)
            {
                NewMessage.PutBool(true);

                AddTextureToMessage(pLightFacet->GetCubemap(), NewMessage);
            }
            else if (pLightFacet->GetType() == Dt::CSkyComponent::Texture || pLightFacet->GetType() == Dt::CSkyComponent::TextureGeometry || pLightFacet->GetType() == Dt::CSkyComponent::TextureLUT)
            {
                NewMessage.PutBool(true);

                AddTextureToMessage(pLightFacet->GetTexture(), NewMessage);
            }

            NewMessage.PutFloat (pLightFacet->GetIntensity());
                
            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Light_Environment_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------
    
    void CLightHelper::OnRequestInfoLightProbe(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CLightProbeComponent* pLightFacet = rCurrentEntity.GetComponent<Dt::CLightProbeComponent>();

        if (pLightFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(static_cast<int>(rCurrentEntity.GetID()));
            NewMessage.PutInt(pLightFacet->GetRefreshMode());
            NewMessage.PutInt(pLightFacet->GetType());
            NewMessage.PutInt(pLightFacet->GetQuality());
            NewMessage.PutInt(pLightFacet->GetClearFlag());
            NewMessage.PutFloat(pLightFacet->GetIntensity());
            NewMessage.PutFloat(pLightFacet->GetNear());
            NewMessage.PutFloat(pLightFacet->GetFar());
            NewMessage.PutBool(pLightFacet->GetParallaxCorrection());
            NewMessage.PutFloat(pLightFacet->GetBoxSize()[0]);
            NewMessage.PutFloat(pLightFacet->GetBoxSize()[1]);
            NewMessage.PutFloat(pLightFacet->GetBoxSize()[2]);

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Light_Probe_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnRequestInfoArealight(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CAreaLightComponent* pLightFacet = rCurrentEntity.GetComponent<Dt::CAreaLightComponent>();

        if (pLightFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(static_cast<int>(rCurrentEntity.GetID()));
            NewMessage.PutInt(static_cast<int>(pLightFacet->HasTemperature()));
            NewMessage.PutFloat(pLightFacet->GetColor()[0]);
            NewMessage.PutFloat(pLightFacet->GetColor()[1]);
            NewMessage.PutFloat(pLightFacet->GetColor()[2]);
            NewMessage.PutFloat(pLightFacet->GetTemperature());
            NewMessage.PutFloat(pLightFacet->GetIntensity());
            NewMessage.PutFloat(glm::degrees(pLightFacet->GetRotation()));
            NewMessage.PutFloat(pLightFacet->GetWidth());
            NewMessage.PutFloat(pLightFacet->GetHeight());
            NewMessage.PutBool(pLightFacet->GetIsTwoSided());
            NewMessage.PutFloat(pLightFacet->GetDirection()[0]);
            NewMessage.PutFloat(pLightFacet->GetDirection()[1]);
            NewMessage.PutFloat(pLightFacet->GetDirection()[2]);

            if (pLightFacet->GetHasTexture())
            {
                NewMessage.PutBool(true);

                NewMessage.PutString(pLightFacet->GetTexture()->GetFileName().c_str());

                NewMessage.PutInt(pLightFacet->GetTexture()->GetHash());
            }
            else
            {
                NewMessage.PutBool(false);
            }

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Light_Arealight_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnInfoPointlight(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CPointLightComponent* pLightFacet = rCurrentEntity.GetComponent<Dt::CPointLightComponent>();

        if (pLightFacet != nullptr)
        {
            float R, G, B;
            float X, Y, Z;

            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            int ColorMode = _rMessage.GetInt();

            R = _rMessage.GetFloat();
            G = _rMessage.GetFloat();
            B = _rMessage.GetFloat();

            glm::vec3 Color = glm::vec3(R, G, B);

            float Temperature       = _rMessage.GetFloat();
            float Intensity         = _rMessage.GetFloat();
            float AttenuationRadius = _rMessage.GetFloat();
            float InnerConeAngle    = glm::radians(_rMessage.GetFloat());
            float OuterConeAngle    = glm::radians(_rMessage.GetFloat());

            X = _rMessage.GetFloat();
            Y = _rMessage.GetFloat();
            Z = _rMessage.GetFloat();

            glm::vec3 Direction = glm::vec3(X, Y, Z);

            int ShadowType    = _rMessage.GetInt();
            int ShadowQuality = _rMessage.GetInt();
            int ShadowRefresh = _rMessage.GetInt();

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

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pLightFacet, Dt::CPointLightComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnInfoSun(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CSunComponent* pLightFacet = rCurrentEntity.GetComponent<Dt::CSunComponent>();

        if (pLightFacet != nullptr)
        {
            float R, G, B;
            float X, Y, Z;

            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            int ColorMode = _rMessage.GetInt();

            R = _rMessage.GetFloat();
            G = _rMessage.GetFloat();
            B = _rMessage.GetFloat();

            glm::vec3 Color = glm::vec3(R, G, B);

            float Temperature = _rMessage.GetFloat();
            float Intensity = _rMessage.GetFloat();

            X = _rMessage.GetFloat();
            Y = _rMessage.GetFloat();
            Z = _rMessage.GetFloat();

            glm::vec3 Direction = glm::vec3(X, Y, Z);

            int ShadowRefresh = _rMessage.GetInt();

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

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pLightFacet, Dt::CSunComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnInfoEnvironment(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CSkyComponent* pLightFacet = rCurrentEntity.GetComponent<Dt::CSkyComponent>();

        if (pLightFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            int RefreshMode = _rMessage.GetInt();

            int Type = _rMessage.GetInt();

            unsigned int TextureHash = _rMessage.GetInt();

            float Intensity = _rMessage.GetFloat();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pLightFacet->SetRefreshMode(static_cast<Dt::CSkyComponent::ERefreshMode>(RefreshMode));
            pLightFacet->SetType       (static_cast<Dt::CSkyComponent::EType>(Type));
            pLightFacet->SetIntensity  (Intensity);

            if (pLightFacet->GetType() == Dt::CSkyComponent::Cubemap)
            {
                Dt::CTextureCube* pTextureCube = Dt::TextureManager::GetTextureCubeByHash(TextureHash);

                if (pTextureCube != nullptr)
                {
                    pLightFacet->SetCubemap(pTextureCube);
                }
            }
            else if (pLightFacet->GetType() == Dt::CSkyComponent::Panorama)
            {
                Dt::CTexture2D* pTexturePanorama = Dt::TextureManager::GetTexture2DByHash(TextureHash);

                if (pTexturePanorama != nullptr)
                {
                    pLightFacet->SetPanorama(pTexturePanorama);
                }
            }
            else if (pLightFacet->GetType() == Dt::CSkyComponent::Texture || pLightFacet->GetType() == Dt::CSkyComponent::TextureGeometry || pLightFacet->GetType() == Dt::CSkyComponent::TextureLUT)
            {
                Dt::CTexture2D* pTexture = Dt::TextureManager::GetTexture2DByHash(TextureHash);

                if (pTexture != nullptr)
                {
                    pLightFacet->SetTexture(pTexture);
                }
            }

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pLightFacet, Dt::CSkyComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnInfoLightProbe(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CLightProbeComponent* pLightFacet = rCurrentEntity.GetComponent<Dt::CLightProbeComponent>();

        if (pLightFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            int RefreshMode = _rMessage.GetInt();

            int Type = _rMessage.GetInt();

            int Quality = _rMessage.GetInt();

            int ClearFlag = _rMessage.GetInt();

            float Intensity = _rMessage.GetFloat();

            float Near = _rMessage.GetFloat();

            float Far = _rMessage.GetFloat();

            bool ParallaxCorrection = _rMessage.GetBool();

            float BoxSizeX = _rMessage.GetFloat();

            float BoxSizeY = _rMessage.GetFloat();

            float BoxSizeZ = _rMessage.GetFloat();

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

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pLightFacet, Dt::CLightProbeComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnInfoArealight(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CAreaLightComponent* pLightFacet = rCurrentEntity.GetComponent<Dt::CAreaLightComponent>();

        if (pLightFacet != nullptr)
        {
            float R, G, B;
            float X, Y, Z;
            unsigned int TextureHash;

            TextureHash = 0;

            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            int ColorMode = _rMessage.GetInt();

            R = _rMessage.GetFloat();
            G = _rMessage.GetFloat();
            B = _rMessage.GetFloat();

            glm::vec3 Color = glm::vec3(R, G, B);

            float Temperature = _rMessage.GetFloat();
            float Intensity   = _rMessage.GetFloat();
            float Rotation    = glm::radians(_rMessage.GetFloat());
            float Width       = _rMessage.GetFloat();
            float Height      = _rMessage.GetFloat();
            bool  IsTwoSided  = _rMessage.GetBool();

            X = _rMessage.GetFloat();
            Y = _rMessage.GetFloat();
            Z = _rMessage.GetFloat();

            glm::vec3 Direction = glm::vec3(X, Y, Z);

            bool HasTexture = _rMessage.GetBool();

            if (HasTexture)
            {
                TextureHash = _rMessage.GetInt();
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
                Dt::CTexture2D* pTexture = Dt::TextureManager::GetTexture2DByHash(TextureHash);

                if (pTexture != nullptr)
                {
                    pLightFacet->SetTexture(pTexture);
                }
            }
            else
            {
                pLightFacet->SetTexture(0);
            }
            
            pLightFacet->UpdateLightness();

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pLightFacet, Dt::CAreaLightComponent::DirtyInfo);
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