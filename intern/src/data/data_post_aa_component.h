
#pragma once

#include "engine/engine_config.h"

#include "data/data_component.h"

namespace Dt
{
    class ENGINE_API CPostAAComponent : public CComponent<CPostAAComponent>
    {
    public:

        enum EType
        {
            SMAA,
            FXAA,
            NumberOfTypes,
            Undefined = -1,
        };

        void SetType(EType _Type);
        EType GetType();

        void UpdateEffect();

    public:

        CPostAAComponent();
        ~CPostAAComponent();

    private:

        EType m_Type;
    };
} // namespace Dt