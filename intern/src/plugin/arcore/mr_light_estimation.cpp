
#include "plugin/arcore/mr_precompiled.h"

#include "base/base_coordinate_system.h"

#include "engine/core/core_plugin.h"

#include "engine/graphic/gfx_texture_manager.h"

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
        , m_CubemapHDRPtr     (nullptr)
    {

    }

    // -----------------------------------------------------------------------------

    CLightEstimation::~CLightEstimation()
    {
        m_CubemapHDRPtr = nullptr;
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

            ArImageCubemap HDRCubemap;

            ArLightEstimate_acquireEnvironmentalHdrCubemap(pArSession, ARLightEstimate, HDRCubemap);

            const uint8_t * pFaceData;
            int W, H;

            ArImage_getHeight(pArSession, HDRCubemap[0], &H);
            ArImage_getWidth(pArSession, HDRCubemap[0], &W);

            if (m_CubemapHDRPtr == nullptr || m_CubemapHDRPtr->GetNumberOfPixelsU() != W || m_CubemapHDRPtr->GetNumberOfPixelsV() != H)
            {
                if (m_CubemapHDRPtr != nullptr) m_CubemapHDRPtr = nullptr;

                Gfx::STextureDescriptor TextureDescriptor = { };

                TextureDescriptor.m_NumberOfPixelsU  = W;
                TextureDescriptor.m_NumberOfPixelsV  = H;
                TextureDescriptor.m_NumberOfPixelsW  = 1;
                TextureDescriptor.m_NumberOfMipMaps  = Gfx::STextureDescriptor::s_GenerateAllMipMaps;
                TextureDescriptor.m_NumberOfTextures = 6;
                TextureDescriptor.m_Binding          = Gfx::CTexture::ShaderResource | Gfx::CTexture::RenderTarget;
                TextureDescriptor.m_Access           = Gfx::CTexture::CPUWrite;
                TextureDescriptor.m_Format           = Gfx::CTexture::Unknown;
                TextureDescriptor.m_Usage            = Gfx::CTexture::GPURead;
                TextureDescriptor.m_Semantic         = Gfx::CTexture::HDR;
                TextureDescriptor.m_pFileName        = nullptr;
                TextureDescriptor.m_pPixels          = nullptr;
                TextureDescriptor.m_Format           = Gfx::CTexture::R16G16B16A16_FLOAT;

                m_CubemapHDRPtr = Gfx::TextureManager::CreateCubeTexture(TextureDescriptor);
            }

            for(auto Face = 0; Face < 6; ++ Face)
            {
                ArImage_getPlaneData(pArSession, HDRCubemap[Face], 0, &pFaceData, 0);

                Gfx::TextureManager::CopyToTextureArray2D(m_CubemapHDRPtr, Face, Base::AABB2UInt(glm::ivec2(0, 0), glm::ivec2(W, H)), 0, pFaceData);

                ArImage_release(HDRCubemap[Face]);
            }

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

    Gfx::CTexturePtr CLightEstimation::GetHDRCubemap() const
    {
        return m_CubemapHDRPtr;
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

extern "C" CORE_PLUGIN_API_EXPORT Gfx::CTexturePtr LightEstimationGetHDRCubemap(const MR::CLightEstimation* _pObj)
{
    return _pObj->GetHDRCubemap();
}