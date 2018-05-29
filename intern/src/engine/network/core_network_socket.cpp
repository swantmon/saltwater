
#include "engine/engine_precompiled.h"

#include "base/base_exception.h"
#include "engine/network/core_network_manager.h"
#include "engine/network/core_network_socket.h"

#include <functional>
#include <iostream>

namespace Net
{
    void CServerSocket::OnAccept(const std::system_error& _rError)
    {
        BASE_UNUSED(_rError);

        std::cout << "Accepted on port " << m_Port << '\n';
    }

    CServerSocket::CServerSocket(int _Port)
        : m_Port(_Port)
    {
        CNetworkManager::GetInstance().RegisterSocket(*this);

        auto& IOService = CNetworkManager::GetInstance().GetIOService();

        m_pEndpoint.reset(new asio::ip::tcp::endpoint(asio::ip::tcp::v4(), static_cast<unsigned short>(_Port)));
        m_pAcceptor.reset(new asio::ip::tcp::acceptor(IOService, *m_pEndpoint));
        m_pSocket.reset(new asio::ip::tcp::socket(IOService));

        m_pAcceptor->async_accept(*m_pSocket, *m_pEndpoint, std::bind(&CServerSocket::OnAccept, this, std::placeholders::_1));
    }

    // -----------------------------------------------------------------------------

    CServerSocket::~CServerSocket()
    {
        CNetworkManager::GetInstance().UnregisterSocket(*this);
    }
} // namespace Net