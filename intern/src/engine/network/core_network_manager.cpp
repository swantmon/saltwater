
#include "engine/engine_precompiled.h"

#include "base/base_exception.h"

#include "engine/network/core_network_manager.h"

#include <functional>
#include <iostream>

namespace Net
{
    CNetworkManager& CNetworkManager::GetInstance()
    {
        static CNetworkManager s_Sinstance;

        return s_Sinstance;
    }
} // namespace Net

namespace Net
{
    void CNetworkManager::OnStart()
    {

    }

    // -----------------------------------------------------------------------------

    void CNetworkManager::OnUpdate()
    {

    }

    // -----------------------------------------------------------------------------

    void CNetworkManager::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    CNetworkManager::CNetworkManager()
    {

    }

    // -----------------------------------------------------------------------------

    CNetworkManager::~CNetworkManager()
    {

    }
} // namespace Net