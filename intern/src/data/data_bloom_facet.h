//
//  data_fx_facet.h
//  data
//
//  Created by Tobias Schwandt on 29/03/16.
//  Copyright © 2016 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_vector4.h"

namespace Dt
{
    class CBloomFXFacet
    {
    public:

        void SetTint(Base::Float4& _rTint);
        Base::Float4& GetTint();

        void SetIntensity(float _Intensity);
        float GetIntensity();

        void SetTreshhold(float _Treshhold);
        float GetTreshhold();

        void SetExposureScale(float _ExposureScale);
        float GetExposureScale();

        void SetSize(unsigned int _Size);
        unsigned int GetSize();

        void UpdateEffect();

    public:

        CBloomFXFacet();
        ~CBloomFXFacet();

    private:

        Base::Float4 m_Tint;                //> Modifies the brightness and color of each bloom. (Default: 1.0, 1.0, 1.0)
        float        m_Intensity;           //> Scales the color of the whole bloom effect. (Range: 0.0 - 8.0; Default: 1)
        float        m_Treshhold;           //> Defines how many luminance units a color needs to have to affect bloom. (Range: -1.0 - 8.0; Default: 1.0f)
        float        m_ExposureScale;       //> Defines a multiplier applied on bloom effect (Default: 2.0f)
        unsigned int m_Size;                //> Size of the bloom lights (Range: 1, 2, 3, 4, 5; Default: 3)
    };
} // namespace Dt