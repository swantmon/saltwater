
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

        using CMessageDelegate = CSocket::CMessageDelegate;

    public:

        void OnStart();
        void Update();
        void OnExit();

        bool IsConnected(SocketHandle _SocketHandle) const;

        int GetPort(SocketHandle _SocketHandle) const;
        const std::string& GetIP(SocketHandle _SocketHandle) const;

        SocketHandle CreateServerSocket(int _Port);
        SocketHandle CreateClientSocket(const std::string& _IP, int _Port);

        CMessageDelegate::HandleType RegisterMessageHandler(SocketHandle _SocketHandle, CMessageDelegate::FunctionType _Function);
        bool SendMessage(SocketHandle _SocketHandle, const CMessage& _rMessage);
        
    private:

        void Run();

        friend class CSocket;
        
        asio::io_service& GetIOService();

    private:

        CNetworkManager();
        ~CNetworkManager();

    private:

        struct SocketDeleter
        {
            void operator()(CSocket* _pSocket)
            {
                delete _pSocket;
            }
        };
        
        std::map<SocketHandle, std::unique_ptr<CSocket, SocketDeleter>> m_Sockets;
        
        std::thread m_WorkerThread;

        std::atomic_bool m_IsRunning;

        asio::io_service m_IOService;
    };
} // namespace Net