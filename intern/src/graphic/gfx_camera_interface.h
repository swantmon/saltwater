
#pragma once

#include "base/base_matrix3x3.h"
#include "base/base_vector2.h"
#include "base/base_vector3.h"

namespace Gfx
{
namespace Cam
{
    void SetPosition(const Base::Float3& _rPosition);
    const Base::Float3& GetPosition();

    void SetRotation(const Base::Float3x3& _rMatrix);
    const Base::Float3x3& GetRotation();
    
    void InjectCameraMatrix(const Base::Float3x3& _rCameraMatrix);
    
    void Update();
} // namespace Cam
} // namespace Gfx