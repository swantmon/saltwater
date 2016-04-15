//
//  main.cpp
//  game
//
//  Created by Tobias Schwandt on 14/10/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#include "app/app_application.h"
#include "app/app_config.h"

#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_input_event.h"

#include "editor/edit_application.h"

#include "editor_gui/edit_gui.h"

#define APP_NAME "Editor"
#define APP_WIDTH 1280
#define APP_HEIGHT 720

// -----------------------------------------------------------------------------

int main(int _Argc, char* _pArgv[])
{    
    Edit::GUI::Create(_Argc, _pArgv);

    Edit::GUI::Setup(APP_WIDTH, APP_HEIGHT);

    Edit::GUI::Show();

    // -----------------------------------------------------------------------------
    // With an window and context we initialize our application and run our game.
    // Furthermore we handle different events by the window.
    // -----------------------------------------------------------------------------
    int ApplicationMessage = 0;
    int WindowMessage      = 0;
    
    //Edit::Runtime::OnStart(APP_WIDTH, APP_HEIGHT);
    
    try
    {
        for (; ApplicationMessage == 0 && WindowMessage == 0; )
        {            
            Edit::GUI::ProcessEvents();

            //ApplicationMessage = Edit::Runtime::OnRun();
        }
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
    
    //Edit::Runtime::OnExit();

    Edit::GUI::Destroy();

    return 0;
}