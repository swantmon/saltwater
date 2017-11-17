
#include "logic/lg_precompiled.h"

#include "base/base_console.h"
#include "base/base_input_event.h"
#include "base/base_math_operations.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "camera/cam_control_manager.h"
#include "camera/cam_game_control.h"

#include "core/core_time.h"

#include "data/data_actor_type.h"
#include "data/data_area_light_manager.h"
#include "data/data_camera_actor_manager.h"
#include "data/data_entity.h"
#include "data/data_entity_manager.h"
#include "data/data_light_probe_manager.h"
#include "data/data_light_type.h"
#include "data/data_map.h"
#include "data/data_mesh_manager.h"
#include "data/data_point_light_manager.h"
#include "data/data_script_manager.h"
#include "data/data_sky_manager.h"
#include "data/data_sun_manager.h"

#include "gui/gui_event_handler.h"

#include "logic/lg_play_state.h"
#include "logic/lg_script_manager.h"

namespace
{
    class CLgPlayState : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CLgPlayState)
        
    public:
        
        int OnEnter();
        int OnLeave();
        int OnRun();
    
    private:
        
        int m_State;
        
    private:
        
        void OnInputEvent(const Base::CInputEvent& _rInputEvent);
        
    };
} // namespace

namespace
{
    int CLgPlayState::OnEnter()
    {
        // -----------------------------------------------------------------------------
        // Define variables
        // -----------------------------------------------------------------------------
        m_State = Lg::Play::SResult::Play;
        
        // -----------------------------------------------------------------------------
        // Register input event delegate @ gui event handler
        // -----------------------------------------------------------------------------
        Gui::EventHandler::RegisterDirectUserListener(GUI_BIND_INPUT_METHOD(&CLgPlayState::OnInputEvent));
        
        return Lg::Play::SResult::Play;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgPlayState::OnLeave()
    {        
        // -----------------------------------------------------------------------------
        // Unregister input event delegate @ gui event handler
        // -----------------------------------------------------------------------------
        Gui::EventHandler::UnregisterDirectUserListener(GUI_BIND_INPUT_METHOD(&CLgPlayState::OnInputEvent));
        
        return Lg::Play::SResult::Play;
    }
    
    // -----------------------------------------------------------------------------
    
    int CLgPlayState::OnRun()
    {
        // -----------------------------------------------------------------------------
        // Prepare updates depending on time difference between frames
        // -----------------------------------------------------------------------------
        float TimeOfLastFrame = static_cast<float>(Core::Time::GetDeltaTimeLastFrame());

        BASE_UNUSED(TimeOfLastFrame);

        // -----------------------------------------------------------------------------
        // Update data manager
        // -----------------------------------------------------------------------------
        Dt::MeshActorManager  ::Update();
        Dt::CameraActorManager::Update();
        Dt::EntityManager     ::Update();
        Dt::SunManager        ::Update();
        Dt::LightProbeManager ::Update();
        Dt::PointLightManager ::Update();
        Dt::AreaLightManager  ::Update();
        Dt::SkyManager        ::Update();
        Dt::ScriptManager     ::Update();

        Lg::ScriptManager::Update();

        // -----------------------------------------------------------------------------
        // Get main camera entity and set this entity to the camera project
        // as linked entity.
        // After the first main camera we can break the loop.
        // -----------------------------------------------------------------------------
        Dt::Map::CEntityIterator CurrentEntity = Dt::Map::EntitiesBegin(Dt::SEntityCategory::Actor);
        Dt::Map::CEntityIterator EndOfEntities = Dt::Map::EntitiesEnd();

        for (; CurrentEntity != EndOfEntities; CurrentEntity = CurrentEntity.Next(Dt::SEntityCategory::Actor))
        {
            Dt::CEntity& rCurrentEntity = *CurrentEntity;

            if (rCurrentEntity.GetType() == Dt::SActorType::Camera)
            {
                Dt::CCameraActorFacet* pCameraActorFacet = static_cast<Dt::CCameraActorFacet*>(rCurrentEntity.GetDetailFacet(Dt::SFacetCategory::Data));

                assert(pCameraActorFacet != nullptr);

                if (pCameraActorFacet->IsMainCamera())
                {
                    Cam::CControl& rControl = Cam::ControlManager::GetActiveControl();

                    assert(rControl.GetType() == Cam::CControl::GameControl);

                    Cam::CGameControl& rGameControl = static_cast<Cam::CGameControl&>(rControl);

                    rGameControl.SetEntity(rCurrentEntity);

                    break;
                }
            }
        }
        
        // -----------------------------------------------------------------------------
        // Return state changes
        // -----------------------------------------------------------------------------
        return m_State;
    }
    
    // -----------------------------------------------------------------------------
    
    void CLgPlayState::OnInputEvent(const Base::CInputEvent& _rInputEvent)
    {
        auto AddLightToScene = [&] (const Base::Float3& _rPosition, float _Range, const Base::Float3& _rColor)
        {
            BASE_UNUSED(_rPosition);
            BASE_UNUSED(_Range);
            BASE_UNUSED(_rColor);
        };
        
        if (_rInputEvent.GetAction() == Base::CInputEvent::KeyReleased)
        {
            
            if (_rInputEvent.GetKey() == Base::CInputEvent::Key1)
            {
                Base::Float3 CurrentPosition = Cam::ControlManager::GetActiveControl().GetPosition();
                
                AddLightToScene(CurrentPosition, 8.0f, Base::Float3(1.0f, 0.0f, 0.0f));
            }
            
            if (_rInputEvent.GetKey() == Base::CInputEvent::Key2)
            {
                Base::Float3 CurrentPosition = Cam::ControlManager::GetActiveControl().GetPosition();
                
                AddLightToScene(CurrentPosition, 8.0f, Base::Float3(0.0f, 1.0f, 0.0f));
            }
            
            if (_rInputEvent.GetKey() == Base::CInputEvent::Key3)
            {
                Base::Float3 CurrentPosition = Cam::ControlManager::GetActiveControl().GetPosition();
                
                AddLightToScene(CurrentPosition, 8.0f, Base::Float3(0.0f, 0.0f, 1.0f));
            }
            
            if (_rInputEvent.GetKey() == Base::CInputEvent::KeyRhombos)
            {
                m_State = Lg::Play::SResult::Debug;
            }
        }
    }
} // namespace

namespace Lg
{
namespace Play
{
    int OnEnter()
    {
        return CLgPlayState::GetInstance().OnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnLeave()
    {
        return CLgPlayState::GetInstance().OnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    int OnRun()
    {
        return CLgPlayState::GetInstance().OnRun();
    }
} // namespace Play
} // namespace Lg