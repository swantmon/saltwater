
#pragma once

#include "engine/engine_precompiled.h"

#include "engine/engine_config.h"

#include <memory>

namespace Net
{
    class ENGINE_API CServerSocket
    {

    public:

        // dummies
        void RegisterDelegate();
        void UnregisterDelegate();
        void SendMessage();

    private:

        friend class CNetworkManager;

        void OnAccept(const std::system_error& _rError);

        int m_Port;
        
        std::unique_ptr<asio::ip::tcp::endpoint> m_pEndpoint;
        std::unique_ptr<asio::ip::tcp::acceptor> m_pAcceptor;
        std::unique_ptr<asio::ip::tcp::socket> m_pSocket;

    private:

        // shared_ptr cannot access the destructor so we use a custom deleter
        friend void SocketDeleter(Net::CServerSocket* _pSocket)
        {
            delete _pSocket;
        }

        CServerSocket(int _Port);
        ~CServerSocket();
    };
} // namespace Net