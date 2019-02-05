
#include "engine/engine_precompiled.h"

#include "engine/engine_config.h"

#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_typedef.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "engine/core/core_console.h"
#include "engine/core/core_plugin_manager.h"

#include <filesystem>
#include <unordered_map>
#include <regex>
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

		void Start();

		void Update();

		void Pause();

		void Resume();

		void Exit();

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

		CInternPlugin::CInstance InternLoadLibrary(const std::string& _rFileName);
		bool InternGetPluginName(CPluginManager::CInternPlugin::CInstance _Instance, std::string& _rName);
		void InternCloseLibrary(CPluginManager::CInternPlugin::CInstance _Instance);

    private:

		using CPluginFiles = std::unordered_map<std::string, std::string>;
        using CPlugins = std::unordered_map<std::string, CInternPlugin>;

    private:

		CPluginFiles m_PluginFiles;
        CPlugins m_Plugins;

    private:

        CPluginManager();
        ~CPluginManager();

		std::regex m_PluginRegex;
    };
} // namespace

namespace
{
#ifdef PLATFORM_WINDOWS

	CPluginManager::CInternPlugin::CInstance CPluginManager::InternLoadLibrary(const std::string& _rFileName)
	{
		WCHAR FileName[32768];

		MultiByteToWideChar(CP_UTF8, 0, _rFileName.c_str(), -1, FileName, 32768);

		std::wstring PluginFile = std::wstring(FileName);

		return LoadLibraryExW(PluginFile.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	}

	bool CPluginManager::InternGetPluginName(CPluginManager::CInternPlugin::CInstance _Instance, std::string& _rName)
	{
		if (_Instance == nullptr)
		{
			return false;
		}

		_rName = ((SPluginInfo*)GetProcAddress(_Instance, "InfoExport"))->m_pPluginName;
	}

	void CPluginManager::InternCloseLibrary(CPluginManager::CInternPlugin::CInstance _Instance)
	{
		FreeLibrary(_Instance);
	}

#elif PLATFORM_ANDROID

	CPluginManager::CInternPlugin::CInstance CPluginManager::InternLoadLibrary(const std::string& _rFileName)
	{
		return dlopen(_rFileName.c_str(), RTLD_NOW);
	}

	bool CPluginManager::InternGetPluginName(CPluginManager::CInternPlugin::CInstance _Instance, std::string& _rName)
	{
		if (_Instance == nullptr)
		{
			return false;
		}

		_rName = ((SPluginInfo*)dlsym(Instance, "InfoExport"))->m_pPluginName;
	}

	void CPluginManager::InternCloseLibrary(CPluginManager::CInternPlugin::CInstance _Instance)
	{
		dlclose(_Instance);
	}

#endif //PLATFORM_WINDOWS

	void CPluginManager::Start()
	{
		// Find all files in the current path

		auto Path = "";
		for (const auto & rEntry : std::filesystem::directory_iterator(Path))
		{
			// Find the ones that look like Saltwater plugins

			auto PluginFileName = rEntry.path().string();
			if (std::regex_match(PluginFileName, m_PluginRegex))
			{
				// Found one! Now load it and get the plugin plugin's name

				CInternPlugin::CInstance Instance = InternLoadLibrary(PluginFileName);

				std::string PluginName;
				if (InternGetPluginName(Instance, PluginName))
				{
					m_PluginFiles[PluginName] = PluginFileName;
				}

				InternCloseLibrary(Instance);
			}
		}
	}

	// -----------------------------------------------------------------------------

	void CPluginManager::Update()
	{

	}

	// -----------------------------------------------------------------------------

	void CPluginManager::Pause()
	{

	}

	// -----------------------------------------------------------------------------

	void CPluginManager::Resume()
	{

	}

	// -----------------------------------------------------------------------------

	void CPluginManager::Exit()
	{

	}

	// -----------------------------------------------------------------------------

    CPluginManager::CPluginManager()
    {
#ifdef PLATFORM_WINDOWS
	#ifdef ENGINE_DEBUG_MODE
		m_PluginRegex = std::regex("^plugin_.*d.dll$");
	#else
		m_PluginRegex = std::regex("^plugin_.*r.dll$");
	#endif // ENGINE_DEBUG_MODE
#elif PLATFORM_ANDROID
		m_PluginRegex = std::regex("^libplugin_.*.so$");
#endif
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

#ifdef ENGINE_DEBUG_MODE
        std::wstring PluginFile = std::wstring(FileName) + L"d.dll";
#else
        std::wstring PluginFile = std::wstring(FileName) + L"r.dll";
#endif // APP_DEBUG_MODE

        Instance = LoadLibraryExW(PluginFile.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#elif PLATFORM_ANDROID
        std::string PluginFile = "lib" + _rLibrary + ".so";

        Instance = dlopen(PluginFile.c_str(), RTLD_NOW);
#endif // PLATFORM_WINDOWS

        if (Instance == NULL)
        {
#ifdef PLATFORM_WINDOWS
            ENGINE_CONSOLE_ERRORV("Plugin '%s' not found.", _rLibrary.c_str());
#elif PLATFORM_ANDROID
            ENGINE_CONSOLE_ERRORV("Plugin '%s' not found (Error: '%s').", _rLibrary.c_str(), dlerror());
#endif // PLATFORM_WINDOWS

            return nullptr;
        }

        ENGINE_CONSOLE_INFOV("Loading plugin '%s' successful.", _rLibrary.c_str());

#ifdef PLATFORM_WINDOWS
        pPluginInfo = (SPluginInfo*)GetProcAddress(Instance, "InfoExport");
#elif PLATFORM_ANDROID
        pPluginInfo = (SPluginInfo*)dlsym(Instance, "InfoExport");
#endif // PLATFORM_WINDOWS

        if (pPluginInfo == NULL)
        {
#ifdef PLATFORM_WINDOWS
            ENGINE_CONSOLE_ERRORV("Loading plugin information failed (Error: %i).", GetLastError());

            FreeLibrary(Instance);
#elif PLATFORM_ANDROID
            ENGINE_CONSOLE_ERRORV("Loading plugin information failed (Error: '%s').", dlerror());

            dlclose(Instance);
#endif // PLATFORM_WINDOWS

            return nullptr;
        }

        // -----------------------------------------------------------------------------
        // Check if plugin is already loaded.
        // -----------------------------------------------------------------------------
        auto PluginIter = m_Plugins.find(_rLibrary);

        if (PluginIter != m_Plugins.end())
        {
            ENGINE_CONSOLE_ERRORV("Plugin '%s' is already loaded (V=%s).", PluginIter->second.m_pInfo->m_pPluginName, PluginIter->second.m_pInfo->m_pPluginVersion);

            return PluginIter->second.m_pInfo;
        }

        ENGINE_CONSOLE_INFOV("Plugin name:        %s"   , pPluginInfo->m_pPluginName);
        ENGINE_CONSOLE_INFOV("Plugin version:     %s"   , pPluginInfo->m_pPluginVersion);
        ENGINE_CONSOLE_INFOV("Plugin description: %s"   , pPluginInfo->m_pPluginDescription);
        ENGINE_CONSOLE_INFOV("Plugin API:         %i.%i", pPluginInfo->m_APIMajorVersion, pPluginInfo->m_APIMinorVersion);

        if (pPluginInfo->m_APIMajorVersion < ENGINE_MAJOR_VERSION || (pPluginInfo->m_APIMajorVersion == ENGINE_MAJOR_VERSION && pPluginInfo->m_APIMinorVersion < ENGINE_MINOR_VERSION))
        {
            ENGINE_CONSOLE_ERRORV("Plugin '%s' is out-dated (Current API version is %i.%i).", PluginIter->second.m_pInfo->m_pPluginName, ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION);

            return nullptr;
        }

        // -----------------------------------------------------------------------------
        // Save plugin
        // -----------------------------------------------------------------------------
        m_Plugins[_rLibrary].m_pInfo    = pPluginInfo;
        m_Plugins[_rLibrary].m_Instance = Instance;

        return m_Plugins[_rLibrary].m_pInfo;
    }

    // -----------------------------------------------------------------------------

    bool CPluginManager::HasPlugin(const std::string& _rName)
    {
        return m_Plugins.find(_rName) != m_Plugins.end();
    }

    // -----------------------------------------------------------------------------

    IPlugin* CPluginManager::GetPlugin(const std::string& _rName)
    {
        auto PluginIter = m_Plugins.find(_rName);

        if (PluginIter == m_Plugins.end())
        {
			auto Plugin = LoadPlugin(_rName);
			if (Plugin == nullptr)
			{
				ENGINE_CONSOLE_ERRORV("Plugin '%s' is not registered.", _rName.c_str());

				return nullptr;
			}
			else
			{
				return &Plugin->GetInstance();
			}
        }
		else
		{
			return &PluginIter->second.m_pInfo->GetInstance();
		}
    }

    // -----------------------------------------------------------------------------

    void* CPluginManager::GetPluginFunction(const std::string& _rName, const std::string& _rFunction)
    {
        auto PluginIter = m_Plugins.find(_rName);

        if (PluginIter == m_Plugins.end()) return nullptr;

        void* pMethod = nullptr;

#ifdef PLATFORM_WINDOWS
        pMethod = (void*)GetProcAddress(m_Plugins[_rName].m_Instance, _rFunction.c_str());
#elif PLATFORM_ANDROID
        pMethod = (void*)dlsym(m_Plugins[Hash].m_Instance, _rFunction.c_str());
#endif // PLATFORM_WINDOWS

        return pMethod;
    }

} // namespace

namespace Core
{
namespace PluginManager
{
	void Start()
	{
		CPluginManager::GetInstance().Start();
	}

	// -----------------------------------------------------------------------------

	void Update()
	{
		CPluginManager::GetInstance().Update();
	}

	// -----------------------------------------------------------------------------

	void Pause()
	{
		CPluginManager::GetInstance().Pause();
	}

	// -----------------------------------------------------------------------------

	void Resume()
	{
		CPluginManager::GetInstance().Resume();
	}

	// -----------------------------------------------------------------------------

	void Exit()
	{
		CPluginManager::GetInstance().Exit();
	}

	// -----------------------------------------------------------------------------

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