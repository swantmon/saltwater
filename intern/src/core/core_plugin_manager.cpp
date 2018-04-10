
#include "engine/engine_precompiled.h"

#include "engine/engine_config.h"

#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_typedef.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_console.h"
#include "core/core_plugin_manager.h"

#include <map>
#include <string>

#ifdef PLATFORM_WINDOWS
#include "windows.h"
#elif PLATFORM_ANDROID
#include <dlfcn.h>
#endif

using namespace Core;

namespace
{
    class CPluginManager : public Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CPluginManager)

    public:

        SPluginInfo* LoadPlugin(const std::string& _rLibrary);

        bool HasPlugin(const std::string& _rName);

        IPlugin* GetPlugin(const std::string& _rName);

        void* GetPluginFunction(const std::string& _rName, const std::string& _rMethod);

    private:

        class CInternPlugin
        {
        public:
#ifdef PLATFORM_WINDOWS
            typedef HINSTANCE CInstance;
#elif PLATFORM_ANDROID
            typedef void* CInstance;
#endif // PLATFORM_WINDOWS

            CInstance m_Instance;
            SPluginInfo* m_pInfo;
        };

    private:

        typedef std::map<Base::BHash, CInternPlugin> CPlugins;

    private:

        CPlugins m_Plugins;

    private:

        CPluginManager();
        ~CPluginManager();

        Base::BHash GenerateHash(const char* _pValue);
    };
} // namespace

namespace
{
    CPluginManager::CPluginManager()
    {

    }

    // -----------------------------------------------------------------------------

    CPluginManager::~CPluginManager()
    {
        for (auto Plugin : m_Plugins)
        {
            CInternPlugin::CInstance Instance = Plugin.second.m_Instance;

#ifdef PLATFORM_WINDOWS
            FreeLibrary(Instance);
#elif PLATFORM_ANDROID
            dlclose(Instance);
#endif // PLATFORM_WINDOWS
        }

        m_Plugins.clear();
    }

    // -----------------------------------------------------------------------------

    SPluginInfo* CPluginManager::LoadPlugin(const std::string& _rLibrary)
    {
        // -----------------------------------------------------------------------------
        // Load library
        // -----------------------------------------------------------------------------
        SPluginInfo* pPluginInfo = 0;
        CInternPlugin::CInstance Instance;


#ifdef PLATFORM_WINDOWS
        WCHAR FileName[32768];

        MultiByteToWideChar(CP_UTF8, 0, _rLibrary.c_str(), -1, FileName, 32768);

        std::wstring PluginFile = std::wstring(FileName) + L".dll";

        Instance = LoadLibraryExW(PluginFile.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#elif PLATFORM_ANDROID
        std::string PluginFile = "lib" + _rLibrary + ".so";

        Instance = dlopen(PluginFile.c_str(), RTLD_NOW);
#endif // PLATFORM_WINDOWS

        if (Instance == NULL)
        {
#ifdef PLATFORM_WINDOWS
            BASE_CONSOLE_ERRORV("Plugin '%s' not found.", _rLibrary.c_str());
#elif PLATFORM_ANDROID
            BASE_CONSOLE_ERRORV("Plugin '%s' not found (Error: '%s').", _rLibrary.c_str(), dlerror());
#endif // PLATFORM_WINDOWS

            return nullptr;
        }

        BASE_CONSOLE_INFOV("Loading plugin '%s' successful.", _rLibrary.c_str());

#ifdef PLATFORM_WINDOWS
        pPluginInfo = (SPluginInfo*)GetProcAddress(Instance, "InfoExport");
#elif PLATFORM_ANDROID
        pPluginInfo = (SPluginInfo*)dlsym(Instance, "InfoExport");
#endif // PLATFORM_WINDOWS

        if (pPluginInfo == NULL)
        {
#ifdef PLATFORM_WINDOWS
            BASE_CONSOLE_ERRORV("Loading plugin information failed (Error: %i).", GetLastError());

            FreeLibrary(Instance);
#elif PLATFORM_ANDROID
            BASE_CONSOLE_ERRORV("Loading plugin information failed (Error: '%s').", dlerror());

            dlclose(Instance);
#endif // PLATFORM_WINDOWS

            return nullptr;
        }

        // -----------------------------------------------------------------------------
        // Check if plugin is already loaded.
        // -----------------------------------------------------------------------------
        Base::BHash Hash = GenerateHash(pPluginInfo->m_pPluginName);

        auto PluginIter = m_Plugins.find(Hash);

        if (PluginIter != m_Plugins.end())
        {
            BASE_CONSOLE_ERRORV("Plugin '%s' is already loaded (V=%s).", PluginIter->second.m_pInfo->m_pPluginName, PluginIter->second.m_pInfo->m_pPluginVersion);

            return PluginIter->second.m_pInfo;
        }

        BASE_CONSOLE_INFOV("Plugin name:    %s"   , pPluginInfo->m_pPluginName);
        BASE_CONSOLE_INFOV("Plugin version: %s"   , pPluginInfo->m_pPluginVersion);
        BASE_CONSOLE_INFOV("Plugin API:     %i.%i", pPluginInfo->m_APIMajorVersion, pPluginInfo->m_APIMinorVersion);
        // BASE_CONSOLE_INFOV("File:           %s", pPluginInfo->m_pFileName);
        // BASE_CONSOLE_INFOV("Class:          %s", pPluginInfo->m_pClassName);

        if (pPluginInfo->m_APIMajorVersion < ENGINE_MAJOR_VERSION || (pPluginInfo->m_APIMajorVersion == ENGINE_MAJOR_VERSION && pPluginInfo->m_APIMinorVersion < ENGINE_MINOR_VERSION))
        {
            BASE_CONSOLE_ERRORV("Plugin '%s' is out-dated (Current API version is %i.%i).", PluginIter->second.m_pInfo->m_pPluginName, ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION);

            return nullptr;
        }

        // -----------------------------------------------------------------------------
        // Save plugin
        // -----------------------------------------------------------------------------
        m_Plugins[Hash].m_pInfo    = pPluginInfo;
        m_Plugins[Hash].m_Instance = Instance;

        return m_Plugins[Hash].m_pInfo;
    }

    // -----------------------------------------------------------------------------

    bool CPluginManager::HasPlugin(const std::string& _rName)
    {
        Base::BHash Hash = GenerateHash(_rName.c_str());

        return m_Plugins.find(Hash) != m_Plugins.end();
    }

    // -----------------------------------------------------------------------------

    IPlugin* CPluginManager::GetPlugin(const std::string& _rName)
    {
        Base::BHash Hash = GenerateHash(_rName.c_str());

        auto PluginIter = m_Plugins.find(Hash);

        if (PluginIter == m_Plugins.end())
        {
            BASE_CONSOLE_ERRORV("Plugin '%s' is not registered.", _rName.c_str());

            return nullptr;
        }

        return &PluginIter->second.m_pInfo->GetInstance();
    }

    // -----------------------------------------------------------------------------

    void* CPluginManager::GetPluginFunction(const std::string& _rName, const std::string& _rFunction)
    {
        Base::BHash Hash = GenerateHash(_rName.c_str());

        auto PluginIter = m_Plugins.find(Hash);

        if (PluginIter == m_Plugins.end()) return nullptr;

        void* pMethod = nullptr;

#ifdef PLATFORM_WINDOWS
        pMethod = (void*)GetProcAddress(m_Plugins[Hash].m_Instance, _rFunction.c_str());
#elif PLATFORM_ANDROID
        pMethod = (void*)dlsym(m_Plugins[Hash].m_Instance, _rFunction.c_str());
#endif // PLATFORM_WINDOWS

        return pMethod;
    }

    // -----------------------------------------------------------------------------

    Base::BHash CPluginManager::GenerateHash(const char* _pValue)
    {
        return Base::CRC32(_pValue, sizeof(char) * static_cast<unsigned int>(strlen(_pValue)));
    }
} // namespace

namespace Core
{
namespace PluginManager
{
    SPluginInfo* LoadPlugin(const std::string& _rLibrary)
    {
        return CPluginManager::GetInstance().LoadPlugin(_rLibrary);
    }

    // -----------------------------------------------------------------------------

    bool HasPlugin(const std::string& _rName)
    {
        return CPluginManager::GetInstance().HasPlugin(_rName);
    }

    // -----------------------------------------------------------------------------

    IPlugin* GetPlugin(const std::string& _rName)
    {
        return CPluginManager::GetInstance().GetPlugin(_rName);
    }

    // -----------------------------------------------------------------------------

    void* GetPluginFunction(const std::string& _rName, const std::string& _rFunction)
    {
        return CPluginManager::GetInstance().GetPluginFunction(_rName, _rFunction);
    }
} // namespace PluginManager
} // namespace Core