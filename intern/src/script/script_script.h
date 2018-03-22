#pragma once

#include "base/base_input_event.h"

#include "data/data_entity.h"

namespace Scpt
{
    class CScript
    {
    public:

        virtual void Start() = 0;
        virtual void Exit() = 0;
        virtual void Update() = 0;

        virtual void OnInput(const Base::CInputEvent& _rEvent) = 0;

    public:

        Dt::CEntity* m_pEntity;
        Base::ID     m_ID;
        bool         m_IsActive;
        bool         m_IsStarted;
    };
} // namespace Scpt