//
//  main.cpp
//  game
//
//  Created by Tobias Schwandt on 14/10/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#include "game/game_precompiled.h"

#include "base/base_console.h"
#include "base/base_exception.h"

#include "game/game_application.h"

int main(int _Argc, const char* _pArgv[])
{
    try
    {
        Game::Application::OnStart(1280, 720);

        Game::Application::OnRun();
    }
    catch (const Base::CException& _rException)
    {
        BASE_CONSOLE_ERROR("An Exception stops application");
        BASE_CONSOLE_INFOV(" > Reason:   %s"   , _rException.GetText());
        BASE_CONSOLE_INFOV(" > Code:     %i"   , _rException.GetCode());
        BASE_CONSOLE_INFOV(" > Location: %s:%i", _rException.GetFile(), _rException.GetLine());
    }
    catch (...)
    {
        BASE_CONSOLE_ERROR("An undefined exception stops application");
    }
    
    try
    {
        Game::Application::OnExit();
    }
    catch (const Base::CException& _rException)
    {
        BASE_CONSOLE_ERROR("An Exception stops application");
        BASE_CONSOLE_INFOV(" > Reason:   %s", _rException.GetText());
        BASE_CONSOLE_INFOV(" > Code:     %i", _rException.GetCode());
        BASE_CONSOLE_INFOV(" > Location: %s:%i", _rException.GetFile(), _rException.GetLine());
    }
    catch (...)
    {
        BASE_CONSOLE_ERROR("An undefined exception stops application");
    }
    
    return 0;
}
