
#pragma once

#include "mr/mr_marker.h"

namespace MR
{
namespace MarkerManager
{
    void OnStart();

    void OnExit();

    void Update();

    const CMarker* AcquireNewMarker(float _X, float _Y);
    void ReleaseMarker(const CMarker* _pMarker);
} // namespace MarkerManager
} // namespace MR