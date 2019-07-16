
#include "plugin/arcore/mr_precompiled.h"

#include "base/base_coordinate_system.h"

#include "engine/core/core_plugin.h"

#include "plugin/arcore/mr_light_estimation.h"
#include "plugin/arcore/mr_control_manager.h"

namespace MR
{
    CLightEstimation::CLightEstimation()
        : m_EstimationState   (Undefined)
        , m_PixelIntensity    (1.0f)
        , m_ColorCorrection   (0.0f)
        , m_MainLightIntensity(1.0f)
        , m_MainLightDirection(0.0f, 0.0f, -1.0f)
    {

    }

    // -----------------------------------------------------------------------------

    CLightEstimation::~CLightEstimation()
    {
    }

    // -----------------------------------------------------------------------------

    void CLightEstimation::Detect()
    {
        auto pArSession = MR::ControlManager::GetCurrentSession();
        auto pArFrame = MR::ControlManager::GetCurrentFrame();

        static glm::mat3 s_ARCToEngineMatrix = Base::CCoordinateSystem::GetBaseMatrix(glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,0,-1));

        ArLightEstimate* ARLightEstimate;
        ArLightEstimateState ARLightEstimateState;

        ArLightEstimate_create(pArSession, &ARLightEstimate);

        ArFrame_getLightEstimate(pArSession, pArFrame, ARLightEstimate);

        ArLightEstimate_getState(pArSession, ARLightEstimate, &ARLightEstimateState);

        m_EstimationState = CLightEstimation::NotValid;

        if (ARLightEstimateState == AR_LIGHT_ESTIMATE_STATE_VALID)
        {
            ArLightEstimate_getPixelIntensity(pArSession, ARLightEstimate, &m_PixelIntensity);

            ArLightEstimate_getColorCorrection(pArSession, ARLightEstimate, &m_ColorCorrection[0]);

            ArLightEstimate_getEnvironmentalHdrMainLightIntensity(pArSession, ARLightEstimate, &m_MainLightIntensity[0]);

            ArLightEstimate_getEnvironmentalHdrMainLightDirection(pArSession, ARLightEstimate, &m_MainLightDirection[0]);

            m_MainLightDirection = s_ARCToEngineMatrix * m_MainLightDirection;

            ArLightEstimate_getEnvironmentalHdrAmbientSphericalHarmonics(pArSession, ARLightEstimate, m_AmbientSH);

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

    void CLightEstimation::AcquireHDRCubemap()
    {
        auto pArSession = MR::ControlManager::GetCurrentSession();
        auto pArFrame = MR::ControlManager::GetCurrentFrame();

        ArLightEstimate* ARLightEstimate;
        ArLightEstimateState ARLightEstimateState;

        ArLightEstimate_create(pArSession, &ARLightEstimate);

        ArFrame_getLightEstimate(pArSession, pArFrame, ARLightEstimate);

        ArLightEstimate_getState(pArSession, ARLightEstimate, &ARLightEstimateState);

        if (ARLightEstimateState == AR_LIGHT_ESTIMATE_STATE_VALID)
        {
            ArLightEstimate_acquireEnvironmentalHdrCubemap(pArSession, ARLightEstimate, m_HDRCubemap);
        }
    }

    // -----------------------------------------------------------------------------

    void CLightEstimation::GetHDRCubemap(int _Face, const void** _FaceData, int& _Width,  int& _Height, int& _BufferSize) const
    {
        auto pArSession = MR::ControlManager::GetCurrentSession();

        ArImage* FaceImage = m_HDRCubemap[_Face];

        ArImage_getWidth(pArSession, FaceImage, &_Width);
        ArImage_getHeight(pArSession, FaceImage, &_Height);
        ArImage_getPlaneData(pArSession, FaceImage, 0, (const uint8_t**)_FaceData, &_BufferSize);
    }

    // -----------------------------------------------------------------------------

    void CLightEstimation::FreeHDRCubemap()
    {
        for (auto Face = 0; Face < 6; ++Face)
        {
            ArImage_release(m_HDRCubemap[Face]);
        }
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

extern "C" CORE_PLUGIN_API_EXPORT void LightEstimationAcquireHDRCubemap(MR::CLightEstimation* _pObj)
{
    _pObj->AcquireHDRCubemap();
}

extern "C" CORE_PLUGIN_API_EXPORT void LightEstimationGetHDRCubemap(const MR::CLightEstimation* _pObj, int _Face, const void** _FaceData, int& _Width,  int& _Height, int& _BufferSize)
{
    _pObj->GetHDRCubemap(_Face, _FaceData, _Width, _Height, _BufferSize);
}

extern "C" CORE_PLUGIN_API_EXPORT void LightEstimationFreeHDRCubemap(MR::CLightEstimation* _pObj)
{
    _pObj->FreeHDRCubemap();
}