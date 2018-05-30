
#include "engine/engine_precompiled.h"

#include "base/base_exception.h"
#include "engine/network/core_network_manager.h"
#include "engine/network/core_network_socket.h"

#include <functional>
#include <iostream>

namespace Net
{
    void CServerSocket::ReceiveHeader(const std::error_code& error, size_t bytes_transferred)
    {
        std::cout << "Received message\n";

        int32_t MessageLength = *reinterpret_cast<int32_t*>(m_Header.data());

        m_Payload.resize(MessageLength);

        auto Callback = std::bind(&CServerSocket::ReceivePayload, this, std::placeholders::_1, std::placeholders::_2);

        asio::async_read(*m_pSocket, asio::buffer(m_Payload), asio::transfer_exactly(MessageLength), Callback);
    }

    // -----------------------------------------------------------------------------

    void CServerSocket::ReceivePayload(const std::error_code& error, size_t bytes_transferred)
    {
        std::cout << "Received message\n";

        StartListening();
    }

    // -----------------------------------------------------------------------------

    void CServerSocket::OnAccept(const std::system_error& _rError)
    {
        BASE_UNUSED(_rError);

        std::cout << "Accepted on port " << m_Port << '\n';
        
        StartListening();
    }

    // -----------------------------------------------------------------------------

    void CServerSocket::StartListening()
    {
        asio::async_read(*m_pSocket, asio::buffer(m_Payload), std::bind(&CServerSocket::ReceiveHeader, this, std::placeholders::_1, std::placeholders::_2));
    }

    // -----------------------------------------------------------------------------

    CServerSocket::CServerSocket(int _Port)
        : m_Port(_Port)
    {
        CNetworkManager::GetInstance().RegisterSocket(*this);

        auto& IOService = CNetworkManager::GetInstance().GetIOService();

        m_pEndpoint.reset(new asio::ip::tcp::endpoint(asio::ip::tcp::v4(), static_cast<unsigned short>(_Port)));
        m_pAcceptor.reset(new asio::ip::tcp::acceptor(IOService, *m_pEndpoint));
        m_pSocket.reset(new asio::ip::tcp::socket(IOService));

        m_Header.resize(8);

        m_pAcceptor->async_accept(*m_pSocket, *m_pEndpoint, std::bind(&CServerSocket::OnAccept, this, std::placeholders::_1));
    }

    // -----------------------------------------------------------------------------

    CServerSocket::~CServerSocket()
    {
        m_pSocket->close();
        CNetworkManager::GetInstance().UnregisterSocket(*this);
    }
} // namespace Net