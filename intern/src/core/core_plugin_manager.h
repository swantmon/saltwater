#pragma once

#include "base/base_typedef.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "core/core_plugin.h"

#include <map>

namespace Core
{
    class CPluginManager : public Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CPluginManager)

    public:

        void RegisterPlugin(const SPluginInfo& _Plugin);

        bool HasPlugin(const char* _pName);

        IPlugin& GetPlugin(const char* _pName);

    private:

        typedef std::map<Base::BHash, SPluginInfo> CPlugins;

    private:

        CPlugins m_Plugins;

    private:

        CPluginManager();
        ~CPluginManager();

        Base::BHash GenerateHash(const char* _pValue);
    };
} // namespace Core