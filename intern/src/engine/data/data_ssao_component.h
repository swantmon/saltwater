
#pragma once

#include "engine/engine_config.h"

#include "engine/data/data_component.h"

namespace Dt
{
    class ENGINE_API CSSAOComponent : public CComponent<CSSAOComponent>
    {
    public:

        CSSAOComponent();
        ~CSSAOComponent();
    };
} // namespace Dt