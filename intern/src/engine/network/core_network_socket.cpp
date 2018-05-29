
#include "engine/engine_precompiled.h"

#include "base/base_exception.h"
#include "engine/network/core_network_manager.h"
#include "engine/network/core_network_socket.h"

#include <functional>
#include <iostream>

namespace Net
{
    CServerSocket::CServerSocket(int _Port)
        : m_Port(_Port)
    {

    }

    // -----------------------------------------------------------------------------

    CServerSocket::~CServerSocket()
    {

    }
} // namespace Net