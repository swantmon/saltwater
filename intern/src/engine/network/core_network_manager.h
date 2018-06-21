
#pragma once

#include "engine/engine_config.h"

#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "engine/network/core_network_common.h"
#include "engine/network/core_network_socket.h"

#include <atomic>
#include <map>
#include <memory>
#include <thread>
#include <vector>

namespace Net
{
    class ENGINE_API CNetworkManager : private Base::CUncopyable
    {
    public:

        BASE_SINGLETON_FUNC(CNetworkManager)

    public:

        void OnStart();
        void Update();
        void OnExit();

        bool IsConnected(int _Port = 0) const;

        void RegisterMessageHandler(int _MessageID, const std::shared_ptr<CMessageDelegate>& _rDelegate, int _Port = 0);
        bool SendMessage(int _MessageID, const std::vector<char>& _rData, int _Length = 0, int _Port = 0);

    private:

        void Run();

        friend class CServerSocket;
        
        CServerSocket& GetSocket(int _Port);

        asio::io_service& GetIOService();

    private:

        CNetworkManager();
        ~CNetworkManager();

    private:

        struct SocketDeleter
        {
            void operator()(CServerSocket* _pSocket)
            {
                delete _pSocket;
            }
        };
        
        std::map<int, std::unique_ptr<CServerSocket, SocketDeleter>> m_Sockets;
        
        std::vector<CMessageDelegate> m_MessageDelegates;
        std::thread m_WorkerThread;

        std::atomic_bool m_IsRunning;

        asio::io_service m_IOService;

        int m_DefaultPort;
    };
} // namespace Net