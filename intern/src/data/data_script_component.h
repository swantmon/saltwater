
#pragma once

#include "base/base_input_event.h"

#include "data/data_component.h"

namespace Dt
{
    class CScriptComponent : public CComponent<CScriptComponent>
    {
    public:

        inline Dt::CEntity* GetEntity();
        inline Dt::CTransformationFacet* GetTransformation();

    public:

        virtual void Start() = 0;
        virtual void Exit() = 0;
        virtual void Update() = 0;

        virtual void OnInput(const Base::CInputEvent& _rEvent) = 0;
    };
} // namespace Dt

namespace Dt
{
    inline Dt::CEntity* CScriptComponent::GetEntity()
    {
        return m_pHostEntity;
    }

    // -----------------------------------------------------------------------------

    inline Dt::CTransformationFacet* CScriptComponent::GetTransformation()
    {
        if (m_pHostEntity == nullptr) return nullptr;

        return m_pHostEntity->GetTransformationFacet();
    }
} // namespace Dt