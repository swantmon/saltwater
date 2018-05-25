
#pragma once

#include "engine/engine_config.h"

#include "base/base_exception.h"
#include "base/base_json.h"

#include "engine/core/core_console.h"

#include <string>

namespace Core
{
    class ENGINE_API CNetwork
    {
    public:

        static CNetwork& GetInstance();

    public:
        
    private:

        CNetwork();
        ~CNetwork();

    };
} // namespace Core

namespace Core
{

} // namespace Core