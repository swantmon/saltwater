
#include "editor/edit_precompiled.h"

#include "base/base_console.h"

#include "camera/cam_control_manager.h"

#include "data/data_actor_facet.h"
#include "data/data_actor_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_light_facet.h"
#include "data/data_light_manager.h"
#include "data/data_fx_facet.h"
#include "data/data_fx_manager.h"
#include "data/data_hierarchy_facet.h"
#include "data/data_map.h"
#include "data/data_material_manager.h"
#include "data/data_model_manager.h"
#include "data/data_plugin_facet.h"
#include "data/data_plugin_manager.h"
#include "data/data_script_facet.h"
#include "data/data_script_manager.h"
#include "data/data_texture_manager.h"
#include "data/data_transformation_facet.h"

#include "editor/edit_edit_state.h"
#include "editor/edit_unload_map_state.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include "graphic/gfx_edit_state.h"

#include "gui/gui_edit_state.h"

#include "logic/lg_edit_state.h"

namespace Edit
{
    CEditState& CEditState::GetInstance()
    {
        static CEditState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Edit

namespace Edit
{
    CEditState::CEditState()
        : m_Action(CState::Edit)
    {
        // -----------------------------------------------------------------------------
        // Register messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SGUIMessageType::Exit  , EDIT_RECEIVE_MESSAGE(&CEditState::OnExit));
        Edit::MessageManager::Register(Edit::SGUIMessageType::Play  , EDIT_RECEIVE_MESSAGE(&CEditState::OnPlay));
        Edit::MessageManager::Register(Edit::SGUIMessageType::NewMap, EDIT_RECEIVE_MESSAGE(&CEditState::OnNewMap));
    }
    
    // -----------------------------------------------------------------------------
    
    CEditState::~CEditState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CEditState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("Edit> Enter edit state.");

        // -----------------------------------------------------------------------------
        // Set editor camera in edit state
        // -----------------------------------------------------------------------------
        Cam::ControlManager::SetActiveControl(Cam::CControl::EditorControl);

        // Cam::ControlManager::GetActiveControl().SetPosition(Base::Float3(0.0f, 0.0f, 10.0f));

        // -----------------------------------------------------------------------------
        // Running states
        // -----------------------------------------------------------------------------
        Lg ::Edit::OnEnter();
        Gui::Edit::OnEnter();
        Gfx::Edit::OnEnter();        
        
        return Edit::CState::Edit;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CEditState::InternOnLeave()
    {
        // -----------------------------------------------------------------------------
        // Reset action
        // -----------------------------------------------------------------------------
        m_Action = CState::Edit;

        // -----------------------------------------------------------------------------
        // Running states
        // -----------------------------------------------------------------------------
        Gfx::Edit::OnLeave();
        Gui::Edit::OnLeave();
        Lg ::Edit::OnLeave();

        BASE_CONSOLE_STREAMINFO("Edit> Leave edit state.");
        
        return Edit::CState::Edit;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CEditState::InternOnRun()
    {
        CState::EStateType NextState = CState::Edit;

        switch (m_Action)
        {
        case Edit::CState::Exit:
            CUnloadMapState::GetInstance().SetNextState(CState::Exit);
            NextState = Edit::CState::UnloadMap;
            break;
        case Edit::CState::Play:
            NextState = Edit::CState::Play;
            break;
        }

        // -----------------------------------------------------------------------------
        // Update logic
        // -----------------------------------------------------------------------------
        Lg::Edit::OnRun();

        // -----------------------------------------------------------------------------
        // Update cameras and views depending on logic and world
        // -----------------------------------------------------------------------------
        Cam::ControlManager::Update();

        // -----------------------------------------------------------------------------
        // Update graphic and GUI
        // -----------------------------------------------------------------------------
        Gui::Edit::OnRun();
        Gfx::Edit::OnRun();

        return NextState;
    }

    // -----------------------------------------------------------------------------

    void CEditState::OnExit(Edit::CMessage& _rMessage)
    {
        m_Action = CState::Exit;
    }

    // -----------------------------------------------------------------------------

    void CEditState::OnPlay(Edit::CMessage& _rMessage)
    {
        m_Action = CState::Play;
    }

    // -----------------------------------------------------------------------------

    void CEditState::OnNewMap(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Allocate a map
        // -----------------------------------------------------------------------------
        int MapX = _rMessage.GetInt();
        int MapY = _rMessage.GetInt();

        Dt::Map::AllocateMap(MapX, MapY);

        // -----------------------------------------------------------------------------
        // Setup cameras
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Actor;
            EntityDesc.m_EntityType     = Dt::SActorType::Camera;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rEntity = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CTransformationFacet* pTransformationFacet = rEntity.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f, 0.0f, 10.0f));
            pTransformationFacet->SetScale(Base::Float3(1.0f));
            pTransformationFacet->SetRotation(Base::Float3(0.0f, 0.0f, 0.0f));

            Dt::CCameraActorFacet* pFacet = Dt::ActorManager::CreateCameraActor();

            pFacet->SetMainCamera(true);

            rEntity.SetDetailFacet(Dt::SFacetCategory::Data, pFacet);

            Dt::CScriptFacet* pScriptFacet = Dt::ScriptManager::CreateScript();

            pScriptFacet->SetScriptFile("scripts/camera_behavior.lua");

            rEntity.SetDetailFacet(Dt::SFacetCategory::Script, pScriptFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

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
            TextureDescriptor.m_pPixels         = 0;
            TextureDescriptor.m_pFileName       = "environments/Ridgecrest_Road_Ref.hdr";
            TextureDescriptor.m_pIdentifier     = 0;

            Dt::CTexture2D* pPanoramaTexture = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

            // -----------------------------------------------------------------------------

            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Light;
            EntityDesc.m_EntityType     = Dt::SLightType::Skybox;
            EntityDesc.m_FacetFlags     = 0;

            Dt::CEntity& rEnvironment = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CSkyboxFacet* pSkyboxFacet = Dt::LightManager::CreateSkybox();

            pSkyboxFacet->SetType(Dt::CSkyboxFacet::Panorama);
            pSkyboxFacet->SetTexture(pPanoramaTexture);
            pSkyboxFacet->SetIntensity(10000.0f);

            rEnvironment.SetDetailFacet(Dt::SFacetCategory::Data, pSkyboxFacet);

            Dt::EntityManager::MarkEntityAsDirty(rEnvironment, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }

        // -----------------------------------------------------------------------------
        // Setup entities
        // -----------------------------------------------------------------------------
        {
            Dt::SEntityDescriptor EntityDesc;

            EntityDesc.m_EntityCategory = Dt::SEntityCategory::Actor;
            EntityDesc.m_EntityType     = Dt::SActorType::Model;
            EntityDesc.m_FacetFlags     = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation;

            Dt::CEntity& rSphere = Dt::EntityManager::CreateEntity(EntityDesc);

            Dt::CTransformationFacet* pTransformationFacet = rSphere.GetTransformationFacet();

            pTransformationFacet->SetPosition(Base::Float3(0.0f));
            pTransformationFacet->SetScale   (Base::Float3(1.0f));
            pTransformationFacet->SetRotation(Base::Float3(0.0f));

            Dt::CModelActorFacet* pModelActorFacet = Dt::ActorManager::CreateModelActor();

            Dt::SModelFileDescriptor ModelFileDesc;

            ModelFileDesc.m_pFileName = "models/sphere.obj";
            ModelFileDesc.m_GenFlag   = Dt::SGeneratorFlag::DefaultFlipUVs;

            Dt::SMaterialFileDescriptor MaterialFileDesc;

            MaterialFileDesc.m_pFileName = "materials/naturals/metals/Gold_Worn_00.mat";

            pModelActorFacet->SetModel(&Dt::ModelManager::CreateModel(ModelFileDesc));
            pModelActorFacet->SetMaterial(0, &Dt::MaterialManager::CreateMaterial(MaterialFileDesc));

            rSphere.SetDetailFacet(Dt::SFacetCategory::Data, pModelActorFacet);

            Dt::EntityManager::MarkEntityAsDirty(rSphere, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
        }
    }
} // namespace Edit