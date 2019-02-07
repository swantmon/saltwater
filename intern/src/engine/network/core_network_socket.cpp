
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
    void CSocket::Update()
    {
        if (m_IsConnectionLost)
        {
            AsyncReconnect();
            m_IsConnectionLost = false;
        }

        m_Mutex.lock();

        while (!m_MessageQueue.empty())
        {
            auto& rMessage = m_MessageQueue.front();
            
            CMessageDelegate::Notify(rMessage, m_Port);

            m_MessageQueue.pop();
        }

        m_Mutex.unlock();

        if (!m_IsSending && !m_OutgoingMessages.empty())
        {
            InternalSendMessage();
        }
    }

    // -----------------------------------------------------------------------------

    CSocket::CMessageDelegate::HandleType CSocket::RegisterMessageHandler(CMessageDelegate::FunctionType _Function)
    {
        return CMessageDelegate::Register(_Function);
    }

    // -----------------------------------------------------------------------------

    void CSocket::OnSendComplete(std::shared_ptr<std::vector<char>> _Data)
    {
        BASE_UNUSED(_Data);

        CMessage Message;
        Message.m_MessageType = 1;
        
        m_Mutex.lock();

        m_MessageQueue.push(std::move(Message));

        m_Mutex.unlock();

        m_IsSending = false;
    }

    // -----------------------------------------------------------------------------
    
    bool CSocket::SendMessage(const CMessage& _rMessage)
    {
        if (IsOpen())
        {
            m_OutgoingMessages.emplace_back(_rMessage);

            return true;
        }
        else
        {
            return false;
        }
    }

    // -----------------------------------------------------------------------------

    void CSocket::InternalSendMessage()
    {
        if (!m_IsSending)
        {
            m_IsSending = true;

            const CMessage& Message = m_OutgoingMessages.front();

            int MessageLength = Message.m_CompressedSize;
            const auto& Data = Message.m_Payload;
            const int MessageCategory = Message.m_Category;

            if (MessageLength == 0)
            {
                MessageLength = static_cast<int>(Data.size());
            }

            int DataLength = MessageLength + 3 * sizeof(int32_t);

            std::shared_ptr<std::vector<char>> pData = std::make_shared<std::vector<char>>();

            pData->resize(DataLength);

            int32_t MessageID32 = static_cast<int32_t>(MessageCategory);
            int32_t MessageLength32 = static_cast<int32_t>(MessageLength);

            std::memcpy(pData->data(), &MessageID32, sizeof(MessageID32));
            std::memcpy(pData->data() + sizeof(int32_t), &MessageLength32, sizeof(MessageLength32));
            std::memcpy(pData->data() + 2 * sizeof(int32_t), &MessageLength32, sizeof(MessageLength32)); // TODO: Add compression
            std::memcpy(pData->data() + 3 * sizeof(int32_t), Data.data(), MessageLength32);

            asio::async_write(*m_pSocket, asio::buffer(*pData, DataLength), std::bind(&CSocket::OnSendComplete, this, pData));

            m_OutgoingMessages.pop_front();
        }
    };

    // -----------------------------------------------------------------------------

    void CSocket::ReceiveHeader(const std::error_code& _rError, size_t _TransferredBytes)
    {
        BASE_UNUSED(_TransferredBytes);
        
        if (!_rError)
        {
            assert(_TransferredBytes == m_Header.size());
            assert(_TransferredBytes == s_HeaderSize);

            int32_t MessageID = *reinterpret_cast<int32_t*>(m_Header.data());
            int32_t CompressedMessageLength = *reinterpret_cast<int32_t*>(m_Header.data() + sizeof(int32_t));
            int32_t DecompressedMessageLength = *reinterpret_cast<int32_t*>(m_Header.data() + 2 * sizeof(int32_t));
            m_Payload.resize(CompressedMessageLength);

            auto Callback = std::bind(&CSocket::ReceivePayload, this, std::placeholders::_1, std::placeholders::_2);
            asio::async_read(*m_pSocket, asio::buffer(m_Payload), asio::transfer_exactly(CompressedMessageLength), Callback);

            m_PendingMessage.m_Category = MessageID;
            m_PendingMessage.m_MessageType = 0;
            m_PendingMessage.m_CompressedSize = CompressedMessageLength;
            m_PendingMessage.m_DecompressedSize = DecompressedMessageLength;
        }
        else
        {
            m_IsConnectionLost = true;
        }
    }

    // -----------------------------------------------------------------------------

    void CSocket::ReceivePayload(const std::error_code& _rError, size_t _TransferredBytes)
    {
        BASE_UNUSED(_TransferredBytes);
        
        m_PendingMessage.m_Payload = std::move(m_Payload);

        m_Mutex.lock();

        m_MessageQueue.push(std::move(m_PendingMessage));

        m_Mutex.unlock();

        if (!_rError)
        {
            StartListening();
        }
        else
        {
            m_IsConnectionLost = true;
        }
    }

    // -----------------------------------------------------------------------------

    void CSocket::OnConnect(const std::system_error& _rError)
    {
        if (!_rError.code())
        {
            m_IsOpen = true;
            ENGINE_CONSOLE_INFOV("Connected on port %i", m_Port);
            StartListening();
        }
        else
        {
            ENGINE_CONSOLE_INFO(_rError.what());
            Connect();
        }
    }
    
    // -----------------------------------------------------------------------------

    void CSocket::StartListening()
    {
        auto Callback = std::bind(&CSocket::ReceiveHeader, this, std::placeholders::_1, std::placeholders::_2);
        asio::async_read(*m_pSocket, asio::buffer(m_Header), asio::transfer_exactly(s_HeaderSize), Callback);
    }

    // -----------------------------------------------------------------------------

    void CSocket::Connect()
    {
        auto& IOService = CNetworkManager::GetInstance().GetIOService();

        try
        {
            if (m_IsServer)
            {
                m_pEndpoint.reset(new asio::ip::tcp::endpoint(asio::ip::tcp::v4(), static_cast<unsigned short>(m_Port)));
                m_pAcceptor.reset(new asio::ip::tcp::acceptor(IOService, *m_pEndpoint));
                m_pSocket.reset(new asio::ip::tcp::socket(IOService));

                m_Header.resize(s_HeaderSize);

                m_pAcceptor->async_accept(*m_pSocket, *m_pEndpoint, std::bind(&CSocket::OnConnect, this, std::placeholders::_1));
            }
            else
            {
                asio::ip::address address = asio::ip::address::from_string(m_IP);

                m_pEndpoint.reset(new asio::ip::tcp::endpoint(address, static_cast<unsigned short>(m_Port)));
                m_pSocket.reset(new asio::ip::tcp::socket(IOService));

                m_Header.resize(s_HeaderSize);

                m_pSocket->async_connect(*m_pEndpoint, std::bind(&CSocket::OnConnect, this, std::placeholders::_1));
            }
        }
        catch (const std::exception& e)
        {
            throw Base::CException(__FILE__, __LINE__, e.what());
        }
    }

    // -----------------------------------------------------------------------------

    void CSocket::AsyncReconnect()
    {
        m_OutgoingMessages.clear();

        // Notify listener that the connection was lost
        CMessage Message;
        Message.m_Category = 0;
        Message.m_MessageType = 2;
        Message.m_CompressedSize = 0;
        Message.m_DecompressedSize = 0;

        m_Mutex.lock();

        m_MessageQueue.push(std::move(Message));

        m_Mutex.unlock();

        // Try to reconnect
        m_IsOpen = false;
        m_IsSending = false;
        m_pSocket->close();
        ENGINE_CONSOLE_INFOV("Connection lost on port %i", m_Port);

        Connect();
    }

    // -----------------------------------------------------------------------------

    bool CSocket::IsOpen() const
    {
        return m_IsOpen;
    }

    // -----------------------------------------------------------------------------

    bool CSocket::IsServer() const
    {
        return m_IsServer;
    }

    // -----------------------------------------------------------------------------

    int CSocket::GetPort() const
    {
        return m_Port;
    }

    // -----------------------------------------------------------------------------

    const std::string& CSocket::GetIP() const
    {
        return m_IP;
    }

    // -----------------------------------------------------------------------------

    CSocket::CSocket(int _Port)
        : m_Port(_Port)
        , m_IsOpen(false)
        , m_IsSending(false)
        , m_IsConnectionLost(false)
        , m_IsServer(true)
    {
        Connect();
    }

    CSocket::CSocket(const std::string& _IP, int _Port)
        : m_Port(_Port)
        , m_IP(_IP)
        , m_IsOpen(false)
        , m_IsSending(false)
        , m_IsConnectionLost(false)
        , m_IsServer(false)
    {
        Connect();
    }

    // -----------------------------------------------------------------------------

    CSocket::~CSocket()
    {
        m_pSocket->close();
    }
} // namespace Net