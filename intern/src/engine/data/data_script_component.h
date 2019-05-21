
#pragma once

#include "engine/engine_config.h"

#include "base/base_input_event.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_manager.h"

namespace Dt
{
    class ENGINE_API CScriptComponent : public CComponent<CScriptComponent>
    {
    public:

        virtual const Base::ID GetScriptTypeID() const = 0;

    public:

        virtual void Start() = 0;
        virtual void Exit() = 0;
        virtual void Update() = 0;

        virtual void OnInput(const Base::CInputEvent& _rEvent) = 0;

        virtual ~CScriptComponent() {};
    };
} // namespace Dt