//
//  main.cpp
//  game_sdl
//
//  Created by Tobias Schwandt on 14/10/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#include "app/app_application.h"
#include "app/app_config.h"

#include "base/base_console.h"
#include "base/base_exception.h"
#include "base/base_input_event.h"

#include <SDL2/SDL.h>
#undef main

#include <iostream>
#include <stdio.h>

#define APP_NAME "Game"
#define APP_WIDTH 1280
#define APP_HEIGHT 720
#define APP_POSITION_X SDL_WINDOWPOS_UNDEFINED
#define APP_POSITION_Y SDL_WINDOWPOS_UNDEFINED
#define APP_STYLE SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE

// -----------------------------------------------------------------------------

Base::Short2  s_LatestMousePosition(0, 0);
SDL_Window*   s_pWindow                   = nullptr;

// -----------------------------------------------------------------------------

bool HandleEvents(const SDL_Event& _rEvent);

// -----------------------------------------------------------------------------

int main(int _Argc, const char* _pArgv[])
{
    SDL_GLContext ContextHandle = 0;
    
    // -----------------------------------------------------------------------------
    // Initialize SDL2 library.
    // We initialize everything like input events, video context, mouse, keyboard,
    // sound, ...
    // -----------------------------------------------------------------------------
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        BASE_CONSOLE_STREAMERROR("Can't initialize SDL2.");
        
        return -1;
    }
    
    // -----------------------------------------------------------------------------
    // Initialize core profile of opengl. We use OpenGL 4.1 because of several
    // reasons like different shader functions and some performance reasons.
    // Moreover we define different specifications like DoubleBuffer, PixelFormat
    // and so on.
    //
    // Example:
    //    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    //    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    //    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    //    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    //    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    //    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
    // -----------------------------------------------------------------------------
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

#if APP_DEBUG_MODE == 1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
    
    // -----------------------------------------------------------------------------
    // Creating a window to show the game
    // -----------------------------------------------------------------------------
    s_pWindow = SDL_CreateWindow(APP_NAME, APP_POSITION_X, APP_POSITION_Y, APP_WIDTH, APP_HEIGHT, APP_STYLE);
    
    if (s_pWindow == nullptr)
    {
        BASE_CONSOLE_STREAMERROR("Can't create an SDL window.");
        
        SDL_Quit();
        
        return -1;
    }
    
    // -----------------------------------------------------------------------------
    // With a window in our hands we create a context for OpenGL. This context
    // handle we can use to initialize our application.
    // -----------------------------------------------------------------------------
    ContextHandle = SDL_GL_CreateContext(s_pWindow);
    
    // -----------------------------------------------------------------------------
    // With an window and context we initialize our application and run our game.
    // Furthermore we handle different events by the window.
    // -----------------------------------------------------------------------------
    int       ApplicationMessage = 0;
    int       WindowMessage      = 0;
    SDL_Event WindowEvent;
    
    App::Application::OnStart(APP_WIDTH, APP_HEIGHT);
    
    try
    {
        for (; ApplicationMessage == 0 && WindowMessage == 0; )
        {
            for (; SDL_PollEvent(&WindowEvent); )
            {
                if(!HandleEvents(WindowEvent))
                {
                    WindowMessage = 1;
                    
                    break;
                }
            }
            
            ApplicationMessage = App::Application::OnRun();
            
            SDL_GL_SwapWindow(s_pWindow);
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
    
    App::Application::OnExit();

    // -----------------------------------------------------------------------------
    // At the end we have to clean our context and window.
    // -----------------------------------------------------------------------------
    SDL_GL_DeleteContext(ContextHandle);
    SDL_DestroyWindow(s_pWindow);
    SDL_Quit();
    
    return 0;
}

// -----------------------------------------------------------------------------

bool HandleEvents(const SDL_Event& _rEvent)
{
    switch (_rEvent.window.event)
    {
        case SDL_WINDOWEVENT_RESIZED:
        {
            int WindowWidth;
            int WindowHeight;
            
            SDL_GetWindowSize(s_pWindow, &WindowWidth, &WindowHeight);
            
            App::Application::OnResize(WindowWidth, WindowHeight);
        }
        break;
            
        case SDL_WINDOWEVENT_CLOSE:
        {
            return false;
        }
        break;
    };
    
    switch (_rEvent.type)
    {
        case SDL_QUIT:
        {
            return false;
        }
        break;
            
        case SDL_KEYDOWN:
        {
            unsigned int Key = _rEvent.key.keysym.sym;
            unsigned int Mod = _rEvent.key.keysym.mod;
            
            Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::KeyPressed, Key, Mod);
            
            App::Application::OnInputEvent(NewInput);
        }
        break;
            
        case SDL_KEYUP:
        {
            unsigned int Key = _rEvent.key.keysym.sym;
            unsigned int Mod = _rEvent.key.keysym.mod;
            
            Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::KeyReleased, Key, Mod);
            
            App::Application::OnInputEvent(NewInput);
        }
        break;
            
        case SDL_MOUSEBUTTONDOWN:
        {
            Base::CInputEvent::EAction MouseAction = Base::CInputEvent::UndefinedAction;
            
            if (SDL_BUTTON(SDL_BUTTON_LEFT))
            {
                MouseAction = Base::CInputEvent::MouseLeftPressed;
            }
            else if (SDL_BUTTON(SDL_BUTTON_RIGHT))
            {
                MouseAction = Base::CInputEvent::MouseMiddlePressed;
            }
            else if (SDL_BUTTON(SDL_BUTTON_RIGHT))
            {
                MouseAction = Base::CInputEvent::MouseRightPressed;
            }
            
            if (MouseAction != Base::CInputEvent::UndefinedAction)
            {
                Base::CInputEvent NewInput(Base::CInputEvent::Input, MouseAction, Base::CInputEvent::Mouse, s_LatestMousePosition);
                
                App::Application::OnInputEvent(NewInput);
            }
        }
        break;
            
        case SDL_MOUSEBUTTONUP:
        {
            Base::CInputEvent::EAction MouseAction = Base::CInputEvent::UndefinedAction;
            
            if (SDL_BUTTON(SDL_BUTTON_LEFT))
            {
                MouseAction = Base::CInputEvent::MouseLeftReleased;
            }
            else if (SDL_BUTTON(SDL_BUTTON_MIDDLE))
            {
                MouseAction = Base::CInputEvent::MouseMiddleReleased;
            }
            else if (SDL_BUTTON(SDL_BUTTON_RIGHT))
            {
                MouseAction = Base::CInputEvent::MouseRightReleased;
            }
            
            if (MouseAction != Base::CInputEvent::UndefinedAction)
            {
                Base::CInputEvent NewInput(Base::CInputEvent::Input, MouseAction, Base::CInputEvent::Mouse, s_LatestMousePosition);
                
                App::Application::OnInputEvent(NewInput);
            }
        }
        break;
        
        case SDL_MOUSEWHEEL:
        {
            int WheelData = _rEvent.wheel.y;
            
            Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseWheel, Base::CInputEvent::Mouse, s_LatestMousePosition, WheelData);
            
            App::Application::OnInputEvent(NewInput);
        }
        break;
            
        case SDL_MOUSEMOTION:
        {
            int MousePositionX;
            int MousePositionY;
            
            SDL_GetMouseState(&MousePositionX, &MousePositionY);
            
            s_LatestMousePosition[0] = MousePositionX;
            s_LatestMousePosition[1] = MousePositionY;
            
            Base::CInputEvent NewInput(Base::CInputEvent::Input, Base::CInputEvent::MouseMove, Base::CInputEvent::Mouse, s_LatestMousePosition);
            
            App::Application::OnInputEvent(NewInput);
        }
        break;
    }
    
    return true;
}

