
#pragma once

#include "engine/engine_config.h"

#include "engine/graphic/gfx_buffer.h"

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

    ENGINE_API void ResetSettings();
    ENGINE_API void SetSettings(const SHistogramSettings& _rSettings);
    ENGINE_API const SHistogramSettings& GetSettings();

    ENGINE_API CBufferPtr GetExposureHistoryBuffer();
    ENGINE_API unsigned int GetCurrentExposureHistoryIndex();
    ENGINE_API unsigned int GetLastExposureHistoryIndex();
    
    ENGINE_API void ResetEyeAdaption();
} // namespace HistogramRenderer
} // namespace Gfx