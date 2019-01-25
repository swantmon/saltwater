
#include "engine/engine_precompiled.h"

#include "base/base_exception.h"
#include "engine/core/core_program_parameters.h"
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

    bool CNetworkManager::IsConnected(int _Port) const
    {
        if (m_Sockets.count(_Port) != 0)
        {
            if (m_Sockets.at(_Port)->IsOpen())
            {
                return true;
            }
        }

        return false;
    }

    // -----------------------------------------------------------------------------

    int CNetworkManager::CreateServerSocket(int _Port)
    {
        if (m_Sockets.count(_Port) == 0)
        {
            m_Sockets[_Port].reset(new CServer(_Port));
        }

        return _Port;
    }

    // -----------------------------------------------------------------------------

    int CNetworkManager::CreateClientSocket(const std::string& _IP, int _Port)
    {
        if (m_Sockets.count(_Port) == 0)
        {
            m_Sockets[_Port].reset(new CServer(_IP, _Port));
        }

        return _Port;
    }

    // -----------------------------------------------------------------------------

    void CNetworkManager::RegisterMessageHandler(int _SocketHandle, const std::shared_ptr<CMessageDelegate>& _rDelegate)
    {
        if (m_Sockets.count(_SocketHandle) == 0)
        {
            throw Base::CException(__FILE__, __LINE__, "Failed to register message handler. No appropriate socket found.");
        }

        m_Sockets[_SocketHandle]->RegisterMessageHandler(_rDelegate);
    }
    
    // -----------------------------------------------------------------------------

    bool CNetworkManager::SendMessage(int _Port, const CMessage& _rMessage)
    {
        if (m_Sockets.count(_Port) == 0)
        {
            throw Base::CException(__FILE__, __LINE__, "Failed to register message handler. No appropriate socket found.");
        }

        return m_Sockets[_Port]->SendMessage(_rMessage);
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

    // -----------------------------------------------------------------------------

    CNetworkManager& CNetworkManager::GetInstance()
    {
        static CNetworkManager s_Instance;
        return s_Instance;
    }
} // namespace Net