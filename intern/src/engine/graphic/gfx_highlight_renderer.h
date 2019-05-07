
#pragma once

#include "engine/engine_config.h"

#include "engine/data/data_entity.h"

namespace Gfx
{
    struct SHighlightSettings
    {
        glm::vec4 m_HighlightColor;
        bool      m_HighlightUseDepth;
        bool      m_HighlightUseWireframe;
    };
} // namespace Gfx

namespace Gfx
{
namespace HighlightRenderer
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
    
    void Update();
    void Render();

    ENGINE_API void HighlightEntity(Base::ID _EntityID);
    ENGINE_API void Reset();

    ENGINE_API void ResetSettings();
    ENGINE_API void SetSettings(const SHighlightSettings& _rSettings);
    ENGINE_API const SHighlightSettings& GetSettings();
} // namespace SelectionRenderer
} // namespace Gfx
