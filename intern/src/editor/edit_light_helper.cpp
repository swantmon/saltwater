
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

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

#include <windows.h>
#undef SendMessage

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

        void OnNewEntityPointlight(Edit::CMessage& _rMessage);
        void OnNewEntitySun(Edit::CMessage& _rMessage);
        void OnNewEntityEnvironment(Edit::CMessage& _rMessage);
        void OnNewEntityGlobalProbe(Edit::CMessage& _rMessage);

        void OnRequestEntityInfoPointlight(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoSun(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoEnvironment(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoGlobalProbe(Edit::CMessage& _rMessage);

        void OnEntityInfoPointlight(Edit::CMessage& _rMessage);
        void OnEntityInfoSun(Edit::CMessage& _rMessage);
        void OnEntityInfoEnvironment(Edit::CMessage& _rMessage);
        void OnEntityInfoGlobalProbe(Edit::CMessage& _rMessage);

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
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewLightPointlight            , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnNewEntityPointlight));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewLightSun                   , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnNewEntitySun));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewLightEnvironment           , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnNewEntityEnvironment));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewLightGlobalProbe           , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnNewEntityGlobalProbe));
        
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestLightInfoPointlight    , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnRequestEntityInfoPointlight));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestLightInfoSun           , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnRequestEntityInfoSun));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestLightInfoEnvironment   , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnRequestEntityInfoEnvironment));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestLightInfoGlobalProbe   , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnRequestEntityInfoGlobalProbe));
       
        Edit::MessageManager::Register(Edit::SGUIMessageType::LightInfoPointlight           , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnEntityInfoPointlight));
        Edit::MessageManager::Register(Edit::SGUIMessageType::LightInfoSun                  , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnEntityInfoSun));
        Edit::MessageManager::Register(Edit::SGUIMessageType::LightInfoEnvironment          , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnEntityInfoEnvironment));
        Edit::MessageManager::Register(Edit::SGUIMessageType::LightInfoGlobalProbe          , EDIT_RECEIVE_MESSAGE(&CLightHelper::OnEntityInfoGlobalProbe));
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnNewEntityPointlight(Edit::CMessage& _rMessage)
    {
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Point;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rPointLight = Dt::EntityManager::CreateEntity(EntityDesc);

            // -----------------------------------------------------------------------------
            // Transformation
            // -----------------------------------------------------------------------------
            Dt::CTransformationFacet* pTransformationFacet = rPointLight.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f));
            pTransformationFacet->SetScale   (Base::Float3(1.0f));
            pTransformationFacet->SetRotation(Base::Float3(0.0f));

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

            rPointLight.SetDetailFacet(Dt::SFacetCategory::Data, pPointLightFacet);

            Dt::EntityManager::MarkEntityAsDirty(rPointLight, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnNewEntitySun(Edit::CMessage& _rMessage)
    {
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Sun;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rSunLight = Dt::EntityManager::CreateEntity(EntityDesc);

            // -----------------------------------------------------------------------------
            // Transformation
            // -----------------------------------------------------------------------------
            Dt::CTransformationFacet* pTransformationFacet = rSunLight.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f));
            pTransformationFacet->SetScale   (Base::Float3(1.0f));
            pTransformationFacet->SetRotation(Base::Float3(0.0f));

            Dt::CSunLightFacet* pSunLightFacet = Dt::SunManager::CreateSunLight();

            pSunLightFacet->EnableTemperature(false);
            pSunLightFacet->SetColor         (Base::Float3(1.0f, 1.0f, 1.0f));
            pSunLightFacet->SetDirection     (Base::Float3(0.0f, 0.0f, -1.0f));
            pSunLightFacet->SetIntensity     (90600.0f);
            pSunLightFacet->SetTemperature   (0);
            pSunLightFacet->SetRefreshMode   (Dt::CSunLightFacet::Dynamic);

            pSunLightFacet->UpdateLightness();

            rSunLight.SetDetailFacet(Dt::SFacetCategory::Data, pSunLightFacet);

            Dt::EntityManager::MarkEntityAsDirty(rSunLight, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnNewEntityEnvironment(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Setup environment
        // -----------------------------------------------------------------------------
        {
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

            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Sky;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rEnvironment = Dt::EntityManager::CreateEntity(EntityDesc);

            // -----------------------------------------------------------------------------
            // Transformation
            // -----------------------------------------------------------------------------
            Dt::CTransformationFacet* pTransformationFacet = rEnvironment.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f));
            pTransformationFacet->SetScale   (Base::Float3(1.0f));
            pTransformationFacet->SetRotation(Base::Float3(0.0f));

            Dt::CSkyFacet* pSkyboxFacet = Dt::SkyManager::CreateSky();

            pSkyboxFacet->SetType     (Dt::CSkyFacet::Panorama);
            pSkyboxFacet->SetPanorama (pPanoramaTexture);
            pSkyboxFacet->SetIntensity(5000.0f);

            rEnvironment.SetDetailFacet(Dt::SFacetCategory::Data, pSkyboxFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEnvironment, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnNewEntityGlobalProbe(Edit::CMessage& _rMessage)
    {
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::LightProbe;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy;

            Dt::CEntity& rGlobalProbeLight = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CLightProbeFacet* pLightProbeFacet = Dt::LightProbeManager::CreateLightProbe();

            pLightProbeFacet->SetRefreshMode(Dt::CLightProbeFacet::Static);
            pLightProbeFacet->SetType       (Dt::CLightProbeFacet::Sky);
            pLightProbeFacet->SetQuality    (Dt::CLightProbeFacet::PX512);
            pLightProbeFacet->SetIntensity  (1.0f);

            rGlobalProbeLight.SetDetailFacet(Dt::SFacetCategory::Data, pLightProbeFacet);

            Dt::EntityManager::MarkEntityAsDirty(rGlobalProbeLight, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnRequestEntityInfoPointlight(Edit::CMessage& _rMessage)
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

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::LightInfoPointlight, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnRequestEntityInfoSun(Edit::CMessage& _rMessage)
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

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::LightInfoSun, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnRequestEntityInfoEnvironment(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CSkyFacet* pLightFacet = static_cast<Dt::CSkyFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::Sky && pLightFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt   (rCurrentEntity.GetID());
            NewMessage.PutInt   (static_cast<int>(pLightFacet->GetType()));

            if (pLightFacet->GetHasCubemap())
            {
                NewMessage.PutString(pLightFacet->GetCubemap()->GetFileName());

                NewMessage.PutInt(pLightFacet->GetCubemap()->GetHash());
            }
            else
            {
                NewMessage.PutString(pLightFacet->GetPanorama()->GetFileName());

                NewMessage.PutInt(pLightFacet->GetPanorama()->GetHash());
            }

            NewMessage.PutFloat (pLightFacet->GetIntensity());
                
            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::LightInfoEnvironment, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------
    
    void CLightHelper::OnRequestEntityInfoGlobalProbe(Edit::CMessage& _rMessage)
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
            NewMessage.PutFloat(pLightFacet->GetIntensity());

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::LightInfoGlobalProbe, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnEntityInfoPointlight(Edit::CMessage& _rMessage)
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

    void CLightHelper::OnEntityInfoSun(Edit::CMessage& _rMessage)
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

    void CLightHelper::OnEntityInfoEnvironment(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));
        Dt::CSkyFacet* pLightFacet = static_cast<Dt::CSkyFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::Light && rCurrentEntity.GetType() == Dt::SLightType::Sky && pLightFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            int Type = _rMessage.GetInt();

            char pTemp[256];

            const char* pTextureName = _rMessage.GetString(pTemp, 256);

            unsigned int TextureHash = _rMessage.GetInt();

            float Intensity = _rMessage.GetFloat();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pLightFacet->SetType     (static_cast<Dt::CSkyFacet::EType>(Type));
            pLightFacet->SetIntensity(Intensity);

            if (pLightFacet->GetType() == Dt::CSkyFacet::Cubemap)
            {
                Dt::CTextureCube* pTextureCube = Dt::TextureManager::GetTextureCubeByHash(TextureHash);

                if (pTextureCube == nullptr)
                {
                    // TODO by tschwandt
                    // Do this inside a texture manager
                    Dt::STextureDescriptor TextureDescriptor;

                    TextureDescriptor.m_NumberOfPixelsU = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
                    TextureDescriptor.m_NumberOfPixelsV = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
                    TextureDescriptor.m_NumberOfPixelsW = 1;
                    TextureDescriptor.m_Format          = Dt::CTextureBase::R16G16B16_FLOAT;
                    TextureDescriptor.m_Semantic        = Dt::CTextureBase::HDR;
                    TextureDescriptor.m_Binding         = Dt::CTextureBase::ShaderResource;
                    TextureDescriptor.m_pPixels         = 0;
                    TextureDescriptor.m_pFileName       = pTextureName;
                    TextureDescriptor.m_pIdentifier     = 0;

                    pTextureCube = Dt::TextureManager::CreateCubeTexture(TextureDescriptor);

                    Dt::TextureManager::MarkTextureAsDirty(pTextureCube, Dt::CTextureBase::DirtyFile);
                }

                pLightFacet->SetCubemap(pTextureCube);
            }
            else if (pLightFacet->GetType() == Dt::CSkyFacet::Panorama)
            {
                Dt::CTexture2D* pTexturePanorama = Dt::TextureManager::GetTexture2DByHash(TextureHash);

                if (pTexturePanorama == nullptr)
                {
                    Dt::STextureDescriptor TextureDescriptor;

                    TextureDescriptor.m_NumberOfPixelsU = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
                    TextureDescriptor.m_NumberOfPixelsV = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
                    TextureDescriptor.m_NumberOfPixelsW = 1;
                    TextureDescriptor.m_Format          = Dt::CTextureBase::R16G16B16_FLOAT;
                    TextureDescriptor.m_Semantic        = Dt::CTextureBase::HDR;
                    TextureDescriptor.m_Binding         = Dt::CTextureBase::ShaderResource;
                    TextureDescriptor.m_pPixels         = 0;
                    TextureDescriptor.m_pFileName       = pTextureName;
                    TextureDescriptor.m_pIdentifier     = 0;

                    pTexturePanorama = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

                    Dt::TextureManager::MarkTextureAsDirty(pTexturePanorama, Dt::CTextureBase::DirtyFile);
                }

                pLightFacet->SetPanorama(pTexturePanorama);
            }
            else if (pLightFacet->GetType() == Dt::CSkyFacet::Texture)
            {
                Dt::CTexture2D* pTexture = Dt::TextureManager::GetTexture2DByHash(TextureHash);

                if (pTexture == nullptr && pTextureName != nullptr)
                {
                    Dt::STextureDescriptor TextureDescriptor;

                    TextureDescriptor.m_NumberOfPixelsU = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
                    TextureDescriptor.m_NumberOfPixelsV = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
                    TextureDescriptor.m_NumberOfPixelsW = 1;
                    TextureDescriptor.m_Format          = Dt::CTextureBase::R16G16B16_FLOAT;
                    TextureDescriptor.m_Semantic        = Dt::CTextureBase::HDR;
                    TextureDescriptor.m_Binding         = Dt::CTextureBase::ShaderResource;
                    TextureDescriptor.m_pPixels         = 0;
                    TextureDescriptor.m_pFileName       = pTextureName;
                    TextureDescriptor.m_pIdentifier     = 0;

                    pTexture = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

                    Dt::TextureManager::MarkTextureAsDirty(pTexture, Dt::CTextureBase::DirtyFile);
                }
                
                pLightFacet->SetTexture(pTexture);
            }

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightHelper::OnEntityInfoGlobalProbe(Edit::CMessage& _rMessage)
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

            float Intensity = _rMessage.GetFloat();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pLightFacet->SetRefreshMode(static_cast<Dt::CLightProbeFacet::ERefreshMode>(RefreshMode));

            pLightFacet->SetType(static_cast<Dt::CLightProbeFacet::EType>(Type));

            pLightFacet->SetQuality(static_cast<Dt::CLightProbeFacet::EQuality>(Quality));

            pLightFacet->SetIntensity(Intensity);

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