#pragma once

#include "core/core_export.h"
#include "core/core_plugin.h"

namespace Core
{
namespace PluginManager
{
    CORE_API void LoadPlugin(const char* _pName);

    CORE_API bool HasPlugin(const char* _pName);

    CORE_API IPlugin& GetPlugin(const char* _pName);
} // namespace PluginManager
} // namespace Core