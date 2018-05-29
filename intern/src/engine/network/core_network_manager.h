
#pragma once

#include "engine/engine_config.h"

#include "engine/network/core_network_socket.h"

#include <string>

namespace Net
{
    class ENGINE_API CNetworkManager
    {
    public:

        static CNetworkManager& GetInstance();

    public:

        void OnStart();
        void OnUpdate();
        void OnExit();
        
    private:
        
        CNetworkManager();
        ~CNetworkManager();

    private:

        asio::io_service m_IOService;
    };
} // namespace Core