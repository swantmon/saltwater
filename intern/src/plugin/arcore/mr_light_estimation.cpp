
#include "plugin/arcore/mr_precompiled.h"

#include "base/base_coordinate_system.h"

#include "engine/core/core_plugin.h"

#include "plugin/arcore/mr_light_estimation.h"

namespace MR
{
    CLightEstimation::CLightEstimation()
        : m_pArSession        (nullptr)
        , m_EstimationState   (Undefined)
        , m_PixelIntensity    (1.0f)
        , m_ColorCorrection   (0.0f)
        , m_MainLightIntensity(1.0f)
        , m_MainLightDirection(0.0f, 0.0f, -1.0f)
    {

    }

    // -----------------------------------------------------------------------------

    CLightEstimation::~CLightEstimation()
    {
        for (auto Face = 0; Face < 6; ++Face)
        {
            ArImage_release(m_HDRCubemap[Face]);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightEstimation::Detect(ArSession* _pARSession, ArFrame* _pFrame)
    {
        m_pArSession = _pARSession;

        static glm::mat3 s_ARCToEngineMatrix = Base::CCoordinateSystem::GetBaseMatrix(glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,0,-1));

        ArLightEstimate* ARLightEstimate;
        ArLightEstimateState ARLightEstimateState;

        ArLightEstimate_create(_pARSession, &ARLightEstimate);

        ArFrame_getLightEstimate(_pARSession, _pFrame, ARLightEstimate);

        ArLightEstimate_getState(_pARSession, ARLightEstimate, &ARLightEstimateState);

        m_EstimationState = CLightEstimation::NotValid;

        if (ARLightEstimateState == AR_LIGHT_ESTIMATE_STATE_VALID)
        {
            ArLightEstimate_getPixelIntensity(_pARSession, ARLightEstimate, &m_PixelIntensity);

            ArLightEstimate_getColorCorrection(_pARSession, ARLightEstimate, &m_ColorCorrection[0]);

            ArLightEstimate_getEnvironmentalHdrMainLightIntensity(_pARSession, ARLightEstimate, &m_MainLightIntensity[0]);

            ArLightEstimate_getEnvironmentalHdrMainLightDirection(_pARSession, ARLightEstimate, &m_MainLightDirection[0]);

            m_MainLightDirection = s_ARCToEngineMatrix * m_MainLightDirection;

            ArLightEstimate_getEnvironmentalHdrAmbientSphericalHarmonics(_pARSession, ARLightEstimate, m_AmbientSH);

            ArLightEstimate_acquireEnvironmentalHdrCubemap(_pARSession, ARLightEstimate, m_HDRCubemap);

            m_EstimationState = CLightEstimation::Valid;
        }

        ArLightEstimate_destroy(ARLightEstimate);

        ARLightEstimate = nullptr;
    }

    // -----------------------------------------------------------------------------

    CLightEstimation::EEstimationState CLightEstimation::GetEstimationState() const
    {
        return m_EstimationState;
    }

    // -----------------------------------------------------------------------------

    float CLightEstimation::GetPixelIntensity() const
    {
        return m_PixelIntensity;
    }

    // -----------------------------------------------------------------------------

    const glm::vec4& CLightEstimation::GetColorCorrection() const
    {
        return m_ColorCorrection;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CLightEstimation::GetMainLightIntensity() const
    {
        return m_MainLightIntensity;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CLightEstimation::GetMainLightDirection() const
    {
        return m_MainLightDirection;
    }

    // -----------------------------------------------------------------------------

    const float* CLightEstimation::GetAmbientSHCoefficients() const
    {
        return m_AmbientSH;
    }

    // -----------------------------------------------------------------------------

    void CLightEstimation::GetHDRCubemap(int _Face, const void** _FaceData, int& _Width,  int& _Height, int& _BufferSize) const
    {
        ArImage* FaceImage = m_HDRCubemap[_Face];

        ArImage_getWidth(m_pArSession, FaceImage, &_Width);
        ArImage_getHeight(m_pArSession, FaceImage, &_Height);
        ArImage_getPlaneData(m_pArSession, FaceImage, 0, (const uint8_t**)_FaceData, &_BufferSize);
    }
} // namespace MR

extern "C" CORE_PLUGIN_API_EXPORT MR::CLightEstimation::EEstimationState GetLightEstimationState(const MR::CLightEstimation* _pObj)
{
    return _pObj->GetEstimationState();
}

extern "C" CORE_PLUGIN_API_EXPORT float GetLightEstimationPixelIntensity(const MR::CLightEstimation* _pObj)
{
    return _pObj->GetPixelIntensity();
}

extern "C" CORE_PLUGIN_API_EXPORT glm::vec4 GetLightEstimationColorCorrection(const MR::CLightEstimation* _pObj)
{
    return _pObj->GetColorCorrection();
}

extern "C" CORE_PLUGIN_API_EXPORT glm::vec3 GetLightEstimationMainLightIntensity(const MR::CLightEstimation* _pObj)
{
    return _pObj->GetMainLightIntensity();
}

extern "C" CORE_PLUGIN_API_EXPORT glm::vec3 GetLightEstimationMainLightDirection(const MR::CLightEstimation* _pObj)
{
    return _pObj->GetMainLightDirection();
}

extern "C" CORE_PLUGIN_API_EXPORT const float* GetLightEstimationAmbientSHCoefficients(const MR::CLightEstimation* _pObj)
{
    return _pObj->GetAmbientSHCoefficients();
}

extern "C" CORE_PLUGIN_API_EXPORT void GetLightEstimationHDRCubemap(const MR::CLightEstimation* _pObj, int _Face, const void** _FaceData, int& _Width,  int& _Height, int& _BufferSize)
{
    _pObj->GetHDRCubemap(_Face, _FaceData, _Width, _Height, _BufferSize);
}