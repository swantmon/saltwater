
//
//  gfx_performance.h
//  gfx
//
//  Created by Tobias Schwandt on 13/11/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_typedef.h"

#include <functional>

#define GFX_BIND_DURATION_QUERY_METHOD(_Method) std::bind(_Method, this, std::placeholders::_1, std::placeholders::_2)

namespace Gfx
{
namespace Performance
{
    typedef std::function<void(Base::U32, Base::F32, Base::U64)> CDurationQueryDelegate;

    void Update();
    void OnExit();

    void BeginEvent(const Base::Char* _pEventName);
    void ResetEventStatistics(const Base::Char* _pEventName);
    void EndEvent();

    void StartDurationQuery(unsigned int _ID = 0, CDurationQueryDelegate _Delegate = nullptr);
    void EndDurationQuery();
    float EndDurationQueryWithSync();
} // namespace Performance
} // namespace Gfx