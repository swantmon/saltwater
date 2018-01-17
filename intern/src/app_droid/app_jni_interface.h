
#pragma once

namespace App
{
namespace JNI
{
    void* GetJavaEnvironment();

    void* GetContext();

    int GetDeviceRotation();

    bool HasCameraPermission();
} // namespace JNI
} // namespace App
