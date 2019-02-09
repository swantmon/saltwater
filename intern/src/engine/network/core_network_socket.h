
#pragma once

#include "base/base_delegate.h"

#include "engine/engine_precompiled.h"

#include "engine/engine_config.h"

#include "engine/network/core_network_common.h"
#include "engine/network/core_network_manager.h"

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <queue>

namespace Net
{
    class ENGINE_API CSocket
    {
    private:

        using CMessageDelegate = Base::CDelegate<const CMessage&, SocketHandle>;

        void StartListening();
        void ReceiveHeader(const std::error_code& _rError, size_t _TransferredBytes);
        void ReceivePayload(const std::error_code& _rError, size_t _TransferredBytes);

        void AsyncReconnect();

        bool IsOpen() const;
        bool IsServer() const;

        int GetPort() const;
        const std::string& GetIP() const;

        void Update();

        CMessageDelegate::HandleType RegisterMessageHandler(CMessageDelegate::FunctionType _Function);
        bool SendMessage(const CMessage& _rMessage);

    private:

        friend class CNetworkManager;

        void Connect();

        void OnConnect(const std::system_error& _rError);
        void OnSendComplete(std::shared_ptr<std::vector<char>> _Data);

        int m_Port;
        
        bool m_IsOpen;

        std::unique_ptr<asio::ip::tcp::endpoint> m_pEndpoint;
        std::unique_ptr<asio::ip::tcp::acceptor> m_pAcceptor;
        std::unique_ptr<asio::ip::tcp::socket> m_pSocket;

        std::vector<char> m_Header;
        std::vector<char> m_Payload;

        CMessage m_PendingMessage;

        std::vector<std::weak_ptr<CMessageDelegate>> m_Delegates;

        std::mutex m_Mutex;

        std::queue<CMessage> m_MessageQueue;

        std::string m_IP;
        const bool m_IsServer;

    private:

        int s_HeaderSize = 12;

        void InternalSendMessage();

        std::deque<CMessage> m_OutgoingMessages;
        std::atomic<bool> m_IsSending;

        std::atomic<bool> m_IsConnectionLost;

        // shared_ptr cannot access the destructor so we use a custom deleter
        friend void SocketDeleter(Net::CSocket* _pSocket)
        {
            delete _pSocket;
        }

        CSocket(int _Port);
        CSocket(const std::string& _IP, int _Port);
        ~CSocket();
    };
} // namespace Net