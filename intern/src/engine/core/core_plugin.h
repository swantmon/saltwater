
#pragma once

#include "base/base_defines.h"

namespace Core
{
    // -----------------------------------------------------------------------------
    // Config
    // -----------------------------------------------------------------------------
#ifdef PLATFORM_WINDOWS
    #define CORE_PLUGIN_API_EXPORT __declspec(dllexport)
#elif PLATFORM_ANDROID
    #define CORE_PLUGIN_API_EXPORT
#endif

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
        virtual void OnPause() = 0;
        virtual void OnResume() = 0;

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
        int            m_APIMajorVersion;
        int            m_APIMinorVersion;
        const char*    m_pPluginName;
        const char*    m_pPluginVersion;
        const char*    m_pPluginDescription;
        PluginInstance GetInstance;
    };

    // -----------------------------------------------------------------------------
    // Macro
    // -----------------------------------------------------------------------------
    #define CORE_PLUGIN_INFO(_ClassName, _PluginName, _PluginVersion, _PluginDesc) \
    extern "C"                                                                     \
    {                                                                              \
        CORE_PLUGIN_API_EXPORT Core::IPlugin& GetInstance()                        \
        {                                                                          \
            static _ClassName s_Instance;                                          \
            return s_Instance;                                                     \
        }                                                                          \
        CORE_PLUGIN_API_EXPORT Core::SPluginInfo InfoExport =                      \
        {                                                                          \
            ENGINE_MAJOR_VERSION,                                                  \
            ENGINE_MINOR_VERSION,                                                  \
            _PluginName,                                                           \
            _PluginVersion,                                                        \
            _PluginDesc,                                                           \
            GetInstance,                                                           \
        };                                                                         \
    }
} // namespace Core