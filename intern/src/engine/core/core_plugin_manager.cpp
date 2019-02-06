
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
#include <regex>
#include <string>
#include <unordered_map>

#ifdef PLATFORM_WINDOWS
#include "windows.h"
#elif PLATFORM_ANDROID
#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
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

        void SetLibraryPath(const std::string& _rPath);

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

		bool InternGetPluginName(const std::string& _rFileName, std::string& _rPluginName);
		CInternPlugin::CInstance InternLoadLibrary(const std::string& _rFileName);
		void InternFreeLibrary(CInternPlugin::CInstance _Library);
		void* InternGetProc(CInternPlugin::CInstance _Library, const std::string& _rProcName);

    private:

		using CPluginFiles = std::unordered_map<std::string, std::string>;
        using CPlugins = std::unordered_map<std::string, CInternPlugin>;

    private:

		CPluginFiles m_PluginFiles;
        CPlugins m_Plugins;
        std::string m_LibraryPath;

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

	// -----------------------------------------------------------------------------

	void CPluginManager::InternFreeLibrary(CPluginManager::CInternPlugin::CInstance _Library)
	{
		FreeLibrary(_Library);
	}

	// -----------------------------------------------------------------------------

	void* CPluginManager::InternGetProc(CPluginManager::CInternPlugin::CInstance _Library, const std::string& _rProcName)
	{
		return GetProcAddress(_Library, _rProcName.c_str());
	}

	// -----------------------------------------------------------------------------

#elif PLATFORM_ANDROID

	CPluginManager::CInternPlugin::CInstance CPluginManager::InternLoadLibrary(const std::string& _rFileName)
	{
		return dlopen(_rFileName.c_str(), RTLD_NOW);
	}

	// -----------------------------------------------------------------------------

	void CPluginManager::InternFreeLibrary(CPluginManager::CInternPlugin::CInstance _Library)
	{
		dlclose(_Library);
	}

	// -----------------------------------------------------------------------------

	void* CPluginManager::InternGetProc(CPluginManager::CInternPlugin::CInstance _Library, const std::string& _rProcName)
	{
		return dlsym(_Library, _rProcName.c_str());
	}

#endif //PLATFORM_WINDOWS

	bool CPluginManager::InternGetPluginName(const std::string& _rFileName, std::string& _rPluginName)
	{
		auto Instance = InternLoadLibrary(_rFileName);

		if (Instance == nullptr)
		{
			return false;
		}

		_rPluginName = ((SPluginInfo*)InternGetProc(Instance, "InfoExport"))->m_pPluginName;

		InternFreeLibrary(Instance);

		return true;
	}

	// -----------------------------------------------------------------------------

	void CPluginManager::Start()
	{
		// Find all files in the current path

#ifdef PLATFORM_WINDOWS
		for (const auto & rEntry : std::filesystem::directory_iterator(m_LibraryPath))
		{
			// Find the ones that look like Saltwater plugins

			const auto PluginFileName = rEntry.path().string();
			if (std::regex_match(PluginFileName, m_PluginRegex))
			{
				// Found one! Now load it and get the plugin plugin's name

				std::string PluginName;
				if (InternGetPluginName(PluginFileName, PluginName))
				{
					m_PluginFiles[PluginName] = PluginFileName;
				}
			}
		}
#elif PLATFORM_ANDROID // TODO: Check if Android supports and std::filesystem and delete this branch
        DIR *d;
        struct dirent *dir;
        d = opendir(m_LibraryPath.c_str());
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                const auto PluginFileName = std::string(dir->d_name);
                if (std::regex_match(PluginFileName, m_PluginRegex))
                {
                    // Found one! Now load it and get the plugin plugin's name

                    std::string PluginName;
                    if (InternGetPluginName(PluginFileName, PluginName))
                    {
                        m_PluginFiles[PluginName] = PluginFileName;
                    }
                }
            }
            closedir(d);
        }
#endif
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
			InternFreeLibrary(Plugin.second.m_Instance);
        }

        m_Plugins.clear();
    }

    // -----------------------------------------------------------------------------

    void CPluginManager::SetLibraryPath(const std::string& _rPath)
    {
	    m_LibraryPath = _rPath;
    }

    // -----------------------------------------------------------------------------

    SPluginInfo* CPluginManager::LoadPlugin(const std::string& _rPluginName)
    {
		// -----------------------------------------------------------------------------
		// Find file name of plugin
		// -----------------------------------------------------------------------------
		const auto Iter = m_PluginFiles.find(_rPluginName);

		if (Iter == m_PluginFiles.end())
		{
			std::string Error = "Could not find plugin with name " + _rPluginName;
			throw Base::CException(__FILE__, __LINE__, Error.c_str());
		}
		
		const std::string& FileName = Iter->second;

		// -----------------------------------------------------------------------------
		// Check if plugin is already loaded.
		// -----------------------------------------------------------------------------
		auto PluginIter = m_Plugins.find(FileName);

		if (PluginIter != m_Plugins.end())
		{
			ENGINE_CONSOLE_ERRORV("Plugin '%s' is already loaded (V=%s).", PluginIter->second.m_pInfo->m_pPluginName, PluginIter->second.m_pInfo->m_pPluginVersion);

			return PluginIter->second.m_pInfo;
		}

		// -----------------------------------------------------------------------------
		// Load library and info
		// -----------------------------------------------------------------------------

		auto Instance = InternLoadLibrary(FileName);

        if (Instance == nullptr)
        {
#ifdef PLATFORM_WINDOWS
            ENGINE_CONSOLE_ERRORV("Plugin '%s' not found.", FileName.c_str());
#elif PLATFORM_ANDROID
            ENGINE_CONSOLE_ERRORV("Plugin '%s' not found (Error: '%s').", FileName.c_str(), dlerror());
#endif // PLATFORM_WINDOWS

            return nullptr;
        }

        ENGINE_CONSOLE_INFOV("Loading plugin '%s' successful.", FileName.c_str());
		
        auto pPluginInfo = (SPluginInfo*)InternGetProc(Instance, "InfoExport");

        if (pPluginInfo == nullptr)
        {
#ifdef PLATFORM_WINDOWS
            ENGINE_CONSOLE_ERRORV("Loading plugin information failed (Error: %i).", GetLastError());
#elif PLATFORM_ANDROID
            ENGINE_CONSOLE_ERRORV("Loading plugin information failed (Error: '%s').", dlerror());
#endif // PLATFORM_WINDOWS

			InternFreeLibrary(Instance);

            return nullptr;
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
        m_Plugins[_rPluginName].m_pInfo    = pPluginInfo;
        m_Plugins[_rPluginName].m_Instance = Instance;

        return m_Plugins[_rPluginName].m_pInfo;
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
	    auto NameIter = m_PluginFiles.find(_rName);
        auto PluginIter = m_Plugins.find(NameIter->second);

        if (PluginIter == m_Plugins.end()) return nullptr;

        return InternGetProc(PluginIter->second.m_Instance, _rFunction);
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

    void SetLibraryPath(const std::string& _rPath)
    {
        CPluginManager::GetInstance().SetLibraryPath(_rPath);
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