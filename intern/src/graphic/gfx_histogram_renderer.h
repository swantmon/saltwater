
#pragma once

#include "graphic/gfx_buffer.h"

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

    void ResetSettings();
    void SetSettings(const SHistogramSettings& _rSettings);
    const SHistogramSettings& GetSettings();

    CBufferPtr GetExposureHistoryBuffer();
    unsigned int GetCurrentExposureHistoryIndex();
    unsigned int GetLastExposureHistoryIndex();
    
    void ResetEyeAdaption();
} // namespace HistogramRenderer
} // namespace Gfx