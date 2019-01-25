
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

        static CNetworkManager& GetInstance();

    public:

        void OnStart();
        void Update();
        void OnExit();

        bool IsConnected(int _Port) const;

        int CreateServerSocket(int _Port);
        int CreateClientSocket(const std::string& _IP, int _Port);

        void RegisterMessageHandler(int _SocketHandle, const std::shared_ptr<CMessageDelegate>& _rDelegate);
        bool SendMessage(int _Port, const CMessage& _rMessage);
        
    private:

        void Run();

        friend class CServer;
        
        asio::io_service& GetIOService();

    private:

        CNetworkManager();
        ~CNetworkManager();

    private:

        struct SocketDeleter
        {
            void operator()(CServer* _pSocket)
            {
                delete _pSocket;
            }
        };
        
        std::map<int, std::unique_ptr<CServer, SocketDeleter>> m_Sockets;  // the key value refers to port numbers
        
        std::thread m_WorkerThread;

        std::atomic_bool m_IsRunning;

        asio::io_service m_IOService;
    };
} // namespace Net