
#include "engine/engine_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_plugin_manager.h"

#include "engine/graphic/gfx_reconstruction_renderer.h"

using namespace Gfx;

namespace
{
    class CGfxEngineReconstructionRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxEngineReconstructionRenderer)
        
    public:

        CGfxEngineReconstructionRenderer();
        ~CGfxEngineReconstructionRenderer();
        
    public:

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

        void OnResize(unsigned int _Width, unsigned int _Height);
        
        void Update();
        void Render();
        void RenderForward();
        void RenderHitProxy();

    private:

        typedef void(*FSLAMRenderHitProxy)();

        FSLAMRenderHitProxy SLAMRenderHitProxy;
    };
} // namespace

namespace
{
    using namespace Base;

    CGfxEngineReconstructionRenderer::CGfxEngineReconstructionRenderer()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxEngineReconstructionRenderer::~CGfxEngineReconstructionRenderer()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxEngineReconstructionRenderer::OnStart()
    {
        
    }

    // -----------------------------------------------------------------------------
    
    void CGfxEngineReconstructionRenderer::OnExit()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxEngineReconstructionRenderer::OnSetupShader()
    {
		
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxEngineReconstructionRenderer::OnSetupKernels()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxEngineReconstructionRenderer::OnSetupRenderTargets()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CGfxEngineReconstructionRenderer::OnSetupStates()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxEngineReconstructionRenderer::OnSetupTextures()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxEngineReconstructionRenderer::OnSetupBuffers()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxEngineReconstructionRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxEngineReconstructionRenderer::OnSetupModels()
    {
    }

    // -----------------------------------------------------------------------------
    
    void CGfxEngineReconstructionRenderer::OnSetupEnd()
    {
        SLAMRenderHitProxy = (FSLAMRenderHitProxy)(Core::PluginManager::GetPluginFunction("SLAM", "OnRenderHitProxy"));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxEngineReconstructionRenderer::OnReload()
    {

    }

    // -----------------------------------------------------------------------------
    
    void CGfxEngineReconstructionRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxEngineReconstructionRenderer::OnUnloadMap()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CGfxEngineReconstructionRenderer::OnResize(unsigned int _Width, unsigned int _Height)
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxEngineReconstructionRenderer::Update()
    {

    }
   
    // -----------------------------------------------------------------------------

    void CGfxEngineReconstructionRenderer::Render()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxEngineReconstructionRenderer::RenderForward()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxEngineReconstructionRenderer::RenderHitProxy()
    {
        if (SLAMRenderHitProxy == 0)
        {
            SLAMRenderHitProxy = (FSLAMRenderHitProxy)(Core::PluginManager::GetPluginFunction("SLAM", "OnRenderHitProxy"));
        }

        if (SLAMRenderHitProxy != 0) SLAMRenderHitProxy();
    }
} // namespace

namespace Gfx
{
namespace ReconstructionRenderer
{
    void OnStart()
    {
        CGfxEngineReconstructionRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxEngineReconstructionRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxEngineReconstructionRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxEngineReconstructionRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxEngineReconstructionRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxEngineReconstructionRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxEngineReconstructionRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxEngineReconstructionRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxEngineReconstructionRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxEngineReconstructionRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxEngineReconstructionRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxEngineReconstructionRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxEngineReconstructionRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxEngineReconstructionRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxEngineReconstructionRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxEngineReconstructionRenderer::GetInstance().Render();
    }

    // -----------------------------------------------------------------------------

    void RenderForward()
    {
        CGfxEngineReconstructionRenderer::GetInstance().RenderForward();
    }

    // -----------------------------------------------------------------------------

    void RenderHitProxy()
    {
        CGfxEngineReconstructionRenderer::GetInstance().RenderHitProxy();
    }
} // namespace ReconstructionRenderer
} // namespace Gfx

