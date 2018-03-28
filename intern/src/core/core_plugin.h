
#pragma once

#include "core/core_plugin_config.h"

namespace Core
{
    // -----------------------------------------------------------------------------
    // Plugin declaration
    // -----------------------------------------------------------------------------
    class IPlugin
    {
    public:

        IPlugin() {};
        virtual ~IPlugin() {};

        virtual void OnStart() = 0;
        virtual void OnExit() = 0;
        virtual void Update() = 0;
    };

    // -----------------------------------------------------------------------------
    // Plugin function
    // -----------------------------------------------------------------------------
    typedef IPlugin& (*PluginInstance)();

    // -----------------------------------------------------------------------------
    // Plugin info
    // -----------------------------------------------------------------------------
    struct SPluginInfo
    {
        int            m_APIversion;
        const char*    m_pFileName;
        const char*    m_pClassName;
        const char*    m_pPluginName;
        const char*    m_pPluginVersion;
        PluginInstance GetInstance;
    };
} // namespace Core