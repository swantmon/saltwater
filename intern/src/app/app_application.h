
//
//  app_application.h
//  app
//
//  Created by Tobias Schwandt on 18/09/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_typedef.h"

#include <functional>

namespace Base
{
    class CInputEvent;
} // namespace Base

namespace App
{
namespace Application
{
    typedef std::function<void(int, int)> CResizeDelegate;
} // namespace Application
} // namespace App

#define APP_BIND_RESIZE_METHOD(_Method) std::bind(_Method, this, std::placeholders::_1, std::placeholders::_2)

namespace App
{
namespace Application
{
    void OnStart();
    void OnExit();

    void Update();
    
    void OnInputEvent(Base::CInputEvent& _rInputEvent);
    void OnResize(int _Width, int _Height);
    
    void RegisterResizeHandler(CResizeDelegate _NewDelgate);
    
    double GetDeltaTimeLastFrame();
    
    Base::U64 GetNumberOfFrame();
} // namespace Application
} // namespace App