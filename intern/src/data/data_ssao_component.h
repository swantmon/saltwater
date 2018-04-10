
#pragma once

#include "engine/engine_config.h"

#include "data/data_component.h"

namespace Dt
{
    class ENGINE_API CSSAOComponent : public CComponent<CSSAOComponent>
    {
    public:

        CSSAOComponent();
        ~CSSAOComponent();
    };
} // namespace Dt