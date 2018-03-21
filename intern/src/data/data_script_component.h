
#pragma once

#include "data/data_component.h"

namespace Dt
{
    class CScriptComponent : public CComponent<CScriptComponent>
    {
    public:

        void* m_pScript;
    };
} // namespace Dt