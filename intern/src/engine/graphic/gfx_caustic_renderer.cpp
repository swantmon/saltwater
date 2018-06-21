
#include "engine/engine_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_asset_manager.h"
#include "engine/core/core_console.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_caustic_renderer.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    class CGfxCausticRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxCausticRenderer)
        
    public:
        
        CGfxCausticRenderer();
        ~CGfxCausticRenderer();
        
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
        
        void Update();
        void Render();

        void ResetSettings();
        void SetSetting(const SCausticSettings& _rSettings);
        const SCausticSettings& GetSettings();

    private:

        SCausticSettings m_Settings;
    };
} // namespace

namespace
{
    CGfxCausticRenderer::CGfxCausticRenderer()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxCausticRenderer::~CGfxCausticRenderer()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnStart()
    {
        ResetSettings();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnExit()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupShader()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupRenderTargets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupStates()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupTextures()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupBuffers()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupResources()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupModels()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::OnReload()
    {
        
    }

    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::Update()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxCausticRenderer::Render()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxCausticRenderer::ResetSettings()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxCausticRenderer::SetSetting(const SCausticSettings& _rSettings)
    {
        m_Settings = _rSettings;
    }

    // -----------------------------------------------------------------------------

    const SCausticSettings& CGfxCausticRenderer::GetSettings()
    {
        return m_Settings;
    }
} // namespace


namespace Gfx
{
namespace CausticRenderer
{
    void OnStart()
    {
        CGfxCausticRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxCausticRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    
    void OnSetupShader()
    {
        CGfxCausticRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxCausticRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxCausticRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxCausticRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxCausticRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxCausticRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxCausticRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxCausticRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxCausticRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxCausticRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxCausticRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxCausticRenderer::GetInstance().Render();
    }

    // -----------------------------------------------------------------------------

    void ResetSettings()
    {
        CGfxCausticRenderer::GetInstance().ResetSettings();
    }

    // -----------------------------------------------------------------------------

    void SetSetting(const SCausticSettings& _rSettings)
    {
        CGfxCausticRenderer::GetInstance().SetSetting(_rSettings);
    }

    // -----------------------------------------------------------------------------

    const SCausticSettings& GetSettings()
    {
        return CGfxCausticRenderer::GetInstance().GetSettings();
    }
} // namespace CausticRenderer
} // namespace Gfx
