
#include "engine/engine_precompiled.h"

#include "engine/engine_config.h"

#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_typedef.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "engine/core/core_console.h"
#include "engine/core/core_plugin_manager.h"
#include "engine/core/core_program_parameters.h"

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
using namespace std::string_literals;

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

        SPluginInfo* InternLoadPlugin(const std::string& _rLibrary);
		
        bool LoadPlugin(const std::string& _rName);

        void* GetPluginFunction(const std::string& _rName, const std::string& _rMethod);

    private:

        struct SInternPlugin
        {
#ifdef PLATFORM_WINDOWS
            typedef HINSTANCE CInstance;
#elif PLATFORM_ANDROID
            typedef void* CInstance;
#endif // PLATFORM_WINDOWS

            CInstance m_Instance;
            SPluginInfo* m_pInfo;
			bool m_IsInitialized;
        };

	private:

		SInternPlugin::CInstance InternLoadLibrary(const std::string& _rFileName);
		void InternFreeLibrary(SInternPlugin::CInstance _Library);
		void* InternGetProc(SInternPlugin::CInstance _Library, const std::string& _rProcName);

    private:

        using CPlugins = std::unordered_map<std::string, SInternPlugin>;

    private:

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

	CPluginManager::SInternPlugin::CInstance CPluginManager::InternLoadLibrary(const std::string& _rFileName)
	{
		WCHAR FileName[32768];
		MultiByteToWideChar(CP_UTF8, 0, _rFileName.c_str(), -1, FileName, 32768);
		std::wstring PluginFile = std::wstring(FileName);
		return LoadLibraryExW(PluginFile.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	}

	// -----------------------------------------------------------------------------

	void CPluginManager::InternFreeLibrary(CPluginManager::SInternPlugin::CInstance _Library)
	{
		FreeLibrary(_Library);
	}

	// -----------------------------------------------------------------------------

	void* CPluginManager::InternGetProc(CPluginManager::SInternPlugin::CInstance _Library, const std::string& _rProcName)
	{
		return GetProcAddress(_Library, _rProcName.c_str());
	}

	// -----------------------------------------------------------------------------

#elif PLATFORM_ANDROID

	CPluginManager::SInternPlugin::CInstance CPluginManager::InternLoadLibrary(const std::string& _rFileName)
	{
		return dlopen(_rFileName.c_str(), RTLD_NOW);
	}

	// -----------------------------------------------------------------------------

	void CPluginManager::InternFreeLibrary(CPluginManager::SInternPlugin::CInstance _Library)
	{
		dlclose(_Library);
	}

	// -----------------------------------------------------------------------------

	void* CPluginManager::InternGetProc(CPluginManager::SInternPlugin::CInstance _Library, const std::string& _rProcName)
	{
		return dlsym(_Library, _rProcName.c_str());
	}

#endif //PLATFORM_WINDOWS

	// -----------------------------------------------------------------------------

	void CPluginManager::Start()
	{
		// Find all files in the current path

#ifdef PLATFORM_WINDOWS
		for (const auto & rEntry : std::filesystem::directory_iterator(m_LibraryPath))
		{
			// Find the ones that have a name like Saltwater plugins

			const auto PluginFileName = rEntry.path().string();
			if (std::regex_match(PluginFileName, m_PluginRegex))
			{
				// Found one! Now load it and get the plugin plugin's name
				// If we can't get the name it's not a valid SW plugin and we release it

				auto pLib = InternLoadLibrary(PluginFileName);
				if (pLib != nullptr)
				{
					auto pInfo = static_cast<SPluginInfo*>(InternGetProc(pLib, "InfoExport"));

					if (pInfo != nullptr)
					{
                        auto Message = "Found plugin \'"s + pInfo->m_pPluginName + "\' in file \'"s + PluginFileName + "\'"s;
                        ENGINE_CONSOLE_INFO(Message.c_str());
						m_Plugins[pInfo->m_pPluginName] = { pLib, pInfo, false };
					}
					else
					{
						InternFreeLibrary(pLib);
					}
				}
			}
		}
#elif PLATFORM_ANDROID // TODO: Check if Android supports std::filesystem and delete this branch
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

		auto SelectedPlugins = Core::CProgramParameters::GetInstance().Get("plugins:selection", std::vector<std::string>());

		for (auto SelectedPlugin : SelectedPlugins)
		{
			if (m_Plugins.find(SelectedPlugin) == m_Plugins.end())
			{
				throw Base::CException(__FILE__, __LINE__, ("Required plugin "s + " is not available"s).c_str());
			}
		}
	}

	// -----------------------------------------------------------------------------

	void CPluginManager::Update()
	{
		for (auto& [Key, Plugin] : m_Plugins)
		{
			if (Plugin.m_IsInitialized)
			{
				Plugin.m_pInfo->GetInstance().Update();
			}
		}
	}

	// -----------------------------------------------------------------------------

	void CPluginManager::Pause()
	{
		for (auto&[Key, Plugin] : m_Plugins)
		{
			if (Plugin.m_IsInitialized)
			{
				Plugin.m_pInfo->GetInstance().OnPause();
			}
		}
	}

	// -----------------------------------------------------------------------------

	void CPluginManager::Resume()
	{
		for (auto&[Key, Plugin] : m_Plugins)
		{
			if (Plugin.m_IsInitialized)
			{
				Plugin.m_pInfo->GetInstance().OnResume();
			}
		}
	}

	// -----------------------------------------------------------------------------

	void CPluginManager::Exit()
	{
		for (auto&[Key, Plugin] : m_Plugins)
		{
			if (Plugin.m_IsInitialized)
			{
				Plugin.m_pInfo->GetInstance().OnExit();
			}
		}
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
        for (auto& rPlugin : m_Plugins)
        {
			InternFreeLibrary(rPlugin.second.m_Instance);
        }

        m_Plugins.clear();
    }

    // -----------------------------------------------------------------------------

    void CPluginManager::SetLibraryPath(const std::string& _rPath)
    {
	    m_LibraryPath = _rPath;
    }

    // -----------------------------------------------------------------------------

    SPluginInfo* CPluginManager::InternLoadPlugin(const std::string& _rPluginName)
    {
		// -----------------------------------------------------------------------------
		// Find file name of plugin
		// -----------------------------------------------------------------------------
		const auto Iter = m_Plugins.find(_rPluginName);

		if (Iter == m_Plugins.end())
		{
			std::string Error = "Could not find plugin with name " + _rPluginName;
			throw Base::CException(__FILE__, __LINE__, Error.c_str());
		}
		
		// -----------------------------------------------------------------------------
		// Get library info
		// -----------------------------------------------------------------------------

		auto Instance = Iter->second.m_Instance;
		auto pPluginInfo = Iter->second.m_pInfo;

		assert(Instance != nullptr);
		assert(pPluginInfo != nullptr);
		assert(!Iter->second.m_IsInitialized);

		ENGINE_CONSOLE_INFOV("Loading plugin '%s' successful.", _rPluginName.c_str());
        ENGINE_CONSOLE_INFOV("Plugin name:        %s"   , pPluginInfo->m_pPluginName);
        ENGINE_CONSOLE_INFOV("Plugin version:     %s"   , pPluginInfo->m_pPluginVersion);
        ENGINE_CONSOLE_INFOV("Plugin description: %s"   , pPluginInfo->m_pPluginDescription);
        ENGINE_CONSOLE_INFOV("Plugin API:         %i.%i", pPluginInfo->m_APIMajorVersion, pPluginInfo->m_APIMinorVersion);

        if (pPluginInfo->m_APIMajorVersion < ENGINE_MAJOR_VERSION || (pPluginInfo->m_APIMajorVersion == ENGINE_MAJOR_VERSION && pPluginInfo->m_APIMinorVersion < ENGINE_MINOR_VERSION))
        {
            ENGINE_CONSOLE_ERRORV("Plugin '%s' is out-dated (Current API version is %i.%i).", _rPluginName.c_str(), ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION);

            return nullptr;
        }

		Iter->second.m_IsInitialized = true;

		pPluginInfo->GetInstance().OnStart();

        return m_Plugins[_rPluginName].m_pInfo;
    }

    // -----------------------------------------------------------------------------

    bool CPluginManager::LoadPlugin(const std::string& _rName)
    {
        auto PluginIter = m_Plugins.find(_rName);

        if (PluginIter == m_Plugins.end())
        {			
			std::string Info = "Plugin " + _rName + " is not available";
			ENGINE_CONSOLE_INFO(Info.c_str());
			return false;
        }
		else
		{
			if (!PluginIter->second.m_IsInitialized)
			{
				InternLoadPlugin(_rName);
			}
			return true;
		}
    }

    // -----------------------------------------------------------------------------

    void* CPluginManager::GetPluginFunction(const std::string& _rName, const std::string& _rFunction)
    {
        auto PluginIter = m_Plugins.find(_rName);

		if (PluginIter == m_Plugins.end())
		{
            auto Error = "Plugin "s + _rName + " not available"s;
            throw Base::CException(__FILE__, __LINE__, Error.c_str());
		}

        auto Proc = InternGetProc(PluginIter->second.m_Instance, _rFunction);

        if (Proc == nullptr)
        {
            auto Error = "Function "s + _rFunction + " not available in plugin "s + _rName;
            throw Base::CException(__FILE__, __LINE__, Error.c_str());
        }

        return Proc;
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

    void SetLibraryPath(const std::string& _rPath)
    {
        CPluginManager::GetInstance().SetLibraryPath(_rPath);
    }

    // -----------------------------------------------------------------------------

    bool LoadPlugin(const std::string& _rName)
    {
        return CPluginManager::GetInstance().LoadPlugin(_rName);
    }

    // -----------------------------------------------------------------------------

    void* GetPluginFunction(const std::string& _rName, const std::string& _rFunction)
    {
        return CPluginManager::GetInstance().GetPluginFunction(_rName, _rFunction);
    }
} // namespace PluginManager
} // namespace Core