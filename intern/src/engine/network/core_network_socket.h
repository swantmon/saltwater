
#pragma once

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
    class ENGINE_API CServerSocket
    {
    private:

        void StartListening();
        void ReceiveHeader(const std::error_code& _rError, size_t _TransferredBytes);
        void ReceivePayload(const std::error_code& _rError, size_t _TransferredBytes);

        void AsyncReconnect();

        bool IsOpen() const;

        void Update();

        void RegisterMessageHandler(int _MessageCategory, const std::shared_ptr<CMessageDelegate>& _rpDelegate);
        bool SendMessage(int _MessageCategory, const std::vector<char>& _rData, int _Length = 0);

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

        std::multimap<int, std::weak_ptr<CMessageDelegate>> m_Delegates;

        std::mutex m_Mutex;

        std::queue<CMessage> m_MessageQueue;

    private:

        int s_HeaderSize = 12;

        struct OutgoingMessage
        {
            OutgoingMessage(int _MessageCategory, std::vector<char> _Payload, int _MessageLength)
                : m_MessageCategory(_MessageCategory)
                , m_PayLoad(_Payload)
                , m_MessageLength(_MessageLength)
            {

            }

            int m_MessageCategory;
            std::vector<char> m_PayLoad;
            int m_MessageLength;
        };

        void InternalSendMessage();

        std::deque<OutgoingMessage> m_OutgoingMessages;
        std::atomic<bool> m_IsSending;

        std::atomic<bool> m_IsConnectionLost;

        // shared_ptr cannot access the destructor so we use a custom deleter
        friend void SocketDeleter(Net::CServerSocket* _pSocket)
        {
            delete _pSocket;
        }

        CServerSocket(int _Port);
        ~CServerSocket();
    };
} // namespace Net