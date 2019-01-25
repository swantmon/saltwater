
#pragma once

#include "engine/engine_config.h"

#include "base/base_include_glm.h"

#include "engine/data/data_component.h"

namespace Dt
{
    class ENGINE_API CBloomComponent : public CComponent<CBloomComponent>
    {
    public:

        void SetTint(glm::vec4& _rTint);
        glm::vec4& GetTint();

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

        CBloomComponent();
        ~CBloomComponent();

    private:

        glm::vec4    m_Tint;                //> Modifies the brightness and color of each bloom. (Default: 1.0, 1.0, 1.0)
        float        m_Intensity;           //> Scales the color of the whole bloom effect. (Range: 0.0 - 8.0; Default: 1)
        float        m_Treshhold;           //> Defines how many luminance units a color needs to have to affect bloom. (Range: -1.0 - 8.0; Default: 1.0f)
        float        m_ExposureScale;       //> Defines a multiplier applied on bloom effect (Default: 2.0f)
        unsigned int m_Size;                //> Size of the bloom lights (Range: 1, 2, 3, 4, 5; Default: 3)

    private:

        friend class CBloomComponentGUI;
    };
} // namespace Dt