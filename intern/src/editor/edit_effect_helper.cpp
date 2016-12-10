
#include "editor/edit_precompiled.h"

#include "base/base_console.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_bloom_manager.h"
#include "data/data_dof_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_fx_type.h"
#include "data/data_post_aa_manager.h"
#include "data/data_map.h"
#include "data/data_ssao_manager.h"
#include "data/data_ssr_manager.h"
#include "data/data_transformation_facet.h"
#include "data/data_volume_fog_manager.h"

#include "editor/edit_effect_helper.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

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

        void OnNewBloom(Edit::CMessage& _rMessage);
        void OnNewDOF(Edit::CMessage& _rMessage);
        void OnNewPostAA(Edit::CMessage& _rMessage);
        void OnNewSSR(Edit::CMessage& _rMessage);
        void OnNewVolumeFog(Edit::CMessage& _rMessage);

        void OnRequestInfoBloom(Edit::CMessage& _rMessage);
        void OnRequestInfoDOF(Edit::CMessage& _rMessage);
        void OnRequestInfoPostAA(Edit::CMessage& _rMessage);
        void OnRequestInfoSSR(Edit::CMessage& _rMessage);
        void OnRequestInfoVolumeFog(Edit::CMessage& _rMessage);

        void OnInfoBloom(Edit::CMessage& _rMessage);
        void OnInfoDOF(Edit::CMessage& _rMessage);
        void OnInfoPostAA(Edit::CMessage& _rMessage);
        void OnInfoSSR(Edit::CMessage& _rMessage);
        void OnInfoVolumeFog(Edit::CMessage& _rMessage);

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
        Edit::MessageManager::Register(Edit::SGUIMessageType::Effect_Bloom_New    , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnNewBloom));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Effect_DOF_New      , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnNewDOF));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Effect_PostAA_New   , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnNewPostAA));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Effect_SSR_New      , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnNewSSR));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Effect_VolumeFog_New, EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnNewVolumeFog));

        Edit::MessageManager::Register(Edit::SGUIMessageType::Effect_Bloom_Info    , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnRequestInfoBloom));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Effect_DOF_Info      , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnRequestInfoDOF));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Effect_PostAA_Info   , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnRequestInfoPostAA));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Effect_SSR_Info      , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnRequestInfoSSR));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Effect_VolumeFog_Info, EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnRequestInfoVolumeFog));
        
        Edit::MessageManager::Register(Edit::SGUIMessageType::Effect_Bloom_Update    , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnInfoBloom));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Effect_DOF_Update      , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnInfoDOF));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Effect_PostAA_Update   , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnInfoPostAA));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Effect_SSR_Update      , EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnInfoSSR));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Effect_VolumeFog_Update, EDIT_RECEIVE_MESSAGE(&CEffectHelper::OnInfoVolumeFog));
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnNewBloom(Edit::CMessage& _rMessage)
    {
        {
            // -----------------------------------------------------------------------------
            // Get entity and set type + category
            // -----------------------------------------------------------------------------
            int EntityID = _rMessage.GetInt();

            Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

            rCurrentEntity.SetCategory(Dt::SEntityCategory::FX);
            rCurrentEntity.SetType(Dt::SFXType::Bloom);

            // -----------------------------------------------------------------------------
            // Create facet and set it
            // -----------------------------------------------------------------------------
            Dt::CBloomFXFacet* pEffectFacet = Dt::BloomManager::CreateBloomFX();

            rCurrentEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnNewDOF(Edit::CMessage& _rMessage)
    {
        {
            // -----------------------------------------------------------------------------
            // Get entity and set type + category
            // -----------------------------------------------------------------------------
            int EntityID = _rMessage.GetInt();

            Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

            rCurrentEntity.SetCategory(Dt::SEntityCategory::FX);
            rCurrentEntity.SetType(Dt::SFXType::DOF);

            // -----------------------------------------------------------------------------
            // Create facet and set it
            // -----------------------------------------------------------------------------
            Dt::CDOFFXFacet* pEffectFacet = Dt::DOFManager::CreateDOFFX();

            rCurrentEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnNewPostAA(Edit::CMessage& _rMessage)
    {
        {
            // -----------------------------------------------------------------------------
            // Get entity and set type + category
            // -----------------------------------------------------------------------------
            int EntityID = _rMessage.GetInt();

            Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

            rCurrentEntity.SetCategory(Dt::SEntityCategory::FX);
            rCurrentEntity.SetType(Dt::SFXType::PostAA);

            // -----------------------------------------------------------------------------
            // Create facet and set it
            // -----------------------------------------------------------------------------
            Dt::CPostAAFXFacet* pEffectFacet = Dt::PostAAManager::CreatePostAAFX();

            rCurrentEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnNewSSR(Edit::CMessage& _rMessage)
    {
        {
            // -----------------------------------------------------------------------------
            // Get entity and set type + category
            // -----------------------------------------------------------------------------
            int EntityID = _rMessage.GetInt();

            Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

            rCurrentEntity.SetCategory(Dt::SEntityCategory::FX);
            rCurrentEntity.SetType(Dt::SFXType::SSR);

            // -----------------------------------------------------------------------------
            // Create facet and set it
            // -----------------------------------------------------------------------------
            Dt::CSSRFXFacet* pEffectFacet = Dt::SSRFXManager::CreateSSRFX();

            rCurrentEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnNewVolumeFog(Edit::CMessage& _rMessage)
    {
        {
            // -----------------------------------------------------------------------------
            // Get entity and set type + category
            // -----------------------------------------------------------------------------
            int EntityID = _rMessage.GetInt();

            Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

            rCurrentEntity.SetCategory(Dt::SEntityCategory::FX);
            rCurrentEntity.SetType(Dt::SFXType::VolumeFog);

            // -----------------------------------------------------------------------------
            // Create facet and set it
            // -----------------------------------------------------------------------------
            Dt::CVolumeFogFXFacet* pEffectFacet = Dt::VolumeFogManager::CreateVolumeFogFX();

            rCurrentEntity.SetDetailFacet(Dt::SFacetCategory::Data, pEffectFacet);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnRequestInfoBloom(Edit::CMessage& _rMessage)
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

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Effect_Bloom_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnRequestInfoDOF(Edit::CMessage& _rMessage)
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

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Effect_DOF_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnRequestInfoPostAA(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CPostAAFXFacet* pFXFacet = static_cast<Dt::CPostAAFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::PostAA && pFXFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.PutInt(rCurrentEntity.GetID());

            NewMessage.PutInt(pFXFacet->GetType());

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Effect_PostAA_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnRequestInfoSSR(Edit::CMessage& _rMessage)
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

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Effect_SSR_Info, NewMessage);
        }
            }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnRequestInfoVolumeFog(Edit::CMessage& _rMessage)
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

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Effect_VolumeFog_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnInfoBloom(Edit::CMessage& _rMessage)
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

    void CEffectHelper::OnInfoDOF(Edit::CMessage& _rMessage)
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

    void CEffectHelper::OnInfoPostAA(Edit::CMessage& _rMessage)
    {
        int EntityID = _rMessage.GetInt();

        Dt::CEntity& rCurrentEntity = Dt::EntityManager::GetEntityByID(static_cast<unsigned int>(EntityID));

        Dt::CPostAAFXFacet* pFXFacet = static_cast<Dt::CPostAAFXFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

        if (rCurrentEntity.GetCategory() == Dt::SEntityCategory::FX && rCurrentEntity.GetType() == Dt::SFXType::PostAA && pFXFacet != nullptr)
        {
            Dt::CPostAAFXFacet::EType Type;

            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------          
            Type = static_cast<Dt::CPostAAFXFacet::EType>(_rMessage.GetInt());

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pFXFacet->SetType(Type);

            pFXFacet->UpdateEffect();

            Dt::EntityManager::MarkEntityAsDirty(rCurrentEntity, Dt::CEntity::DirtyDetail);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnInfoSSR(Edit::CMessage& _rMessage)
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

    void CEffectHelper::OnInfoVolumeFog(Edit::CMessage& _rMessage)
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