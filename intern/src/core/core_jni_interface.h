
#pragma once

#include "base/base_vector2.h"

#include <string>
#include <vector>

namespace Core
{
namespace JNI
{
    void* GetJavaEnvironment();

    void* GetContext();

    int GetDeviceRotation();

    const Base::Int2& GetDeviceDimension();

    bool CheckPermission(const std::string& _rPermission);

    void AcquirePermissions(const std::string* _pPermissions, unsigned int _NumberOfPermissions);
} // namespace JNI
} // namespace Core
