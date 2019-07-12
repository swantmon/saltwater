#pragma once

#include "base/base_include_glm.h"

#include "arcore_c_api.h"

namespace MR
{
    class CLightEstimation
    {
    public:

        static const int s_NumberOfSHCoefficients = 27;

    public:

        enum EEstimationState
        {
            NotValid,
            Valid,
            Undefined
        };

    public:

        CLightEstimation();
        ~CLightEstimation();

        EEstimationState GetEstimationState() const;
        float GetPixelIntensity() const;
        const glm::vec4& GetColorCorrection() const;
        const glm::vec3& GetMainLightIntensity() const;
        const glm::vec3& GetMainLightDirection() const;
        const float* GetAmbientSHCoefficients() const;
        const ArImageCubemap& GetHDRCubemap() const;

    protected:

        EEstimationState m_EstimationState;

        float m_PixelIntensity;
        glm::vec4 m_ColorCorrection;
        glm::vec3 m_MainLightIntensity;
        glm::vec3 m_MainLightDirection;
        float m_AmbientSH[s_NumberOfSHCoefficients];
        ArImageCubemap m_HDRCubemap;
    };
} // namespace MR