
#include "base/base_console.h"
#include "base/base_serialize_text_writer.h"

#include "editor/edit_unload_map_state.h"

#include "graphic/gfx_unload_map_state.h"

#include "gui/gui_unload_map_state.h"

#include "logic/lg_unload_map_state.h"

#include <assert.h>
#include <fstream>
#include <string>


namespace Edit
{
    CUnloadMapState& CUnloadMapState::GetInstance()
    {
        static CUnloadMapState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Edit

namespace Edit
{
    CUnloadMapState::CUnloadMapState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CUnloadMapState::~CUnloadMapState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CUnloadMapState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("Edit> Enter unload map state.");

        // -----------------------------------------------------------------------------
        // Save current map to file to get same result on next load
        // Take care because this is unusual; only the editor could save
        // data to file... (?)
        // -----------------------------------------------------------------------------
        std::stringstream Stream;
        
        Base::CTextWriter TextWriter(Stream, 1);
        
        // -----------------------------------------------------------------------------
        // Start normal states
        // -----------------------------------------------------------------------------
        Lg ::UnloadMap::OnEnter(TextWriter);
        Gfx::UnloadMap::OnEnter(TextWriter);
        Gui::UnloadMap::OnEnter(TextWriter);
        
        // -----------------------------------------------------------------------------
        // Save stream data to file
        // -----------------------------------------------------------------------------
//         std::ofstream File("../data/level/default_map.txt", std::ios::out);
//         
//         assert(File.is_open());
//         
//         File << Stream.str();
//         
//         File.close();
        
        return Edit::CState::UnloadMap;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CUnloadMapState::InternOnLeave()
    {
        Gui::UnloadMap::OnLeave();
        Gfx::UnloadMap::OnLeave();
        Lg ::UnloadMap::OnLeave();

        BASE_CONSOLE_STREAMINFO("Edit> Leave unload map state.");
        
        return Edit::CState::UnloadMap;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CUnloadMapState::InternOnRun()
    {
        CState::EStateType NextState = CState::Exit;
        
        Lg ::UnloadMap::OnRun();
        Gfx::UnloadMap::OnRun();
        Gui::UnloadMap::OnRun();
        
        return NextState;
    }
} // namespace Edit