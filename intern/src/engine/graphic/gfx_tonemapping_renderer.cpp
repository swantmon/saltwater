
#include "engine/engine_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_program_parameters.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_mesh.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_tonemapping_renderer.h"
#include "engine/graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    class CGfxShadingRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxShadingRenderer)
        
    public:
        CGfxShadingRenderer();
        ~CGfxShadingRenderer();
        
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
        
        void Update();
        void Render();

        void ResetSettings();
        void SetSettings(const SColorGradingSettings& _rSettings);
        const SColorGradingSettings& GetSettings();

    private:

        struct SConstantBufferPS
        {
            glm::vec4 m_ColorMatrixR_ColorCurveCd1;
            glm::vec4 m_ColorMatrixG_ColorCurveCd3Cm3;
            glm::vec4 m_ColorMatrixB_ColorCurveCm2;
            glm::vec4 m_ColorCurve_Cm0Cd0_Cd2_Ch0Cm1_Ch3;
            glm::vec4 m_ColorCurve_Ch1_Ch2;
            glm::vec4 m_ColorShadow_Luma;
            glm::vec4 m_ColorShadow_Tint1;
            glm::vec4 m_ColorShadow_Tint2;
        };

    private:
        
        SConstantBufferPS      m_ConstantBufferPS;

        CBufferPtr             m_TonemapBufferPtr;
        
        CShaderPtr             m_FullquadShaderVSPtr;
        
        CShaderPtr             m_ShadingPSPtr;

        CRenderContextPtr      m_ShadingContextPtr;

        SColorGradingSettings  m_ColorGradingSettings;
    };
} // namespace

namespace
{
    CGfxShadingRenderer::CGfxShadingRenderer()
        : m_ConstantBufferPS      ()
        , m_TonemapBufferPtr      ()
        , m_FullquadShaderVSPtr   ()
        , m_ShadingPSPtr          ()
        , m_ColorGradingSettings  ()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxShadingRenderer::~CGfxShadingRenderer()
    {
    
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::OnStart()
    {
        ResetSettings();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::OnExit()
    {
        m_TonemapBufferPtr       = 0;
        m_FullquadShaderVSPtr    = 0;
        m_ShadingPSPtr           = 0;
        m_ShadingContextPtr      = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::OnSetupShader()
    {
        m_FullquadShaderVSPtr   = ShaderManager::CompileVS("vs_fullscreen.glsl" , "main");

        m_ShadingPSPtr          = ShaderManager::CompilePS("fs_tone_mapping.glsl", "main");
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::OnSetupRenderTargets()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::OnSetupStates()
    {
        CCameraPtr          MainCameraPtr      = ViewManager ::GetMainCamera();
        CCameraPtr          QuadCameraPtr      = ViewManager ::GetFullQuadCamera();

        CViewPortSetPtr     ViewPortSetPtr     = ViewManager ::GetViewPortSet();

        CRenderStatePtr     ShadingStatePtr    = StateManager::GetRenderState(CRenderState::NoDepth);

        CTargetSetPtr       SystemTargetSetPtr = TargetSetManager::GetDefaultTargetSet();

        // -----------------------------------------------------------------------------

        CRenderContextPtr ShadingContextPtr = ContextManager::CreateRenderContext();

        ShadingContextPtr->SetCamera(QuadCameraPtr);
        ShadingContextPtr->SetViewPortSet(ViewPortSetPtr);
        ShadingContextPtr->SetTargetSet(SystemTargetSetPtr);
        ShadingContextPtr->SetRenderState(ShadingStatePtr);

        m_ShadingContextPtr = ShadingContextPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::OnSetupTextures()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstanteBufferDesc;
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SConstantBufferPS);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_TonemapBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::OnSetupModels()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::OnUnloadMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::Update()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::Render()
    {
        Performance::BeginEvent("Tonemapping");

        // -----------------------------------------------------------------------------
        // Upload data
        // -----------------------------------------------------------------------------
        SConstantBufferPS ConstantBufferPS;

        ConstantBufferPS.m_ColorMatrixR_ColorCurveCd1       = m_ConstantBufferPS.m_ColorMatrixR_ColorCurveCd1;
        ConstantBufferPS.m_ColorMatrixG_ColorCurveCd3Cm3    = m_ConstantBufferPS.m_ColorMatrixG_ColorCurveCd3Cm3;
        ConstantBufferPS.m_ColorMatrixB_ColorCurveCm2       = m_ConstantBufferPS.m_ColorMatrixB_ColorCurveCm2;
        ConstantBufferPS.m_ColorCurve_Cm0Cd0_Cd2_Ch0Cm1_Ch3 = m_ConstantBufferPS.m_ColorCurve_Cm0Cd0_Cd2_Ch0Cm1_Ch3;
        ConstantBufferPS.m_ColorCurve_Ch1_Ch2               = m_ConstantBufferPS.m_ColorCurve_Ch1_Ch2;
        ConstantBufferPS.m_ColorShadow_Luma                 = m_ConstantBufferPS.m_ColorShadow_Luma;
        ConstantBufferPS.m_ColorShadow_Tint1                = m_ConstantBufferPS.m_ColorShadow_Tint1;
        ConstantBufferPS.m_ColorShadow_Tint2                = m_ConstantBufferPS.m_ColorShadow_Tint2;

        BufferManager::UploadBufferData(m_TonemapBufferPtr, &ConstantBufferPS);

        // -----------------------------------------------------------------------------
        // Prepare renderer
        // -----------------------------------------------------------------------------
        

        ContextManager::SetRenderContext(m_ShadingContextPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_FullquadShaderVSPtr);

        ContextManager::SetShaderPS(m_ShadingPSPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_TonemapBufferPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

        ContextManager::SetTexture(0, TargetSetManager::GetLightAccumulationTargetSet()->GetRenderTarget(0));

        ContextManager::Draw(3, 0);

        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetTopology();

        ContextManager::ResetSampler(0);

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxShadingRenderer::ResetSettings()
    {
        ENGINE_CONSOLE_STREAMINFO("Gfx> Loading default settings for final shading & tone mapping");
        
        SColorGradingSettings Settings;

        Settings.m_Tint             = Core::CProgramParameters::GetInstance().Get("graphics:tone_mapping:tint", glm::vec3(1.0f));
        Settings.m_DarkTint         = Core::CProgramParameters::GetInstance().Get("graphics:tone_mapping:dark_tint:color", glm::vec3(0.2f));
        Settings.m_DarkTintBlend    = Core::CProgramParameters::GetInstance().Get("graphics:tone_mapping:dark_tint:blend", 0.5f);
        Settings.m_DarkTintStrength = Core::CProgramParameters::GetInstance().Get("graphics:tone_mapping:dark_tint:strength", 0.5f);
        Settings.m_Saturation       = Core::CProgramParameters::GetInstance().Get("graphics:tone_mapping:saturation", 1.0f);
        Settings.m_Contrast         = Core::CProgramParameters::GetInstance().Get("graphics:tone_mapping:contrast", 0.05f);
        Settings.m_DynamicRange     = Core::CProgramParameters::GetInstance().Get("graphics:tone_mapping:dynamic_range", 4.0f);
        Settings.m_CrushBrights     = Core::CProgramParameters::GetInstance().Get("graphics:tone_mapping:crush_brights", 0.18f);
        Settings.m_CrushDarks       = Core::CProgramParameters::GetInstance().Get("graphics:tone_mapping:crush_darks", 1.0f);
        Settings.m_Red              = Core::CProgramParameters::GetInstance().Get("graphics:tone_mapping:red", glm::vec3(1.0f, 0.0f, 0.0f));
        Settings.m_Green            = Core::CProgramParameters::GetInstance().Get("graphics:tone_mapping:green", glm::vec3(0.0f, 1.0f, 0.0f));
        Settings.m_Blue             = Core::CProgramParameters::GetInstance().Get("graphics:tone_mapping:blue", glm::vec3(0.0f, 0.0f, 1.0f));

        SetSettings(Settings);
    }

    // -----------------------------------------------------------------------------

    void CGfxShadingRenderer::SetSettings(const SColorGradingSettings& _rSettings)
    {
        // -----------------------------------------------------------------------------
        // Save settings
        // -----------------------------------------------------------------------------
        m_ColorGradingSettings = _rSettings;

        // -----------------------------------------------------------------------------
        // Must insure inputs are in correct range (else possible generation of NaNs).
        // -----------------------------------------------------------------------------
        float InExposure = 1.0f;

        glm::vec3 InWhitePoint = _rSettings.m_Tint;

        float InSaturation = glm::clamp(_rSettings.m_Saturation, 0.0f, 2.0f);

        glm::vec3 InLuma(1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f);

        glm::vec3 InMatrixR = _rSettings.m_Red;
        glm::vec3 InMatrixG = _rSettings.m_Green;
        glm::vec3 InMatrixB = _rSettings.m_Blue;

        float InContrast = glm::clamp(_rSettings.m_Contrast, 0.0f, 1.0f) + 1.0f;

        float InDynamicRange = glm::pow(2.0f, glm::clamp(_rSettings.m_DynamicRange, 1.0f, 18.0f));

        float InToe = (1.0f - glm::clamp(_rSettings.m_CrushDarks, 0.0f, 1.0f)) * 0.18f;
        InToe = glm::clamp(InToe, 0.18f / 8.0f, 0.18f * (15.0f / 16.0f));

        float InHeal = 1.0f - (glm::max(1.0f / 32.0f, 1.0f - glm::clamp(_rSettings.m_CrushBrights, 0.0f, 1.0f)) * (1.0f - 0.18f)); 

        glm::vec3 InShadowTint = _rSettings.m_DarkTint;

        float InShadowTintBlend = glm::clamp(_rSettings.m_DarkTintBlend, 0.0f, 1.0f) * 64.0f;

        // -----------------------------------------------------------------------------
        // Shadow tint amount enables turning off shadow tinting.
        // -----------------------------------------------------------------------------
        float InShadowTintAmount = glm::clamp(_rSettings.m_DarkTintStrength, 0.0f, 1.0f);

        InShadowTint = InWhitePoint + (InShadowTint - InWhitePoint) * InShadowTintAmount;

        // -----------------------------------------------------------------------------
        // Make sure channel mixer inputs sum to 1 (+ smart dealing with all zeros).
        // -----------------------------------------------------------------------------
        InMatrixR[0] += 1.0f / (256.0f * 256.0f * 32.0f);
        InMatrixG[1] += 1.0f / (256.0f * 256.0f * 32.0f);
        InMatrixB[2] += 1.0f / (256.0f * 256.0f * 32.0f);

        InMatrixR *= 1.0f / glm::dot(InMatrixR, glm::vec3(1.0f));
        InMatrixG *= 1.0f / glm::dot(InMatrixG, glm::vec3(1.0f));
        InMatrixB *= 1.0f / glm::dot(InMatrixB, glm::vec3(1.0f));

        // -----------------------------------------------------------------------------
        // Conversion from linear rgb to luma (using HDTV coef).
        // -----------------------------------------------------------------------------
        glm::vec3 LumaWeights(0.2126f, 0.7152f, 0.0722f);

        // -----------------------------------------------------------------------------
        // Make sure white point has 1.0 as luma (so adjusting white point doesn't 
        // change exposure).
        // Make sure {0.0,0.0,0.0} inputs do something sane (default to white).
        // -----------------------------------------------------------------------------
        InWhitePoint += glm::vec3(1.0f / (256.0f * 256.0f * 32.0f));
        InWhitePoint *= 1.0f / glm::dot(InWhitePoint, LumaWeights);
        InShadowTint += glm::vec3(1.0f / (256.0f * 256.0f * 32.0f));
        InShadowTint *= 1.0f / glm::dot(InShadowTint, LumaWeights);

        // Grey after color matrix is applied.
        glm::vec3 ColorMatrixLuma(0.0f);
        
        ColorMatrixLuma[0] = glm::dot(glm::vec3(1.0f), glm::vec3(InMatrixR[0], InMatrixG[0], InMatrixB[0]) * InLuma[0]);
        ColorMatrixLuma[1] = glm::dot(glm::vec3(1.0f), glm::vec3(InMatrixR[1], InMatrixG[1], InMatrixB[1]) * InLuma[1]);
        ColorMatrixLuma[2] = glm::dot(glm::vec3(1.0f), glm::vec3(InMatrixR[2], InMatrixG[2], InMatrixB[2]) * InLuma[2]);

        glm::vec3 OutMatrixR(0.0f);
        glm::vec3 OutMatrixG(0.0f);
        glm::vec3 OutMatrixB(0.0f);

        glm::vec3 OutColorShadow_Luma = LumaWeights * InShadowTintBlend;
        glm::vec3 OutColorShadow_Tint1 = InWhitePoint;
        glm::vec3 OutColorShadow_Tint2 = InShadowTint - InWhitePoint;

        // -----------------------------------------------------------------------------
        // Final color matrix effected by saturation and exposure.
        // -----------------------------------------------------------------------------
        OutMatrixR = (ColorMatrixLuma + ((InMatrixR - ColorMatrixLuma) * InSaturation)) * InExposure;
        OutMatrixG = (ColorMatrixLuma + ((InMatrixG - ColorMatrixLuma) * InSaturation)) * InExposure;
        OutMatrixB = (ColorMatrixLuma + ((InMatrixB - ColorMatrixLuma) * InSaturation)) * InExposure;

        // -----------------------------------------------------------------------------
        // Curve constants.
        // -----------------------------------------------------------------------------
        float OutColorCurveCh3;
        float OutColorCurveCh0Cm1;
        float OutColorCurveCd2;
        float OutColorCurveCm0Cd0;
        float OutColorCurveCh1;
        float OutColorCurveCh2;
        float OutColorCurveCd1;
        float OutColorCurveCd3Cm3;
        float OutColorCurveCm2;

        // -----------------------------------------------------------------------------
        // Line for linear section.
        // -----------------------------------------------------------------------------
        float FilmLineOffset = 0.18f - 0.18f * InContrast;
        float FilmXAtY0 = -FilmLineOffset / InContrast;
        float FilmXAtY1 = (1.0f - FilmLineOffset) / InContrast;
        float FilmXS = FilmXAtY1 - FilmXAtY0;

        // -----------------------------------------------------------------------------
        // Coordinates of linear section.
        // -----------------------------------------------------------------------------
        float FilmHiX = FilmXAtY0 + InHeal * FilmXS;
        float FilmHiY = FilmHiX * InContrast + FilmLineOffset;
        float FilmLoX = FilmXAtY0 + InToe * FilmXS;
        float FilmLoY = FilmLoX * InContrast + FilmLineOffset;

        // -----------------------------------------------------------------------------
        // Supported exposure range before clipping.
        // -----------------------------------------------------------------------------
        float FilmHeal = InDynamicRange - FilmHiX;

        // -----------------------------------------------------------------------------
        // Intermediates.
        // -----------------------------------------------------------------------------
        float FilmMidXS = FilmHiX - FilmLoX;
        float FilmMidYS = FilmHiY - FilmLoY;
        float FilmSlope = FilmMidYS / (FilmMidXS);
        float FilmHiYS = 1.0f - FilmHiY;
        float FilmLoYS = FilmLoY;
        float FilmToe = FilmLoX;
        float FilmHiG = (-FilmHiYS + (FilmSlope * FilmHeal)) / (FilmSlope * FilmHeal);
        float FilmLoG = (-FilmLoYS + (FilmSlope * FilmToe)) / (FilmSlope * FilmToe);

        // -----------------------------------------------------------------------------
        // Constants.
        // -----------------------------------------------------------------------------
        OutColorCurveCh1 = FilmHiYS / FilmHiG;
        OutColorCurveCh2 = -FilmHiX * (FilmHiYS / FilmHiG);
        OutColorCurveCh3 = FilmHiYS / (FilmSlope * FilmHiG) - FilmHiX;
        OutColorCurveCh0Cm1 = FilmHiX;
        OutColorCurveCm2 = FilmSlope;
        OutColorCurveCm0Cd0 = FilmLoX;
        OutColorCurveCd3Cm3 = FilmLoY - FilmLoX * FilmSlope;

        // -----------------------------------------------------------------------------
        // Handle these separate in case of FilmLoG being 0.
        // -----------------------------------------------------------------------------
        if (FilmLoG != 0.0f)
        {
            OutColorCurveCd1 = -FilmLoYS / FilmLoG;
            OutColorCurveCd2 = FilmLoYS / (FilmSlope * FilmLoG);
        }
        else
        {
            // -----------------------------------------------------------------------------
            // FilmLoG being zero means dark region is a linear segment (so just continue 
            // the middle section).
            // -----------------------------------------------------------------------------
            OutColorCurveCd1 = 0.0f;
            OutColorCurveCd2 = 1.0f;
            OutColorCurveCm0Cd0 = 0.0f;
            OutColorCurveCd3Cm3 = 0.0f;
        }

        m_ConstantBufferPS.m_ColorMatrixR_ColorCurveCd1       = glm::vec4(OutMatrixR, OutColorCurveCd1);
        m_ConstantBufferPS.m_ColorMatrixG_ColorCurveCd3Cm3    = glm::vec4(OutMatrixG, OutColorCurveCd3Cm3); 
        m_ConstantBufferPS.m_ColorMatrixB_ColorCurveCm2       = glm::vec4(OutMatrixB, OutColorCurveCm2); 
        m_ConstantBufferPS.m_ColorCurve_Cm0Cd0_Cd2_Ch0Cm1_Ch3 = glm::vec4(OutColorCurveCm0Cd0, OutColorCurveCd2, OutColorCurveCh0Cm1, OutColorCurveCh3); 
        m_ConstantBufferPS.m_ColorCurve_Ch1_Ch2               = glm::vec4(OutColorCurveCh1, OutColorCurveCh2, 0.0f, 0.0f); 
        m_ConstantBufferPS.m_ColorShadow_Luma                 = glm::vec4(OutColorShadow_Luma, 0.0f);
        m_ConstantBufferPS.m_ColorShadow_Tint1                = glm::vec4(OutColorShadow_Tint1, 0.0f);
        m_ConstantBufferPS.m_ColorShadow_Tint2                = glm::vec4(OutColorShadow_Tint2, 0.0f);
    }

    // -----------------------------------------------------------------------------

    const SColorGradingSettings& CGfxShadingRenderer::GetSettings()
    {
        return m_ColorGradingSettings;
    }
} // namespace


namespace Gfx
{
namespace TonemappingRenderer
{
    void OnStart()
    {
        CGfxShadingRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxShadingRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxShadingRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxShadingRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxShadingRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxShadingRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxShadingRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxShadingRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxShadingRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxShadingRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxShadingRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxShadingRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxShadingRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxShadingRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxShadingRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxShadingRenderer::GetInstance().Render();
    }

    // -----------------------------------------------------------------------------

    void ResetSettings()
    {
        CGfxShadingRenderer::GetInstance().ResetSettings();
    }

    // -----------------------------------------------------------------------------

    void SetSettings(const SColorGradingSettings& _rSettings)
    {
        CGfxShadingRenderer::GetInstance().SetSettings(_rSettings);
    }

    // -----------------------------------------------------------------------------

    const SColorGradingSettings& GetSettings()
    {
        return CGfxShadingRenderer::GetInstance().GetSettings();
    }
} // namespace TonemappingRenderer
} // namespace Gfx
