
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_tonemapping_renderer.h"
#include "graphic/gfx_view_manager.h"

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

        void SetSettings();
        void SetSettings(const SColorGradingSettings& _rSettings);

        const SColorGradingSettings& GetSettings();

    private:

        struct SConstantBufferPS
        {
            Base::Float4 m_ColorMatrixR_ColorCurveCd1;
            Base::Float4 m_ColorMatrixG_ColorCurveCd3Cm3;
            Base::Float4 m_ColorMatrixB_ColorCurveCm2;
            Base::Float4 m_ColorCurve_Cm0Cd0_Cd2_Ch0Cm1_Ch3;
            Base::Float4 m_ColorCurve_Ch1_Ch2;
            Base::Float4 m_ColorShadow_Luma;
            Base::Float4 m_ColorShadow_Tint1;
            Base::Float4 m_ColorShadow_Tint2;
        };

    private:
        
        SConstantBufferPS      m_ConstantBufferPS;

        CMeshPtr               m_QuadModelPtr;
       
        CBufferPtr             m_TonemapBufferPtr;
        
        CInputLayoutPtr        m_QuadInputLayoutPtr;
        
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
        , m_QuadModelPtr          ()
        , m_TonemapBufferPtr      ()
        , m_QuadInputLayoutPtr    ()
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
        SetSettings();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::OnExit()
    {
        m_QuadModelPtr           = 0;
        m_TonemapBufferPtr       = 0;
        m_QuadInputLayoutPtr     = 0;
        m_FullquadShaderVSPtr    = 0;
        m_ShadingPSPtr           = 0;
        m_ShadingContextPtr      = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxShadingRenderer::OnSetupShader()
    {
        m_FullquadShaderVSPtr   = ShaderManager::CompileVS("vs_screen_p_quad.glsl" , "main");

        m_ShadingPSPtr          = ShaderManager::CompilePS("fs_tone_mapping.glsl", "main");

        // -----------------------------------------------------------------------------
        
        const SInputElementDescriptor QuadInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };
        
        m_QuadInputLayoutPtr       = ShaderManager::CreateInputLayout(QuadInputLayout, 1, m_FullquadShaderVSPtr);
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
        m_QuadModelPtr = MeshManager::CreateRectangle(0.0f, 0.0f, 1.0f, 1.0f);
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

        BufferManager::UploadConstantBufferData(m_TonemapBufferPtr, &ConstantBufferPS);

        // -----------------------------------------------------------------------------
        // Prepare renderer
        // -----------------------------------------------------------------------------
        const unsigned int pOffset[] = { 0, 0 };

        ContextManager::SetRenderContext(m_ShadingContextPtr);

        ContextManager::SetVertexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_QuadInputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_FullquadShaderVSPtr);

        ContextManager::SetShaderPS(m_ShadingPSPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_TonemapBufferPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

        ContextManager::SetTexture(0, TargetSetManager::GetLightAccumulationTargetSet()->GetRenderTarget(0));

        ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);

        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetTopology();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBuffer();

        ContextManager::ResetSampler(0);

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxShadingRenderer::SetSettings()
    {
        BASE_CONSOLE_STREAMINFO("Gfx> Loading default settings for final shading & tone mapping");
        
        SColorGradingSettings Settings;

        Settings.m_Tint             = Base::Float3(1.0f, 1.0f, 1.0f);
        Settings.m_DarkTint         = Base::Float3(0.2f, 0.2f, 0.2f);
        Settings.m_DarkTintBlend    = 0.5f;
        Settings.m_DarkTintStrength = 0.5f;
        Settings.m_Saturation       = 1.0f;
        Settings.m_Contrast         = 0.05f;
        Settings.m_DynamicRange     = 4.0f;
        Settings.m_CrushBrights     = 0.18f;
        Settings.m_CrushDarks       = 1.0f;
        Settings.m_Red              = Base::Float3(1.0f, 0.0f, 0.0f);
        Settings.m_Green            = Base::Float3(0.0f, 1.0f, 0.0f);
        Settings.m_Blue             = Base::Float3(0.0f, 0.0f, 1.0f);

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

	    Base::Float3 InWhitePoint = _rSettings.m_Tint;

	    float InSaturation = Base::Clamp(_rSettings.m_Saturation, 0.0f, 2.0f);

	    Base::Float3 InLuma(1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f);

	    Base::Float3 InMatrixR = _rSettings.m_Red;
	    Base::Float3 InMatrixG = _rSettings.m_Green;
	    Base::Float3 InMatrixB = _rSettings.m_Blue;

	    float InContrast = Base::Clamp(_rSettings.m_Contrast, 0.0f, 1.0f) + 1.0f;

	    float InDynamicRange = Base::Pow(2.0f, Base::Clamp(_rSettings.m_DynamicRange, 1.0f, 18.0f));

	    float InToe = (1.0f - Base::Clamp(_rSettings.m_CrushDarks, 0.0f, 1.0f)) * 0.18f;
	    InToe = Base::Clamp(InToe, 0.18f / 8.0f, 0.18f * (15.0f / 16.0f));

	    float InHeal = 1.0f - (Base::Max(1.0f / 32.0f, 1.0f - Base::Clamp(_rSettings.m_CrushBrights, 0.0f, 1.0f)) * (1.0f - 0.18f)); 

	    Base::Float3 InShadowTint = _rSettings.m_DarkTint;

	    float InShadowTintBlend = Base::Clamp(_rSettings.m_DarkTintBlend, 0.0f, 1.0f) * 64.0f;

        // -----------------------------------------------------------------------------
	    // Shadow tint amount enables turning off shadow tinting.
	    // -----------------------------------------------------------------------------
        float InShadowTintAmount = Base::Clamp(_rSettings.m_DarkTintStrength, 0.0f, 1.0f);

	    InShadowTint = InWhitePoint + (InShadowTint - InWhitePoint) * InShadowTintAmount;

        // -----------------------------------------------------------------------------
	    // Make sure channel mixer inputs sum to 1 (+ smart dealing with all zeros).
	    // -----------------------------------------------------------------------------
        InMatrixR[0] += 1.0f / (256.0f * 256.0f * 32.0f);
	    InMatrixG[1] += 1.0f / (256.0f * 256.0f * 32.0f);
	    InMatrixB[2] += 1.0f / (256.0f * 256.0f * 32.0f);

	    InMatrixR *= 1.0f / InMatrixR.DotProduct(Base::Float3::s_One);
	    InMatrixG *= 1.0f / InMatrixG.DotProduct(Base::Float3::s_One);
	    InMatrixB *= 1.0f / InMatrixB.DotProduct(Base::Float3::s_One);

        // -----------------------------------------------------------------------------
	    // Conversion from linear rgb to luma (using HDTV coef).
	    // -----------------------------------------------------------------------------
        Base::Float3 LumaWeights(0.2126f, 0.7152f, 0.0722f);

        // -----------------------------------------------------------------------------
	    // Make sure white point has 1.0 as luma (so adjusting white point doesn't 
	    // change exposure).
	    // Make sure {0.0,0.0,0.0} inputs do something sane (default to white).
	    // -----------------------------------------------------------------------------
	    InWhitePoint += Base::Float3(1.0f / (256.0f * 256.0f * 32.0f));
	    InWhitePoint *= 1.0f / InWhitePoint.DotProduct(LumaWeights);
	    InShadowTint += Base::Float3(1.0f / (256.0f * 256.0f * 32.0f));
	    InShadowTint *= 1.0f / InShadowTint.DotProduct(LumaWeights);

	    // Grey after color matrix is applied.
	    Base::Float3 ColorMatrixLuma(0.0f);
        
	    ColorMatrixLuma[0] = Base::Float3::s_One.DotProduct(Base::Float3(InMatrixR[0], InMatrixG[0], InMatrixB[0]) * InLuma[0]);
	    ColorMatrixLuma[1] = Base::Float3::s_One.DotProduct(Base::Float3(InMatrixR[1], InMatrixG[1], InMatrixB[1]) * InLuma[1]);
	    ColorMatrixLuma[2] = Base::Float3::s_One.DotProduct(Base::Float3(InMatrixR[2], InMatrixG[2], InMatrixB[2]) * InLuma[2]);

	    Base::Float3 OutMatrixR(0.0f);
	    Base::Float3 OutMatrixG(0.0f);
	    Base::Float3 OutMatrixB(0.0f);

	    Base::Float3 OutColorShadow_Luma = LumaWeights * InShadowTintBlend;
	    Base::Float3 OutColorShadow_Tint1 = InWhitePoint;
	    Base::Float3 OutColorShadow_Tint2 = InShadowTint - InWhitePoint;

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

        m_ConstantBufferPS.m_ColorMatrixR_ColorCurveCd1      .Set(OutMatrixR[0], OutMatrixR[1], OutMatrixR[2], OutColorCurveCd1);
        m_ConstantBufferPS.m_ColorMatrixG_ColorCurveCd3Cm3   .Set(OutMatrixG[0], OutMatrixG[1], OutMatrixG[2], OutColorCurveCd3Cm3); 
        m_ConstantBufferPS.m_ColorMatrixB_ColorCurveCm2      .Set(OutMatrixB[0], OutMatrixB[1], OutMatrixB[2], OutColorCurveCm2); 
        m_ConstantBufferPS.m_ColorCurve_Cm0Cd0_Cd2_Ch0Cm1_Ch3.Set(OutColorCurveCm0Cd0, OutColorCurveCd2, OutColorCurveCh0Cm1, OutColorCurveCh3); 
        m_ConstantBufferPS.m_ColorCurve_Ch1_Ch2              .Set(OutColorCurveCh1, OutColorCurveCh2, 0.0f, 0.0f); 
        m_ConstantBufferPS.m_ColorShadow_Luma                .Set(OutColorShadow_Luma [0], OutColorShadow_Luma [1], OutColorShadow_Luma [2], 0.0f);
        m_ConstantBufferPS.m_ColorShadow_Tint1               .Set(OutColorShadow_Tint1[0], OutColorShadow_Tint1[1], OutColorShadow_Tint1[2], 0.0f);
        m_ConstantBufferPS.m_ColorShadow_Tint2               .Set(OutColorShadow_Tint2[0], OutColorShadow_Tint2[1], OutColorShadow_Tint2[2], 0.0f);
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

    void SetSettings()
    {
        CGfxShadingRenderer::GetInstance().SetSettings();
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
