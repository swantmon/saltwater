
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_area_light_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_light_probe_manager.h"
#include "data/data_light_type.h"
#include "data/data_map.h"
#include "data/data_point_light_manager.h"
#include "data/data_sky_manager.h"
#include "data/data_sun_manager.h"
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

            rCurrentEntity.SetCategory(Dt::SEntityCategory::Light);
            rCurrentEntity.SetType(Dt::SLightType::Point);

            // -----------------------------------------------------------------------------
            // Create facet and set it
            // -----------------------------------------------------------------------------
            Dt::CPointLightFacet* pPointLightFacet = Dt::PointLightManager::CreatePointLight();

            pPointLightFacet->SetRefreshMode      (Dt::CPointLightFacet::Static);
            pPointLightFacet->SetShadowType       (Dt::CPointLightFacet::HardShadows);
            pPointLightFacet->SetShadowQuality    (Dt::CPointLightFacet::High);
            pPointLightFacet->EnableTemperature   (false);
            pPointLightFacet->SetColor            (Base::Float3(1.0f, 1.0f, 1.0f));
            pPointLightFacet->SetAttenuationRadius(10.0f);
            pPointLightFacet->SetInnerConeAngle   (Base::DegreesToRadians(45.0f));
            pPointLightFacet->SetOuterConeAngle   (Base::DegreesToRadians(90.0f));
            pPointLightFacet->SetDirection        (Base::Float3(-1.0f, -1.0f, -1.0f));
            pPointLightFacet->SetIntensity        (1200.0f);
            pPointLightFacet->SetTemperature      (0);

            pPointLightFacet->UpdateLightness();

            rCurrentEntity.SetDetailFacet(Dt::SFacetCategory::Data, pPointLightFacet);
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

            rCurrentEntity.SetCategory(Dt::SEntityCategory::Light);
            rCurrentEntity.SetType(Dt::SLightType::Sun);

            // -----------------------------------------------------------------------------
            // Create facet and set it
            // -----------------------------------------------------------------------------
            Dt::CSunLightFacet* pSunLightFacet = Dt::SunManager::CreateSunLight();

            pSunLightFacet->EnableTemperature(false);
            pSunLightFacet->SetColor         (Base::Float3(1.0f, 1.0f, 1.0f));
            pSunLightFacet->SetDirection     (Base::Float3(0.01f, 0.01f, -1.0f));
            pSunLightFacet->SetIntensity     (90600.0f);
            pSunLightFacet->SetTemperature   (0);
            pSunLightFacet->SetRefreshMode   (Dt::CSunLightFacet::Dynamic);

            pSunLightFacet->UpdateLightness();

            rCurrentEntity.SetDetailFacet(Dt::SFacetCategory::Data, pSunLightFacet);
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

            rCurrentEntity.SetCategory(Dt::SEntityCategory::Light);
            rCurrentEntity.SetType(Dt::SLightType::Sky);

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

            Dt::CSkyFacet* pSkyboxFacet = Dt::SkyManager::CreateSky();

            pSkyboxFacet->SetRefreshMode(Dt::CSkyFacet::Static);
            pSkyboxFacet->SetType       (Dt::CSkyFacet::Panorama);
            pSkyboxFacet->SetPanorama   (pPanoramaTexture);
            pSkyboxFacet->SetIntensity  (5000.0f);

            rCurrentEntity.SetDetailFacet(Dt::SFacetCategory::Data, pSkyboxFacet);
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

            rCurrentEntity.SetCategory(Dt::SEntityCategory::Light);
            rCurrentEntity.SetType(Dt::SLightType::LightProbe);

            // -----------------------------------------------------------------------------
            // Create facet and set it
            // -----------------------------------------------------------------------------
            Dt::CLightProbeFacet* pLightProbeFacet = Dt::LightProbeManager::CreateLightProbe();

            pLightProbeFacet->SetRefreshMode       (Dt::CLightProbeFacet::Static);
            pLightProbeFacet->SetType              (Dt::CLightProbeFacet::Local);
            pLightProbeFacet->SetQuality           (Dt::CLightProbeFacet::PX256);
            pLightProbeFacet->SetClearFlag         (Dt::CLightProbeFacet::Skybox);
            pLightProbeFacet->SetIntensity         (1.0f);
            pLightProbeFacet->SetNear              (0.1f);
            pLightProbeFacet->SetFar               (10.0f);
            pLightProbeFacet->SetParallaxCorrection(true);
            pLightProbeFacet->SetBoxSize           (Base::Float3(10.0f));

            rCurrentEntity.SetDetailFacet(Dt::SFacetCategory::Data, pLightProbeFacet);
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

            rCurrentEntity.SetCategory(Dt::SEntityCategory::Light);
            rCurrentEntity.SetType(Dt::SLightType::Area);

            // -----------------------------------------------------------------------------
            // Create facet and set it
            // -----------------------------------------------------------------------------
            Dt::CAreaLightFacet* pLightFacet = Dt::AreaLightManager::CreateAreaLight();

            pLightFacet->EnableTemperature   (false);
            pLightFacet->SetColor            (Base::Float3(1.0f, 1.0f, 1.0f));
            pLightFacet->SetRotation         (0.0f);
            pLightFacet->SetWidth            (8.0f);
            pLightFacet->SetHeight           (8.0f);
            pLightFacet->SetDirection        (Base::Float3(-0.01f, 0.01f, -1.0f));
            pLightFacet->SetIntensity        (1200.0f);
            pLightFacet->SetTemperature      (0);
            pLightFacet->SetIsTwoSided       (false);

            pLightFacet->UpdateLightness();

            rCurrentEntity.SetDetailFacet(Dt::SFacetCategory::Data, pLightFacet);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnRequestInfoPointlight(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CPointLightFacet* pPointLightFacet = static_cast<Dt::CPointLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::Point && pPointLightFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(rCurrentEntity.GetID());
            NewMessage.PutInt(static_cast<int>(pPointLightFacet->HasTemperature()));
            NewMessage.PutFloat(pPointLightFacet->GetColor()[0]);
            NewMessage.PutFloat(pPointLightFacet->GetColor()[1]);
            NewMessage.PutFloat(pPointLightFacet->GetColor()[2]);
            NewMessage.PutFloat(pPointLightFacet->GetTemperature());
            NewMessage.PutFloat(pPointLightFacet->GetIntensity());
            NewMessage.PutFloat(pPointLightFacet->GetAttenuationRadius());
            NewMessage.PutFloat(Base::RadiansToDegree(pPointLightFacet->GetInnerConeAngle()));
            NewMessage.PutFloat(Base::RadiansToDegree(pPointLightFacet->GetOuterConeAngle()));
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

        Dt::CSunLightFacet* pLightFacet = static_cast<Dt::CSunLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::Sun && pLightFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(rCurrentEntity.GetID());
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
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CSkyFacet* pLightFacet = static_cast<Dt::CSkyFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::Sky && pLightFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(rCurrentEntity.GetID());
            NewMessage.PutInt(static_cast<int>(pLightFacet->GetRefreshMode()));
            NewMessage.PutInt(static_cast<int>(pLightFacet->GetType()));

            if (pLightFacet->GetType() == Dt::CSkyFacet::Cubemap)
            {
                NewMessage.PutBool(true);

                if (pLightFacet->GetCubemap()->GetFileName().length() > 0)
                {
                    NewMessage.PutBool(true);

                    NewMessage.PutString(pLightFacet->GetCubemap()->GetFileName().c_str());
                }
                else
                {
                    NewMessage.PutBool(false);
                }

                NewMessage.PutInt(pLightFacet->GetCubemap()->GetHash());
            }
            else if (pLightFacet->GetType() == Dt::CSkyFacet::Panorama)
            {
                NewMessage.PutBool(true);

                if (pLightFacet->GetPanorama()->GetFileName().length() > 0)
                {
                    NewMessage.PutBool(true);

                    NewMessage.PutString(pLightFacet->GetPanorama()->GetFileName().c_str());
                }
                else
                {
                    NewMessage.PutBool(false);
                }

                NewMessage.PutInt(pLightFacet->GetPanorama()->GetHash());
            }
            else if (pLightFacet->GetType() == Dt::CSkyFacet::Texture || pLightFacet->GetType() == Dt::CSkyFacet::TextureGeometry || pLightFacet->GetType() == Dt::CSkyFacet::TextureLUT)
            {
                NewMessage.PutBool(false);

                NewMessage.PutInt(pLightFacet->GetTexture()->GetHash());
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

        Dt::CLightProbeFacet* pLightFacet = static_cast<Dt::CLightProbeFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::LightProbe && pLightFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(rCurrentEntity.GetID());
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

        Dt::CAreaLightFacet* pLightFacet = static_cast<Dt::CAreaLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::Area && pLightFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(rCurrentEntity.GetID());
            NewMessage.PutInt(static_cast<int>(pLightFacet->HasTemperature()));
            NewMessage.PutFloat(pLightFacet->GetColor()[0]);
            NewMessage.PutFloat(pLightFacet->GetColor()[1]);
            NewMessage.PutFloat(pLightFacet->GetColor()[2]);
            NewMessage.PutFloat(pLightFacet->GetTemperature());
            NewMessage.PutFloat(pLightFacet->GetIntensity());
            NewMessage.PutFloat(Base::RadiansToDegree(pLightFacet->GetRotation()));
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
        Dt::CPointLightFacet* pPointLightFacet = static_cast<Dt::CPointLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::Point && pPointLightFacet != nullptr)
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

            Base::Float3 Color = Base::Float3(R, G, B);

            float Temperature       = _rMessage.GetFloat();
            float Intensity         = _rMessage.GetFloat();
            float AttenuationRadius = _rMessage.GetFloat();
            float InnerConeAngle    = Base::DegreesToRadians(_rMessage.GetFloat());
            float OuterConeAngle    = Base::DegreesToRadians(_rMessage.GetFloat());

            X = _rMessage.GetFloat();
            Y = _rMessage.GetFloat();
            Z = _rMessage.GetFloat();

            Base::Float3 Direction = Base::Float3(X, Y, Z);

            int ShadowType    = _rMessage.GetInt();
            int ShadowQuality = _rMessage.GetInt();
            int ShadowRefresh = _rMessage.GetInt();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pPointLightFacet->EnableTemperature   (ColorMode == 1);
            pPointLightFacet->SetColor            (Color);
            pPointLightFacet->SetTemperature      (Temperature);
            pPointLightFacet->SetIntensity        (Intensity);
            pPointLightFacet->SetAttenuationRadius(AttenuationRadius);
            pPointLightFacet->SetInnerConeAngle   (InnerConeAngle);
            pPointLightFacet->SetOuterConeAngle   (OuterConeAngle);
            pPointLightFacet->SetDirection        (Direction);
            pPointLightFacet->SetShadowType       (static_cast<Dt::CPointLightFacet::EShadowType>(ShadowType));
            pPointLightFacet->SetShadowQuality    (static_cast<Dt::CPointLightFacet::EShadowQuality>(ShadowQuality));
            pPointLightFacet->SetRefreshMode      (static_cast<Dt::CPointLightFacet::ERefreshMode>(ShadowRefresh));
            
            pPointLightFacet->UpdateLightness();

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnInfoSun(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CSunLightFacet* pLightFacet = static_cast<Dt::CSunLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::Sun && pLightFacet != nullptr)
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

            Base::Float3 Color = Base::Float3(R, G, B);

            float Temperature = _rMessage.GetFloat();
            float Intensity = _rMessage.GetFloat();

            X = _rMessage.GetFloat();
            Y = _rMessage.GetFloat();
            Z = _rMessage.GetFloat();

            Base::Float3 Direction = Base::Float3(X, Y, Z);

            int ShadowRefresh = _rMessage.GetInt();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pLightFacet->EnableTemperature(ColorMode == 1);
            pLightFacet->SetColor(Color);
            pLightFacet->SetTemperature(Temperature);
            pLightFacet->SetIntensity(Intensity);
            pLightFacet->SetDirection(Direction);
            pLightFacet->SetRefreshMode(static_cast<Dt::CSunLightFacet::ERefreshMode>(ShadowRefresh));

            pLightFacet->UpdateLightness();

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnInfoEnvironment(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));
        Dt::CSkyFacet* pLightFacet = static_cast<Dt::CSkyFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::Sky && pLightFacet != nullptr)
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
            pLightFacet->SetRefreshMode(static_cast<Dt::CSkyFacet::ERefreshMode>(RefreshMode));
            pLightFacet->SetType       (static_cast<Dt::CSkyFacet::EType>(Type));
            pLightFacet->SetIntensity  (Intensity);

            if (pLightFacet->GetType() == Dt::CSkyFacet::Cubemap)
            {
                Dt::CTextureCube* pTextureCube = Dt::TextureManager::GetTextureCubeByHash(TextureHash);

                if (pTextureCube != nullptr)
                {
                    pLightFacet->SetCubemap(pTextureCube);
                }
            }
            else if (pLightFacet->GetType() == Dt::CSkyFacet::Panorama)
            {
                Dt::CTexture2D* pTexturePanorama = Dt::TextureManager::GetTexture2DByHash(TextureHash);

                if (pTexturePanorama != nullptr)
                {
                    pLightFacet->SetPanorama(pTexturePanorama);
                }
            }
            else if (pLightFacet->GetType() == Dt::CSkyFacet::Texture || pLightFacet->GetType() == Dt::CSkyFacet::TextureGeometry || pLightFacet->GetType() == Dt::CSkyFacet::TextureLUT)
            {
                Dt::CTexture2D* pTexture = Dt::TextureManager::GetTexture2DByHash(TextureHash);

                if (pTexture != nullptr)
                {
                    pLightFacet->SetTexture(pTexture);
                }
            }

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnInfoLightProbe(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CLightProbeFacet* pLightFacet = static_cast<Dt::CLightProbeFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::LightProbe && pLightFacet != nullptr)
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
            pLightFacet->SetRefreshMode(static_cast<Dt::CLightProbeFacet::ERefreshMode>(RefreshMode));

            pLightFacet->SetType(static_cast<Dt::CLightProbeFacet::EType>(Type));

            pLightFacet->SetQuality(static_cast<Dt::CLightProbeFacet::EQuality>(Quality));

            pLightFacet->SetClearFlag(static_cast<Dt::CLightProbeFacet::EClearFlag>(ClearFlag));

            pLightFacet->SetIntensity(Intensity);

            pLightFacet->SetNear(Near);

            pLightFacet->SetFar(Far);

            pLightFacet->SetParallaxCorrection(ParallaxCorrection);

            pLightFacet->SetBoxSize(Base::Float3(BoxSizeX, BoxSizeY, BoxSizeZ));

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnInfoArealight(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));
        Dt::CAreaLightFacet* pLightFacet = static_cast<Dt::CAreaLightFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::Area && pLightFacet != nullptr)
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

            Base::Float3 Color = Base::Float3(R, G, B);

            float Temperature = _rMessage.GetFloat();
            float Intensity   = _rMessage.GetFloat();
            float Rotation    = Base::DegreesToRadians(_rMessage.GetFloat());
            float Width       = _rMessage.GetFloat();
            float Height      = _rMessage.GetFloat();
            bool  IsTwoSided  = _rMessage.GetBool();

            X = _rMessage.GetFloat();
            Y = _rMessage.GetFloat();
            Z = _rMessage.GetFloat();

            Base::Float3 Direction = Base::Float3(X, Y, Z);

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

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
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