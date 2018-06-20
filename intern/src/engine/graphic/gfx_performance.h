
#pragma once

#include "base/base_typedef.h"

#include <functional>

#define GFX_BIND_DURATION_QUERY_METHOD(_Method) std::bind(_Method, this, std::placeholders::_1, std::placeholders::_2)

namespace Gfx
{
namespace Performance
{
    typedef std::function<void(Base::U32, Base::F32, Base::U64)> CDurationQueryDelegate;

    void OnStart();
    void Update();
    void OnExit();

    ENGINE_API void BeginEvent(const Base::Char* _pEventName);
    ENGINE_API void ResetEventStatistics(const Base::Char* _pEventName);
    ENGINE_API void EndEvent();

    ENGINE_API void StartDurationQuery(unsigned int _ID = 0, CDurationQueryDelegate _Delegate = nullptr);
    ENGINE_API void EndDurationQuery();
    ENGINE_API float EndDurationQueryWithSync();
} // namespace Performance
} // namespace Gfx