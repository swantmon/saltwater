
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
    typedef std::function<void(int, const std::vector<char>&)> CNetworkMessageDelegate;

    class ENGINE_API CNetworkManager : private Base::CUncopyable
    {
    public:

        BASE_SINGLETON_FUNC(CNetworkManager)

    public:

        void OnStart();
        void Update();
        void OnExit();

        void RegisterMessageHandler(int _MessageID, CNetworkMessageDelegate, int _Port = 0);
        void UnregisterMessageHandler(int _MessageID, CNetworkMessageDelegate, int _Port = 0);

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
        std::vector<CNetworkMessageDelegate> m_MessageDelegates;
        std::thread m_WorkerThread;

        std::atomic_bool m_IsRunning;

        asio::io_service m_IOService;

        const static int s_DefaultPort = 12345;
    };
} // namespace Net