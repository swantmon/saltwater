
#include "engine/engine_precompiled.h"

#include "base/base_exception.h"

#include "engine/network/core_network_manager.h"

#include <algorithm>

namespace Net
{
    void CNetworkManager::OnStart()
    {
        m_IsRunning = true;
        m_WorkerThread = std::thread(std::bind(&CNetworkManager::Run, this));
    }

    // -----------------------------------------------------------------------------

    void CNetworkManager::Run()
    {
        while (m_IsRunning)
        {
            m_IOService.run();
            m_IOService.reset();
        }
    }

    // -----------------------------------------------------------------------------

    void CNetworkManager::Update()
    {
        for (auto& pSocket : m_Sockets)
        {
            pSocket->CallDelegates();
        }
    }

    // -----------------------------------------------------------------------------

    void CNetworkManager::OnExit()
    {
        m_IsRunning = false;
        m_IOService.stop();
        m_WorkerThread.join();
    }

    // -----------------------------------------------------------------------------

    CServerSocketPtr CNetworkManager::CreateServerSocket(int _Port)
    {
        std::shared_ptr<CServerSocket> pSocket(new CServerSocket(_Port), SocketDeleter);

        return pSocket;
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