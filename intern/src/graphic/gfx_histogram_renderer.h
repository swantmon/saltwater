
#pragma once

#include "graphic/gfx_buffer.h"
#include "graphic/gfx_export.h"

namespace Gfx
{
    struct SHistogramSettings
    {
        float m_HistogramLowerBound;
        float m_HistogramUpperBound;
        float m_HistogramLogMin;
        float m_HistogramLogMax;
        float m_EyeAdaptionSpeedUp;
        float m_EyeAdaptionSpeedDown;
        bool  m_ResetEyeAdaption;
    };
} // namespace Gfx

namespace Gfx
{
namespace HistogramRenderer
{
    void OnStart();
    void OnExit();

    void OnSetupShader();
    void OnSetupKernels();
    void OnSetupRenderTargets();
    void OnSetupStates();
    void OnSetupTextures();
    void OnSetupBuffers();
    void OnSetupResources();
    void OnSetupModels();
    void OnSetupEnd();

    void OnReload();
    void OnNewMap();
    void OnUnloadMap();

    void Update();
    void Render();

    GFX_API void ResetSettings();
    GFX_API void SetSettings(const SHistogramSettings& _rSettings);
    GFX_API const SHistogramSettings& GetSettings();

    GFX_API CBufferPtr GetExposureHistoryBuffer();
    GFX_API unsigned int GetCurrentExposureHistoryIndex();
    GFX_API unsigned int GetLastExposureHistoryIndex();
    
    GFX_API void ResetEyeAdaption();
} // namespace HistogramRenderer
} // namespace Gfx