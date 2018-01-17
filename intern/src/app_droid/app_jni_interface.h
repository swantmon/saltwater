
#pragma once

namespace App
{
namespace JNI
{
    void* GetJavaEnvironment();

    void* GetContext();

    bool HasCameraPermission();
} // namespace JNI
} // namespace App
