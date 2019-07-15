
#include "editor/edit_precompiled.h"

#include "editor/edit_play_state.h"
#include "editor/edit_unload_map_state.h"

#include "engine/camera/cam_control_manager.h"

namespace Edit
{
    CPlayState& CPlayState::GetInstance()
    {
        static CPlayState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Edit

namespace Edit
{
    CPlayState::CPlayState()
        : CState(Play)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CPlayState::~CPlayState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CPlayState::InternOnEnter()
    {
        // -----------------------------------------------------------------------------
        // Activate game control for play state
        // -----------------------------------------------------------------------------
        Cam::ControlManager::SetActiveControl(Cam::CControl::GameControl);
    }
    
    // -----------------------------------------------------------------------------
    
    void CPlayState::InternOnLeave()
    {
        m_NextState = Play;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CPlayState::InternOnRun()
    {
        return m_NextState;
    }
} // namespace Edit