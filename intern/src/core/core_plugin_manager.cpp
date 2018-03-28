
#include "core/core_precompiled.h"

#include "base/base_console.h"
#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_typedef.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_config.h"
#include "core/core_plugin_manager.h"

#include <map>
#include <string>

#ifdef PLATFORM_WINDOWS
#include "windows.h"
#endif

using namespace Core;

namespace
{
    class CPluginManager : public Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CPluginManager)

    public:

        void LoadPlugin(const char* _pName);

        bool HasPlugin(const char* _pName);

        IPlugin& GetPlugin(const char* _pName);

    private:

        class CInternPlugin
        {
        public:
#ifdef PLATFORM_WINDOWS
            HINSTANCE m_Instance;
#endif // PLATFORM_WINDOWS

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
#ifdef PLATFORM_WINDOWS
            HINSTANCE Instance = Plugin.second.m_Instance;

            FreeLibrary(Instance);
#endif // PLATFORM_WINDOWS
        }

        m_Plugins.clear();
    }

    // -----------------------------------------------------------------------------

    void CPluginManager::LoadPlugin(const char* _pName)
    {
        // -----------------------------------------------------------------------------
        // Check if plugin is already loaded.
        // -----------------------------------------------------------------------------
        Base::BHash Hash = GenerateHash(_pName);

        auto PluginIter = m_Plugins.find(Hash);

        if (PluginIter != m_Plugins.end())
        {
            BASE_CONSOLE_ERRORV("Plugin '%s' is already loaded (V=%s).", PluginIter->second.m_pInfo->m_pPluginName, PluginIter->second.m_pInfo->m_pPluginVersion);

            return;
        }

        // -----------------------------------------------------------------------------
        // Load library
        // -----------------------------------------------------------------------------
#ifdef PLATFORM_WINDOWS
        WCHAR FileName[32768];

        MultiByteToWideChar(CP_UTF8, 0, _pName, -1, FileName, 32768);

#if APP_DEBUG_MODE == 1
        std::wstring PluginFile = L"plugin_" + std::wstring(FileName) + L"d.dll";
#else
        std::wstring PluginFile = L"plugin_" + std::wstring(FileName) + L"r.dll";
#endif // APP_DEBUG_MODE
        

        HINSTANCE Instance = LoadLibraryExW(PluginFile.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

        if (Instance == NULL)
        {
            BASE_CONSOLE_ERRORV("Loading plugin '%s' failed.", _pName);

            return;
        }

        BASE_CONSOLE_INFOV("Loading plugin '%s' successful.", _pName);

        SPluginInfo* pPluginInfo = (SPluginInfo*)GetProcAddress(Instance, "InfoExport");

        if (pPluginInfo == NULL)
        {
            BASE_CONSOLE_ERROR("Loading plugin information failed.");

            FreeLibrary(Instance);

            return;
        }
#endif // PLATFORM_WINDOWS

        // BASE_CONSOLE_INFOV("API:            %i", pPluginInfo->m_APIversion);
        // BASE_CONSOLE_INFOV("File:           %s", pPluginInfo->m_pFileName);
        // BASE_CONSOLE_INFOV("Class:          %s", pPluginInfo->m_pClassName);
        BASE_CONSOLE_INFOV("Plugin name:    %s", pPluginInfo->m_pPluginName);
        BASE_CONSOLE_INFOV("Plugin version: %s", pPluginInfo->m_pPluginVersion);

        // -----------------------------------------------------------------------------
        // Save plugin
        // -----------------------------------------------------------------------------
        m_Plugins[Hash].m_pInfo    = pPluginInfo;

#ifdef PLATFORM_WINDOWS
        m_Plugins[Hash].m_Instance = Instance;
#endif // PLATFORM_WINDOWS
    }

    // -----------------------------------------------------------------------------

    bool CPluginManager::HasPlugin(const char* _pName)
    {
        Base::BHash Hash = GenerateHash(_pName);

        return m_Plugins.find(Hash) != m_Plugins.end();
    }

    // -----------------------------------------------------------------------------

    IPlugin& CPluginManager::GetPlugin(const char* _pName)
    {
        Base::BHash Hash = GenerateHash(_pName);

        auto PluginIter = m_Plugins.find(Hash);

        if (PluginIter == m_Plugins.end()) BASE_THROWV("Plugin '%s' is not registered.", _pName);

        return PluginIter->second.m_pInfo->GetInstance();
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
    void LoadPlugin(const char* _pName)
    {
        CPluginManager::GetInstance().LoadPlugin(_pName);
    }

    // -----------------------------------------------------------------------------

    bool HasPlugin(const char* _pName)
    {
        return CPluginManager::GetInstance().HasPlugin(_pName);
    }

    // -----------------------------------------------------------------------------

    IPlugin& GetPlugin(const char* _pName)
    {
        return CPluginManager::GetInstance().GetPlugin(_pName);
    }
} // namespace PluginManager
} // namespace Core