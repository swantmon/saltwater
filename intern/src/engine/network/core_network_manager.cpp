
#include "engine/engine_precompiled.h"

#include "base/base_exception.h"
#include "engine/core/core_program_parameters.h"
#include "engine/network/core_network_manager.h"

#include <algorithm>

namespace Net
{
    void CNetworkManager::OnStart()
    {
        m_DefaultPort = Core::CProgramParameters::GetInstance().Get("network:default_port", 12345);
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
        /*for (auto& pSocket : m_Sockets)
        {
            pSocket->CallDelegates();
        }*/
    }

    // -----------------------------------------------------------------------------

    void CNetworkManager::OnExit()
    {
        m_IsRunning = false;
        m_IOService.stop();
        m_WorkerThread.join();

        m_Sockets.clear();
    }

    // -----------------------------------------------------------------------------

    CServerSocket& CNetworkManager::GetSocket(int _Port)
    {
        if (m_Sockets.count(_Port) == 0)
        {
            m_Sockets[_Port].reset(new CServerSocket(_Port));
        }

        return *m_Sockets[_Port];
    }
    
    // -----------------------------------------------------------------------------

    void CNetworkManager::RegisterMessageHandler(int _MessageID, CNetworkMessageDelegate, int _Port)
    {
        int Port = _Port == 0 ? m_DefaultPort : _Port;

        CServerSocket& rSocket = GetSocket(Port);
    }

    // -----------------------------------------------------------------------------

    void CNetworkManager::UnregisterMessageHandler(int _MessageID, CNetworkMessageDelegate, int _Port)
    {
        int Port = _Port == 0 ? m_DefaultPort : _Port;

        CServerSocket& rSocket = GetSocket(Port);
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