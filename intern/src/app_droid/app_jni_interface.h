
#pragma once

#include "base/base_vector2.h"

namespace App
{
namespace JNI
{
    void* GetJavaEnvironment();

    void* GetContext();

    int GetDeviceRotation();

    const Base::Int2& GetDeviceDimension();

    bool HasCameraPermission();
} // namespace JNI
} // namespace App
