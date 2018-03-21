#pragma once

#include "base/base_input_event.h"

#include "data/data_entity.h"

namespace Script
{
    class CBaseScript
    {
    public:

        Dt::CEntity* m_pEntity;

    public:

        virtual void Start() = 0;
        virtual void Exit() = 0;
        virtual void Update() = 0;

        virtual void OnInput(const Base::CInputEvent& _rEvent) = 0;
    };
} // namespace Script