
#include "engine/engine_precompiled.h"

#include "engine/core/core_network.h"

namespace Core
{
    CNetwork& CNetwork::GetInstance()
    {
        static CNetwork s_Sinstance;

        return s_Sinstance;
    }
} // namespace Core

using namespace nlohmann;

namespace Core
{
    CNetwork::CNetwork()
    {

    }

    // -----------------------------------------------------------------------------

    CNetwork::~CNetwork()
    {

    }
} // namespace Core