
#include "core/core_precompiled.h"

#include "base/base_console.h"
#include "base/base_crc.h"
#include "base/base_exception.h"

#include "core/core_plugin_manager.h"

namespace Core
{
    CPluginManager::CPluginManager()
    {

    }

    // -----------------------------------------------------------------------------

    CPluginManager::~CPluginManager()
    {
        m_Plugins.clear();
    }

    // -----------------------------------------------------------------------------

    void CPluginManager::RegisterPlugin(const SPluginInfo& _Plugin)
    {
        Base::BHash Hash = GenerateHash(_Plugin.m_pPluginName);

        auto PluginIter = m_Plugins.find(Hash);

        if (PluginIter != m_Plugins.end())
        {
            BASE_CONSOLE_ERRORV("The plugin '%s' is already registered (V=%s).", _Plugin.m_pPluginName, _Plugin.m_pPluginVersion);

            return;
        }

        BASE_CONSOLE_INFOV("Plugin '%s' with version '%s' successfully registered.", _Plugin.m_pPluginName, _Plugin.m_pPluginVersion);

        m_Plugins[Hash] = _Plugin;
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

        return PluginIter->second.GetInstance();
    }

    // -----------------------------------------------------------------------------

    Base::BHash CPluginManager::GenerateHash(const char* _pValue)
    {
        return Base::CRC32(_pValue, sizeof(char) * static_cast<unsigned int>(strlen(_pValue)));
    }
} // namespace Core