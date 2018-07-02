
#pragma once


#include "engine/engine_config.h"

#include <atomic>
#include <functional>
#include <vector>

namespace Net
{
    typedef std::function<void(int, const std::vector<char>&, int)> CMessageDelegate;
} // namespace Net