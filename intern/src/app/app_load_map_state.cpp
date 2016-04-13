
#include "app/app_load_map_state.h"

#include "base/base_console.h"
#include "base/base_serialize_text_reader.h"

#include "graphic/gfx_load_map_state.h"

#include "gui/gui_load_map_state.h"

#include "logic/lg_load_map_state.h"

#include <assert.h>
#include <fstream>
#include <string>

namespace App
{
    CLoadMapState& CLoadMapState::GetInstance()
    {
        static CLoadMapState s_Singleton;
        
        return s_Singleton;
    }
} // namespace App

namespace App
{
    CLoadMapState::CLoadMapState()
        : m_pMapfile("")
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CLoadMapState::~CLoadMapState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CLoadMapState::SetMapfile(const char* _pFilename)
    {
        BASE_CONSOLE_STREAMINFO("App> Setting load level to " << _pFilename);

        m_pMapfile = _pFilename;
    }
    
    // -----------------------------------------------------------------------------
    
    const char* CLoadMapState::GetMapfile() const
    {
        return m_pMapfile;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CLoadMapState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("App> Enter load level state.");

        // -----------------------------------------------------------------------------
        // Load an map given by name from the gui. So the steps are as following:
        // 1. Get mapname from property
        // 2. Load map by file into text reader
        // 3. Give this text reader to every part of the game (map, graphic, ...)
        // -----------------------------------------------------------------------------
        
        BASE_CONSOLE_STREAMINFO("App> Load level " << m_pMapfile);

        // 1.
        const char* pFilename = m_pMapfile;
        
        // 2.
        std::stringstream Stream;
        
        std::ifstream File(pFilename, std::ios::in);
        
        Stream << File.rdbuf();
        
        File.close();
        
        Base::CTextReader TextReader(Stream, 1);
        
        // -----------------------------------------------------------------------------
        // 3. Start normal states
        // -----------------------------------------------------------------------------
        Lg ::LoadMap::OnEnter(TextReader);
        Gui::LoadMap::OnEnter(TextReader);
        Gfx::LoadMap::OnEnter(TextReader);
        
        return App::CState::LoadMap;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CLoadMapState::InternOnLeave()
    {
        Gfx::LoadMap::OnLeave();
        Gui::LoadMap::OnLeave();
        Lg ::LoadMap::OnLeave();        

        BASE_CONSOLE_STREAMINFO("App> Leave load level state.");
        
        return App::CState::LoadMap;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CLoadMapState::InternOnRun()
    {
        CState::EStateType NextState = CState::LoadMap;
        
        switch(Lg::LoadMap::OnRun())
        {
            case Lg::LoadMap::SResult::LoadMap:
            NextState = CState::LoadMap;
            break;
            case Lg::LoadMap::SResult::Play:
            NextState = CState::Play;
            break;
        }
        
        Gui::LoadMap::OnRun();
        Gfx::LoadMap::OnRun();
        
        return NextState;
    }
} // namespace App