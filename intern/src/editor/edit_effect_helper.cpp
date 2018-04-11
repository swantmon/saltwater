
#include "editor/edit_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_console.h"

#include "engine/data/data_bloom_component.h"
#include "engine/data/data_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_dof_component.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_map.h"
#include "engine/data/data_post_aa_component.h"
#include "engine/data/data_ssao_component.h"
#include "engine/data/data_ssr_component.h"
#include "engine/data/data_transformation_facet.h"
#include "engine/data/data_volume_fog_component.h"

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
            Base::ID EntityID = _rMessage.Get<Base::ID>();

            Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CBloomComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CBloomComponent::DirtyCreate);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnNewDOF(Edit::CMessage& _rMessage)
    {
        {
            // -----------------------------------------------------------------------------
            // Get entity and set type + category
            // -----------------------------------------------------------------------------
            Base::ID EntityID = _rMessage.Get<Base::ID>();

            Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CDOFComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CDOFComponent::DirtyCreate);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnNewPostAA(Edit::CMessage& _rMessage)
    {
        {
            // -----------------------------------------------------------------------------
            // Get entity and set type + category
            // -----------------------------------------------------------------------------
            Base::ID EntityID = _rMessage.Get<Base::ID>();

            Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CPostAAComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CPostAAComponent::DirtyCreate);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnNewSSR(Edit::CMessage& _rMessage)
    {
        {
            // -----------------------------------------------------------------------------
            // Get entity and set type + category
            // -----------------------------------------------------------------------------
            Base::ID EntityID = _rMessage.Get<Base::ID>();

            Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CSSRComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CSSRComponent::DirtyCreate);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnNewVolumeFog(Edit::CMessage& _rMessage)
    {
        {
            // -----------------------------------------------------------------------------
            // Get entity and set type + category
            // -----------------------------------------------------------------------------
            Base::ID EntityID = _rMessage.Get<Base::ID>();

            Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CVolumeFogComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CVolumeFogComponent::DirtyCreate);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnRequestInfoBloom(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CBloomComponent* pFXFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CBloomComponent>();

        if (pFXFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.Put(pCurrentEntity->GetID());

            NewMessage.Put(pFXFacet->GetTint()[0]);
            NewMessage.Put(pFXFacet->GetTint()[1]);
            NewMessage.Put(pFXFacet->GetTint()[2]);
            NewMessage.Put(pFXFacet->GetTint()[3]);

            NewMessage.Put(pFXFacet->GetIntensity());
            NewMessage.Put(pFXFacet->GetTreshhold());
            NewMessage.Put(pFXFacet->GetExposureScale());
            NewMessage.Put(pFXFacet->GetSize());

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Effect_Bloom_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnRequestInfoDOF(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CDOFComponent* pFXFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CDOFComponent>();

        if (pFXFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.Put(pCurrentEntity->GetID());

            NewMessage.Put(pFXFacet->GetNearDistance());
            NewMessage.Put(pFXFacet->GetFarDistance());
            NewMessage.Put(pFXFacet->GetNearToFarRatio());
            NewMessage.Put(pFXFacet->GetFadeUnToSmallBlur());
            NewMessage.Put(pFXFacet->GetFadeSmallToMediumBlur());

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Effect_DOF_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnRequestInfoPostAA(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CPostAAComponent* pFXFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CPostAAComponent>();

        if (pFXFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.Put(pCurrentEntity->GetID());

            NewMessage.Put(pFXFacet->GetType());

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Effect_PostAA_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnRequestInfoSSR(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CSSRComponent* pFXFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CSSRComponent>();

        if (pFXFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.Put(pCurrentEntity->GetID());

            NewMessage.Put(pFXFacet->GetIntensity());
            NewMessage.Put(pFXFacet->GetRoughnessMask());
            NewMessage.Put(pFXFacet->GetDistance());
            NewMessage.Put(pFXFacet->GetUseLastFrame());

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Effect_SSR_Info, NewMessage);
        }
            }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnRequestInfoVolumeFog(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CVolumeFogComponent* pFXFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CVolumeFogComponent>();

        if (pFXFacet != nullptr)
        {
            Edit::CMessage NewMessage;

            NewMessage.Put(pCurrentEntity->GetID());

            NewMessage.Put(pFXFacet->GetWindDirection()[0]);
            NewMessage.Put(pFXFacet->GetWindDirection()[1]);
            NewMessage.Put(pFXFacet->GetWindDirection()[2]);
            NewMessage.Put(pFXFacet->GetWindDirection()[3]);

            NewMessage.Put(pFXFacet->GetFogColor()[0]);
            NewMessage.Put(pFXFacet->GetFogColor()[1]);
            NewMessage.Put(pFXFacet->GetFogColor()[2]);
            NewMessage.Put(pFXFacet->GetFogColor()[3]);

            NewMessage.Put(pFXFacet->GetFrustumDepthInMeter());
            NewMessage.Put(pFXFacet->GetShadowIntensity());
            NewMessage.Put(pFXFacet->GetScatteringCoefficient());
            NewMessage.Put(pFXFacet->GetAbsorptionCoefficient());
            NewMessage.Put(pFXFacet->GetDensityLevel());
            NewMessage.Put(pFXFacet->GetDensityAttenuation());

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SApplicationMessageType::Effect_VolumeFog_Info, NewMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnInfoBloom(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CBloomComponent* pFXFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CBloomComponent>();

        if (pFXFacet != nullptr)
        {
            float R, G, B, A;

            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            R = _rMessage.Get<float>();
            G = _rMessage.Get<float>();
            B = _rMessage.Get<float>();
            A = _rMessage.Get<float>();

            glm::vec4 Color = glm::vec4(R, G, B, A);

            float Intensity     = _rMessage.Get<float>();
            float Treshhold     = _rMessage.Get<float>();
            float ExposureScale = _rMessage.Get<float>();
            int   Size          = _rMessage.Get<int>();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pFXFacet->SetTint(Color);

            pFXFacet->SetIntensity(Intensity);

            pFXFacet->SetTreshhold(Treshhold);

            pFXFacet->SetExposureScale(ExposureScale);

            pFXFacet->SetSize(Size);

            pFXFacet->UpdateEffect();

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pFXFacet, Dt::CBloomComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnInfoDOF(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CDOFComponent* pFXFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CDOFComponent>();

        if (pFXFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------               
            float Near            = _rMessage.Get<float>();
            float Far             = _rMessage.Get<float>();
            float NearToFarRatio  = _rMessage.Get<float>();
            float FadeUnSmall     = _rMessage.Get<float>();
            float FadeSmallMedium = _rMessage.Get<float>();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pFXFacet->SetNearDistance(Near);

            pFXFacet->SetFarDistance(Far);

            pFXFacet->SetNearToFarRatio(NearToFarRatio);

            pFXFacet->SetFadeUnToSmallBlur(FadeUnSmall);

            pFXFacet->SetFadeSmallToMediumBlur(FadeSmallMedium);

            pFXFacet->UpdateEffect();

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pFXFacet, Dt::CDOFComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnInfoPostAA(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CPostAAComponent* pFXFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CPostAAComponent>();

        if (pFXFacet != nullptr)
        {
            Dt::CPostAAComponent::EType Type;

            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------          
            Type = static_cast<Dt::CPostAAComponent::EType>(_rMessage.Get<int>());

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pFXFacet->SetType(Type);

            pFXFacet->UpdateEffect();

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pFXFacet, Dt::CPostAAComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnInfoSSR(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CSSRComponent* pFXFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CSSRComponent>();

        if (pFXFacet != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------               
            float Intensity            = _rMessage.Get<float>();
            float RoughnessMask        = _rMessage.Get<float>();
            float Distance             = _rMessage.Get<float>();
            bool  UseDoubleReflections = _rMessage.Get<bool>();

            // -----------------------------------------------------------------------------
            // Set values
            // -----------------------------------------------------------------------------
            pFXFacet->SetIntensity(Intensity);

            pFXFacet->SetRoughnessMask(RoughnessMask);

            pFXFacet->SetDistance(Distance);

            pFXFacet->SetUseLastFrame(UseDoubleReflections);

            pFXFacet->UpdateEffect();

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pFXFacet, Dt::CSSRComponent::DirtyInfo);
        }
    }

    // -----------------------------------------------------------------------------

    void CEffectHelper::OnInfoVolumeFog(Edit::CMessage& _rMessage)
    {
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(EntityID);

        Dt::CVolumeFogComponent* pFXFacet = pCurrentEntity->GetComponentFacet()->GetComponent<Dt::CVolumeFogComponent>();

        if (pFXFacet != nullptr)
        {
            float R, G, B, A;
            float X, Y, Z, W;

            // -----------------------------------------------------------------------------
            // Read values
            // -----------------------------------------------------------------------------
            X = _rMessage.Get<float>();
            Y = _rMessage.Get<float>();
            Z = _rMessage.Get<float>();
            W = _rMessage.Get<float>();

            glm::vec4 WindDirection = glm::vec4(X, Y, Z, W);

            R = _rMessage.Get<float>();
            G = _rMessage.Get<float>();
            B = _rMessage.Get<float>();
            A = _rMessage.Get<float>();

            glm::vec4 Color = glm::vec4(R, G, B, A);

            float FrustumDepth       = _rMessage.Get<float>();
            float ShadowIntensity    = _rMessage.Get<float>();
            float ScatteringCoeff    = _rMessage.Get<float>();
            float AbsorptionCoeff    = _rMessage.Get<float>();
            float DensityLevel       = _rMessage.Get<float>();
            float DensityAttenuation = _rMessage.Get<float>();

            BASE_UNUSED(ShadowIntensity);

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

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pFXFacet, Dt::CVolumeFogComponent::DirtyInfo);
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