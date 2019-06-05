
#pragma once

#include "engine/engine_config.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_manager.h"

namespace Dt
{
    class ENGINE_API CSSAOComponent : public CComponent<CSSAOComponent>
    {
    public:

        CSSAOComponent();
        ~CSSAOComponent();

    public:

        inline void Read(CSceneReader& _rCodec) override
        {
            CComponent::Read(_rCodec);
        }

        inline void Write(CSceneWriter& _rCodec) override
        {
            CComponent::Write(_rCodec);
        }

        inline IComponent* Allocate() override
        {
            return new CSSAOComponent();
        }
    };
} // namespace Dt