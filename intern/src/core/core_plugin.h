
#pragma once

#include "base/base_defines.h"

namespace Core
{
    // -----------------------------------------------------------------------------
    // Config
    // -----------------------------------------------------------------------------
    #define CORE_PLUGIN_API_VERSION 1
    #define CORE_PLUGIN_API_EXPORT __declspec(dllexport)

    // -----------------------------------------------------------------------------
    // Plugin interface
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
    CORE_PLUGIN_API_EXPORT typedef IPlugin& (*PluginInstance)();

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
    #define CORE_PLUGIN_INFO(_ClassName, _PluginName, _PluginVersion)              \
    extern "C"                                                                     \
    {                                                                              \
        CORE_PLUGIN_API_EXPORT Core::IPlugin& GetInstance()                        \
        {                                                                          \
            static _ClassName s_Instance;                                          \
            return s_Instance;                                                     \
        }                                                                          \
        CORE_PLUGIN_API_EXPORT Core::SPluginInfo InfoExport =                      \
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