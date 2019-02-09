
#pragma once


#include "engine/engine_config.h"

#include <atomic>
#include <functional>
#include <vector>

namespace Net
{
    struct CMessage
    {
        int m_Category;
        int m_MessageType;
        int m_CompressedSize;
        int m_DecompressedSize;
        std::vector<char> m_Payload;
    };

    using SocketHandle = int;
} // namespace Net