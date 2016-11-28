
#pragma once

#include "base/base_vector3.h"

namespace Dt
{
    class CFXAAFXFacet
    {
    public:

        void SetLuma(Base::Float3& _rLuma);
        Base::Float3& GetLuma();

        void UpdateEffect();

    public:

        CFXAAFXFacet();
        ~CFXAAFXFacet();

    private:

        Base::Float3 m_Luma;            //< Tone for AA (Default: 0.299f, 0.587f, 0.144f)
    };
} // namespace Dt