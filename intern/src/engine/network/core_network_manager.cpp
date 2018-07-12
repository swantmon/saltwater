
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
        for (auto Iterator = m_Sockets.begin(); Iterator != m_Sockets.end(); ++ Iterator)
        {
            Iterator->second->Update();
        }
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

    bool CNetworkManager::IsConnected(int _Port /* = 0 */) const
    {
        if (_Port == 0)
        {
            _Port = m_DefaultPort;
        }

        if (m_Sockets.count(_Port) == 0)
        {
            if (m_Sockets.at(_Port)->IsOpen())
            {
                return true;
            }
        }

        return false;
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

    void CNetworkManager::RegisterMessageHandler(int _MessageCategory, const std::shared_ptr<CMessageDelegate>& _rDelegate, int _Port)
    {
        int Port = _Port == 0 ? m_DefaultPort : _Port;

        CServerSocket& rSocket = GetSocket(Port);
        rSocket.RegisterMessageHandler(_MessageCategory, _rDelegate);
    }
    
    // -----------------------------------------------------------------------------

    bool CNetworkManager::SendMessage(int _MessageCategory, const std::vector<char>& _rData, int _Length, int _Port)
    {
        if (_Port == 0)
        {
            _Port = m_DefaultPort;
        }

        return m_Sockets[_Port]->SendMessage(_MessageCategory, _rData, _Length);
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