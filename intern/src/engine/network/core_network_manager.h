
#pragma once

#include "engine/engine_config.h"

#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "engine/network/core_network_socket.h"

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

namespace Net
{
    typedef std::shared_ptr<Net::CServerSocket> CServerSocketPtr;

    class ENGINE_API CNetworkManager : private Base::CUncopyable
    {
    public:

        BASE_SINGLETON_FUNC(CNetworkManager)

    public:

        void OnStart();
        void Update();
        void OnExit();

        CServerSocketPtr CreateServerSocket(int _Port);

    private:
        
        void Run();

        friend class CServerSocket;

        void RegisterSocket(const CServerSocket& _rSocket);
        void UnregisterSocket(const CServerSocket& _rSocket);

        asio::io_service& GetIOService();

    private:

        CNetworkManager();
        ~CNetworkManager();

    private:

        std::vector<const CServerSocket*> m_Sockets;
        std::thread m_WorkerThread;

        std::atomic_bool m_IsRunning;

        asio::io_service m_IOService;
    };
} // namespace Net