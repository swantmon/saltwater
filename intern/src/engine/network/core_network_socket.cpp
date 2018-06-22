
#include "engine/engine_precompiled.h"

#include "base/base_exception.h"

#include "engine/core/core_console.h"
#include "engine/network/core_network_manager.h"
#include "engine/network/core_network_socket.h"

#include <functional>
#include <iostream>
#include <memory>

namespace Net
{
    void CServerSocket::Update()
    {
        m_Mutex.lock();

        while (!m_MessageQueue.empty())
        {
            auto& rMessage = m_MessageQueue.front();

            int ID = rMessage.m_ID;

            auto Range = m_Delegates.equal_range(ID);
            for (auto Iterator = Range.first; Iterator != Range.second; )
            {
                auto Delegate = Iterator->second.lock();
                if (Delegate == nullptr)
                {
                    Iterator = m_Delegates.erase(Iterator);
                }
                else
                {
                    (*Delegate)(ID, rMessage.m_Payload, m_Port);
                    ++ Iterator;
                }
            }

            m_MessageQueue.pop();
        }

        m_Mutex.unlock();
    }

    // -----------------------------------------------------------------------------

    void CServerSocket::RegisterMessageHandler(int _MessageID, const std::shared_ptr<CMessageDelegate>& _rDelegate)
    {
        m_Delegates.insert(std::make_pair(_MessageID, _rDelegate));
    }

    // -----------------------------------------------------------------------------

    void CServerSocket::OnSendComplete(std::shared_ptr<std::vector<char>> _Data)
    {
        ENGINE_CONSOLE_INFOV("Send complete");
    }

    // -----------------------------------------------------------------------------
    
    bool CServerSocket::SendMessage(int _MessageID, const std::vector<char>& _rData, int _MessageLength)
    {
        if (IsOpen())
        {
            if (_MessageLength == 0)
            {
                _MessageLength = static_cast<int>(_rData.size());
            }

            int DataLength = _MessageLength + sizeof(int32_t);

            std::shared_ptr<std::vector<char>> pData = std::make_shared<std::vector<char>>();

            pData->resize(DataLength);

            int32_t MessageID = static_cast<int32_t>(_MessageID);
            std::memcpy(pData->data(), &MessageID, sizeof(MessageID));
            std::memcpy(pData->data() + sizeof(int32_t), _rData.data(), _MessageLength);

            m_pSocket->async_send(asio::buffer(*pData, DataLength), std::bind(&CServerSocket::OnSendComplete, this, pData));

            return true;
        }
        else
        {
            return false;
        }
    }

    // -----------------------------------------------------------------------------

    void CServerSocket::ReceiveHeader(const std::error_code& _rError, size_t _TransferredBytes)
    {
        BASE_UNUSED(_TransferredBytes);

        if (!_rError)
        {
            assert(_TransferredBytes == m_Header.size());
            assert(_TransferredBytes == s_HeaderSize);

            int32_t MessageLength = *reinterpret_cast<int32_t*>(m_Header.data());
            m_Payload.resize(MessageLength);

            auto Callback = std::bind(&CServerSocket::ReceivePayload, this, std::placeholders::_1, std::placeholders::_2);
            asio::async_read(*m_pSocket, asio::buffer(m_Payload), asio::transfer_exactly(MessageLength), Callback);

            CMessage Message;
            Message.m_ID = 0;
            Message.m_Payload = m_Payload;

            m_Mutex.lock();

            m_MessageQueue.push(std::move(Message));

            m_Mutex.unlock();
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
            m_IsOpen = false;
            AsyncReconnect();
        }
    }

    // -----------------------------------------------------------------------------

    void CServerSocket::OnAccept(const std::system_error& _rError)
    {
        m_IsOpen = true;
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

    bool CServerSocket::IsOpen() const
    {
        return m_IsOpen;
    }

    // -----------------------------------------------------------------------------

    CServerSocket::CServerSocket(int _Port)
        : m_Port(_Port)
        , m_IsOpen(false)
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