
#include "engine/engine_precompiled.h"

#include "base/base_exception.h"

#include "engine/core/core_console.h"
#include "engine/network/core_network_manager.h"
#include "engine/network/core_network_socket.h"

#include <functional>
#include <iostream>

namespace Net
{
    void CServerSocket::CallDelegates() const
    {

    }

    void CServerSocket::ReceiveHeader(const std::error_code& _rError, size_t _TransferredBytes)
    {
        BASE_UNUSED(_TransferredBytes);

        if (!_rError)
        {
            int32_t MessageLength = *reinterpret_cast<int32_t*>(m_Header.data());
            m_Payload.resize(MessageLength);

            auto Callback = std::bind(&CServerSocket::ReceivePayload, this, std::placeholders::_1, std::placeholders::_2);
            asio::async_read(*m_pSocket, asio::buffer(m_Payload), asio::transfer_exactly(MessageLength), Callback);
        }
        else
        {
            AsyncReconnect();
        }
    }

    // -----------------------------------------------------------------------------

    void CServerSocket::ReceivePayload(const std::error_code& _rError, size_t _TransferredBytes)
    {
        BASE_UNUSED(_TransferredBytes);

        if (!_rError)
        {
            StartListening();
        }
        else
        {
            AsyncReconnect();
        }
    }

    // -----------------------------------------------------------------------------

    void CServerSocket::OnAccept(const std::system_error& _rError)
    {
        BASE_UNUSED(_rError);                
        ENGINE_CONSOLE_INFOV("Connected on port %i", m_Port);
        StartListening();
    }

    // -----------------------------------------------------------------------------

    void CServerSocket::StartListening()
    {
        auto Callback = std::bind(&CServerSocket::ReceiveHeader, this, std::placeholders::_1, std::placeholders::_2);
        asio::async_read(*m_pSocket, asio::buffer(m_Header), asio::transfer_exactly(s_HeaderSize), Callback);
    }

    // -----------------------------------------------------------------------------

    void CServerSocket::AsyncReconnect()
    {
        m_pSocket->close();
        ENGINE_CONSOLE_INFOV("Connection lost on port %i", m_Port);
        m_pAcceptor->async_accept(*m_pSocket, *m_pEndpoint, std::bind(&CServerSocket::OnAccept, this, std::placeholders::_1));
    }

    // -----------------------------------------------------------------------------

    CServerSocket::CServerSocket(int _Port)
        : m_Port(_Port)
    {
        auto& IOService = CNetworkManager::GetInstance().GetIOService();

        m_pEndpoint.reset(new asio::ip::tcp::endpoint(asio::ip::tcp::v4(), static_cast<unsigned short>(_Port)));
        m_pAcceptor.reset(new asio::ip::tcp::acceptor(IOService, *m_pEndpoint));
        m_pSocket.reset(new asio::ip::tcp::socket(IOService));

        m_Header.resize(s_HeaderSize);

        m_pAcceptor->async_accept(*m_pSocket, *m_pEndpoint, std::bind(&CServerSocket::OnAccept, this, std::placeholders::_1));
    }

    // -----------------------------------------------------------------------------

    CServerSocket::~CServerSocket()
    {
        m_pSocket->close();
    }
} // namespace Net