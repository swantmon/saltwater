
#pragma once

#include "base/base_vector3.h"

namespace Dt
{
    class CPostAAFXFacet
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

        CPostAAFXFacet();
        ~CPostAAFXFacet();

    private:

        EType m_Type;
    };
} // namespace Dt