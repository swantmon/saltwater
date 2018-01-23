
#pragma once

#include "base/base_vector2.h"

#include <functional>
#include <string>
#include <vector>

namespace Core
{
namespace JNI
{
    typedef std::function<void(const std::string& _rPermission, int _GrantResult)> COnAcquirePermissionDelegate;
} // namespace JNI
} // namespace Core

#define CORE_JNI_ON_ACQUIRE_PERMISSION_METHOD(_Method) std::bind(_Method, this, std::placeholders::_1, std::placeholders::_2)

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

    void RegisterOnAcquirePermission(COnAcquirePermissionDelegate _NewDelegate);
} // namespace JNI
} // namespace Core
