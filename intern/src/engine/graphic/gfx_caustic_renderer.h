
#pragma once

namespace Gfx
{
    struct SCausticSettings
    {

    };
} // namespace Gfx

namespace Gfx
{
namespace CausticRenderer
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

    void ResetSettings();
    void SetSetting(const SCausticSettings& _rSettings);
    const SCausticSettings& GetSettings();
} // namespace CausticRenderer
} // namespace Gfx
