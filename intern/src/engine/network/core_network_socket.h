
#pragma once

#include "engine/engine_precompiled.h"

#include "engine/engine_config.h"

#include <memory>

namespace Net
{
    class ENGINE_API CServerSocket
    {

    public:
        
        CServerSocket(int _Port);
        ~CServerSocket();

    public:



    private:

        int m_Port;
        
        std::unique_ptr<asio::ip::tcp::endpoint> m_pEndpoint;
        std::unique_ptr<asio::ip::tcp::acceptor> m_pAcceptor;
        std::unique_ptr<asio::ip::tcp::socket> m_pSocket;
    };
} // namespace Net