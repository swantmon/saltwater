
#include "game/game_precompiled.h"

#include "base/base_console.h"
#include "base/base_serialize_text_writer.h"

#include "game/game_unload_map_state.h"

#include "graphic/gfx_unload_map_state.h"

#include "gui/gui_unload_map_state.h"

#include "logic/lg_unload_map_state.h"

#include <assert.h>
#include <fstream>
#include <string>


namespace Game
{
    CUnloadMapState& CUnloadMapState::GetInstance()
    {
        static CUnloadMapState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Game

namespace Game
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
        BASE_CONSOLE_STREAMINFO("Game> Enter unload map state.");

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
        
        return Game::CState::UnloadMap;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CUnloadMapState::InternOnLeave()
    {
        Gui::UnloadMap::OnLeave();
        Gfx::UnloadMap::OnLeave();
        Lg ::UnloadMap::OnLeave();

        BASE_CONSOLE_STREAMINFO("Game> Leave unload map state.");
        
        return Game::CState::UnloadMap;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CUnloadMapState::InternOnRun()
    {
        CState::EStateType NextState = CState::UnloadMap;
        
        switch(Lg::UnloadMap::OnRun())
        {
            case Lg::UnloadMap::SResult::UnloadMap:
            NextState = CState::UnloadMap;
            break;
            case Lg::UnloadMap::SResult::Exit:
            NextState = CState::Exit;
            break;
        }
        
        Gfx::UnloadMap::OnRun();
        Gui::UnloadMap::OnRun();
        
        return NextState;
    }
} // namespace Game