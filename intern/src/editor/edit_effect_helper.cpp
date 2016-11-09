
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_fx_facet.h"
#include "data/data_fx_manager.h"
#include "data/data_map.h"
#include "data/data_transformation_facet.h"

#include "editor/edit_effect_helper.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <windows.h>
#undef SendMessage

namespace
{
    class CEffectHelper : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CEffectHelper)

    public:

        CEffectHelper();
        ~CEffectHelper();

    public:

        void OnStart();
        void OnExit();

    private:

        void OnNewEntityBloom(Edit::CMessage& _rMessage);
        void OnNewEntityDOF(Edit::CMessage& _rMessage);
        void OnNewEntityFXAA(Edit::CMessage& _rMessage);
        void OnNewEntitySSR(Edit::CMessage& _rMessage);
        void OnNewEntityVolumeFog(Edit::CMessage& _rMessage);

        void OnRequestEntityInfoBloom(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoDOF(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoFXAA(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoSSR(Edit::CMessage& _rMessage);
        void OnRequestEntityInfoVolumeFog(Edit::CMessage& _rMessage);

        void OnEntityInfoBloom(Edit::CMessage& _rMessage);
        void OnEntityInfoDOF(Edit::CMessage& _rMessage);
        void OnEntityInfoFXAA(Edit::CMessage& _rMessage);
        void OnEntityInfoSSR(Edit::CMessage& _rMessage);
        void OnEntityInfoVolumeFog(Edit::CMessage& _rMessage);

        void OnDirtyEntity(Dt::CEntity* _pEntity);
    };
} // namespace

namespace
{
    CEffectHelper::CEffectHelper()
    {
        
    }

    // -----------------------------------------------------------------------------

    CEffectHelper::~CEffectHelper()
    {

    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Entity
        // -----------------------------------------------------------------------------
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CEffectHelper::OnDirtyEntity));

        // -----------------------------------------------------------------------------
        // Edit
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewEffectBloom                 , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnNewEntityBloom));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewEffectDOF                   , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnNewEntityDOF));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewEffectFXAA                  , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnNewEntityFXAA));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewEffectSSR                   , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnNewEntitySSR));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewEffectVolumeFog             , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnNewEntityVolumeFog));

        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEffectInfoBloom         , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnRequestEntityInfoBloom));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEffectInfoDOF           , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnRequestEntityInfoDOF));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEffectInfoFXAA          , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnRequestEntityInfoFXAA));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEffectInfoSSR           , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnRequestEntityInfoSSR));
        Edit::MessageManager::Register(Edit::SGUIMessageType::RequestEffectInfoVolumeFog     , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnRequestEntityInfoVolumeFog));
        
        Edit::MessageManager::Register(Edit::SGUIMessageType::EffectInfoBloom                , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnEntityInfoBloom));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EffectInfoDOF                  , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnEntityInfoDOF));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EffectInfoFXAA                 , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnEntityInfoFXAA));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EffectInfoSSR                  , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnEntityInfoSSR));
        Edit::MessageManager::Register(Edit::SGUIMessageType::EffectInfoVolumeFog            , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnEntityInfoVolumeFog));
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnNewEntityBloom(Edit::CMessage& _rMessage)
    {
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::FX;
            EntityDesc.m_EntityType     = Dt::SFXType::Bloom;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEffectEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CBloomFXFacet* pEffectFacet = Dt::FXManager::CreateBloomFX();

            rEffectEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEffectEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnNewEntityDOF(Edit::CMessage& _rMessage)
    {
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::FX;
            EntityDesc.m_EntityType     = Dt::SFXType::DOF;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEffectEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CDOFFXFacet* pEffectFacet = Dt::FXManager::CreateDOFFX();

            rEffectEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEffectEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnNewEntityFXAA(Edit::CMessage& _rMessage)
    {
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::FX;
            EntityDesc.m_EntityType     = Dt::SFXType::FXAA;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEffectEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CFXAAFXFacet* pEffectFacet = Dt::FXManager::CreateFXAAFX();

            rEffectEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEffectEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnNewEntitySSR(Edit::CMessage& _rMessage)
    {
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::FX;
            EntityDesc.m_EntityType     = Dt::SFXType::SSR;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEffectEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CSSRFXFacet* pEffectFacet = Dt::FXManager::CreateSSRFX();

            rEffectEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEffectEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnNewEntityVolumeFog(Edit::CMessage& _rMessage)
    {
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::FX;
            EntityDesc.m_EntityType     = Dt::SFXType::VolumeFog;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEffectEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CVolumeFogFXFacet* pEffectFacet = Dt::FXManager::CreateVolumeFogFX();

            rEffectEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEffectEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnRequestEntityInfoBloom(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CBloomFXFacet* pFXFacet = static_cast<Dt::CBloomFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::Bloom && pFXFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(rCurrentEntity.GetID());

            NewMessage.PutFloat(pFXFacet->GetTint()[0]);
            NewMessage.PutFloat(pFXFacet->GetTint()[1]);
            NewMessage.PutFloat(pFXFacet->GetTint()[2]);
            NewMessage.PutFloat(pFXFacet->GetTint()[3]);

            NewMessage.PutFloat(pFXFacet->GetIntensity());
            NewMessage.PutFloat(pFXFacet->GetTreshhold());
            NewMessage.PutFloat(pFXFacet->GetExposureScale());
            NewMessage.PutInt(pFXFacet->GetSize());

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::EffectInfoBloom, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnRequestEntityInfoDOF(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CDOFFXFacet* pFXFacet = static_cast<Dt::CDOFFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::DOF && pFXFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(rCurrentEntity.GetID());

            NewMessage.PutFloat(pFXFacet->GetNearDistance());
            NewMessage.PutFloat(pFXFacet->GetFarDistance());
            NewMessage.PutFloat(pFXFacet->GetNearToFarRatio());
            NewMessage.PutFloat(pFXFacet->GetFadeUnToSmallBlur());
            NewMessage.PutFloat(pFXFacet->GetFadeSmallToMediumBlur());

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::EffectInfoDOF, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnRequestEntityInfoFXAA(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CFXAAFXFacet* pFXFacet = static_cast<Dt::CFXAAFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::FXAA && pFXFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(rCurrentEntity.GetID());

            NewMessage.PutFloat(pFXFacet->GetLuma()[0]);
            NewMessage.PutFloat(pFXFacet->GetLuma()[1]);
            NewMessage.PutFloat(pFXFacet->GetLuma()[2]);

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::EffectInfoFXAA, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnRequestEntityInfoSSR(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CSSRFXFacet* pFXFacet = static_cast<Dt::CSSRFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::SSR && pFXFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(rCurrentEntity.GetID());

            NewMessage.PutFloat(pFXFacet->GetIntensity());
            NewMessage.PutFloat(pFXFacet->GetRoughnessMask());
            NewMessage.PutFloat(pFXFacet->GetDistance());
            NewMessage.PutBool(pFXFacet->GetUseLastFrame());

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::EffectInfoSSR, NewMessage);
        }
            }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnRequestEntityInfoVolumeFog(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CVolumeFogFXFacet* pFXFacet = static_cast<Dt::CVolumeFogFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::VolumeFog && pFXFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(rCurrentEntity.GetID());

            NewMessage.PutFloat(pFXFacet->GetWindDirection()[0]);
            NewMessage.PutFloat(pFXFacet->GetWindDirection()[1]);
            NewMessage.PutFloat(pFXFacet->GetWindDirection()[2]);
            NewMessage.PutFloat(pFXFacet->GetWindDirection()[3]);

            NewMessage.PutFloat(pFXFacet->GetFogColor()[0]);
            NewMessage.PutFloat(pFXFacet->GetFogColor()[1]);
            NewMessage.PutFloat(pFXFacet->GetFogColor()[2]);
            NewMessage.PutFloat(pFXFacet->GetFogColor()[3]);

            NewMessage.PutFloat(pFXFacet->GetFrustumDepthInMeter());
            NewMessage.PutFloat(pFXFacet->GetShadowIntensity());
            NewMessage.PutFloat(pFXFacet->GetScatteringCoefficient());
            NewMessage.PutFloat(pFXFacet->GetAbsorptionCoefficient());
            NewMessage.PutFloat(pFXFacet->GetDensityLevel());
            NewMessage.PutFloat(pFXFacet->GetDensityAttenuation());

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::EffectInfoVolumeFog, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnEntityInfoBloom(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CBloomFXFacet* pFXFacet = static_cast<Dt::CBloomFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::Bloom && pFXFacet != nullptr)
        {
            float R, G, B, A;

            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            R = _rMessage.GetFloat();
            G = _rMessage.GetFloat();
            B = _rMessage.GetFloat();
            A = _rMessage.GetFloat();

            Base::Float4 Color = Base::Float4(R, G, B, A);

            float Intensity     = _rMessage.GetFloat();
            float Treshhold     = _rMessage.GetFloat();
            float ExposureScale = _rMessage.GetFloat();
            int   Size          = _rMessage.GetInt();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pFXFacet->SetTint(Color);

            pFXFacet->SetIntensity(Intensity);

            pFXFacet->SetTreshhold(Treshhold);

            pFXFacet->SetExposureScale(ExposureScale);

            pFXFacet->SetSize(Size);

            pFXFacet->UpdateEffect();

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnEntityInfoDOF(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CDOFFXFacet* pFXFacet = static_cast<Dt::CDOFFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::DOF && pFXFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------               
            float Near            = _rMessage.GetFloat();
            float Far             = _rMessage.GetFloat();
            float NearToFarRatio  = _rMessage.GetFloat();
            float FadeUnSmall     = _rMessage.GetFloat();
            float FadeSmallMedium = _rMessage.GetFloat();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pFXFacet->SetNearDistance(Near);

            pFXFacet->SetFarDistance(Far);

            pFXFacet->SetNearToFarRatio(NearToFarRatio);

            pFXFacet->SetFadeUnToSmallBlur(FadeUnSmall);

            pFXFacet->SetFadeSmallToMediumBlur(FadeSmallMedium);

            pFXFacet->UpdateEffect();

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnEntityInfoFXAA(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CFXAAFXFacet* pFXFacet = static_cast<Dt::CFXAAFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::FXAA && pFXFacet != nullptr)
        {
            float R, G, B;

            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------          
            R = _rMessage.GetFloat();
            G = _rMessage.GetFloat();
            B = _rMessage.GetFloat();

            Base::Float3 Color = Base::Float3(R, G, B);

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pFXFacet->SetLuma(Color);

            pFXFacet->UpdateEffect();

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnEntityInfoSSR(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CSSRFXFacet* pFXFacet = static_cast<Dt::CSSRFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::SSR && pFXFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------               
            float Intensity            = _rMessage.GetFloat();
            float RoughnessMask        = _rMessage.GetFloat();
            float Distance             = _rMessage.GetFloat();
            bool  UseDoubleReflections = _rMessage.GetBool();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pFXFacet->SetIntensity(Intensity);

            pFXFacet->SetRoughnessMask(RoughnessMask);

            pFXFacet->SetDistance(Distance);

            pFXFacet->SetUseLastFrame(UseDoubleReflections);

            pFXFacet->UpdateEffect();

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnEntityInfoVolumeFog(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CVolumeFogFXFacet* pFXFacet = static_cast<Dt::CVolumeFogFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::VolumeFog && pFXFacet != nullptr)
        {
            float R, G, B, A;
            float X, Y, Z, W;

            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            X = _rMessage.GetFloat();
            Y = _rMessage.GetFloat();
            Z = _rMessage.GetFloat();
            W = _rMessage.GetFloat();

            Base::Float4 WindDirection = Base::Float4(X, Y, Z, W);

            R = _rMessage.GetFloat();
            G = _rMessage.GetFloat();
            B = _rMessage.GetFloat();
            A = _rMessage.GetFloat();

            Base::Float4 Color = Base::Float4(R, G, B, A);

            float FrustumDepth       = _rMessage.GetFloat();
            float ShadowIntensity    = _rMessage.GetFloat();
            float ScatteringCoeff    = _rMessage.GetFloat();
            float AbsorptionCoeff    = _rMessage.GetFloat();
            float DensityLevel       = _rMessage.GetFloat();
            float DensityAttenuation = _rMessage.GetFloat();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pFXFacet->SetWindDirection(WindDirection);

            pFXFacet->SetFogColor(Color);

            pFXFacet->SetFrustumDepthInMeter(FrustumDepth);

            pFXFacet->SetScatteringCoefficient(ScatteringCoeff);

            pFXFacet->SetAbsorptionCoefficient(AbsorptionCoeff);

            pFXFacet->SetDensityLevel(DensityLevel);

            pFXFacet->SetDensityAttenuation(DensityAttenuation);

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnDirtyEntity(Dt::CEntity* _pEntity)
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
namespace Effect
{
    void OnStart()
    {
        CEffectHelper::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CEffectHelper::GetInstance().OnExit();
    }
} // namespace Effect
} // namespace Helper
} // namespace Edit