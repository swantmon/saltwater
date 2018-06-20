
#pragma once

namespace Gfx
{
    struct SIndirectLightSettings
    {
        float m_RSMDivision;
    };
} // namespace Gfx

namespace Gfx
{
namespace LightIndirectRenderer
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
    ENGINE_API void SetSettings(const SIndirectLightSettings& _rSettings);
    ENGINE_API const SIndirectLightSettings& GetSettings();
} // namespace LightIndirectRenderer
} // namespace Gfx
