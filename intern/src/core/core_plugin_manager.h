#pragma once

#include "core/core_plugin.h"

namespace Core
{
namespace PluginManager
{
    void LoadPlugin(const char* _pName);

    bool HasPlugin(const char* _pName);

    IPlugin& GetPlugin(const char* _pName);
} // namespace PluginManager
} // namespace Core