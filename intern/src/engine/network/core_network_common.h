
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

		CMessage()
			: m_Category(0)
			, m_MessageType(0)
			, m_CompressedSize(0)
			, m_DecompressedSize(0)
			, m_Payload()
		{

		}
    };

    using SocketHandle = int;
} // namespace Net