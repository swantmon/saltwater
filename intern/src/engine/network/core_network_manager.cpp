
#include "engine/engine_precompiled.h"

#include "base/base_exception.h"

#include "engine/network/core_network_manager.h"

#include <algorithm>

namespace Net
{
    void CNetworkManager::OnStart()
    {

    }

    // -----------------------------------------------------------------------------

    void CNetworkManager::Update()
    {

    }

    // -----------------------------------------------------------------------------

    void CNetworkManager::OnExit()
    {

    }

    // -----------------------------------------------------------------------------

    void CNetworkManager::RegisterSocket(const CServerSocket& _rSocket)
    {
        m_Sockets.push_back(&_rSocket);
    }

    // -----------------------------------------------------------------------------

    void CNetworkManager::UnregisterSocket(const CServerSocket& _rSocket)
    {
        m_Sockets.erase(std::remove(m_Sockets.begin(), m_Sockets.end(), &_rSocket));
    }

    // -----------------------------------------------------------------------------

    asio::io_service& CNetworkManager::GetIOService()
    {
        return m_IOService;
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