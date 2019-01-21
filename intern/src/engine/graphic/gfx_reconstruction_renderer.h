#pragma once

namespace Gfx
{
namespace ReconstructionRenderer
{
    void OnStart();
    void OnExit();
    
    void OnSetupShader();
    void OnSetupKernels();
    void OnSetupRenderTargets();
    void OnSetupStates();
    void OnSetupTextures();
    void OnSetupBuffers();
    void OnSetupModels();
    void OnSetupResources();
    void OnSetupEnd();
    
    void OnReload();
    void OnNewMap();
    void OnUnloadMap();
    
    void Update();
    void Render();
    void RenderForward();
    void RenderHitProxy();
} // namespace ReconstructionRenderer
} // namespace Gfx
