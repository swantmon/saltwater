
#pragma once

#include "core/core_plugin_config.h"

namespace Core
{
    #ifdef PLATFORM_WINDOWS
    #ifndef PLATFORM_SHARED_LIBRARY
    #define PLATFORM_EXTERN __declspec(dllexport)
    #else
    #define PLATFORM_EXTERN __declspec(dllimport)
    #endif
    #else
    #define PLATFORM_EXTERN // nothing
    #endif

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
    BASE_EXTERN typedef IPlugin& (*PluginInstance)();

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

    // -----------------------------------------------------------------------------
    // Macro
    // -----------------------------------------------------------------------------
    #define CORE_PLUGIN(_ClassName, _PluginName, _PluginVersion)                   \
    extern "C"                                                                     \
    {                                                                              \
        BASE_EXTERN Core::IPlugin& GetInstance()                                   \
        {                                                                          \
            static _ClassName s_Instance;                                          \
            return s_Instance;                                                     \
        }                                                                          \
        BASE_EXTERN Core::SPluginInfo InfoExport =                                 \
        {                                                                          \
            CORE_PLUGIN_API_VERSION,                                               \
            __FILE__,                                                              \
            #_ClassName,                                                           \
            _PluginName,                                                           \
            _PluginVersion,                                                        \
            GetInstance,                                                           \
        };                                                                         \
    }
} // namespace Core