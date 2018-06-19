
#include "engine/engine_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"
#include "engine/core/core_time.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_debug_renderer.h"
#include "engine/graphic/gfx_histogram_renderer.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

using namespace Gfx;

namespace
{
    class CGfxHistogramRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxHistogramRenderer)
        
    public:
        CGfxHistogramRenderer();
        ~CGfxHistogramRenderer();
        
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
        void SetSettings(const SHistogramSettings& _rSettings);
        const SHistogramSettings& GetSettings();
        
        CBufferPtr GetExposureHistoryBuffer();

        unsigned int GetCurrentExposureHistoryIndex();

        unsigned int GetLastExposureHistoryIndex();
        
        void ResetEyeAdaption();

    private:

        static const unsigned int s_HistogramSize                = 64;
        static const unsigned int s_HistogramTileSize            = 8;
        static const unsigned int s_HistogramHistorySize         = 8;
        static const unsigned int s_HistogramThreadGroupSizeX    = 8;
        static const unsigned int s_HistogramThreadGroupSizeY    = 8;
        static const unsigned int s_NumberOfPixelsPerThreadGroup = (s_HistogramThreadGroupSizeX * s_HistogramTileSize) * (s_HistogramThreadGroupSizeY * s_HistogramTileSize);
        static const unsigned int s_MaxNumberOfThreadGroups      = (3840 * 2160) / s_NumberOfPixelsPerThreadGroup + 1;

    private:

        enum EHistogramPass
        {
            HistogramBuild,
            HistogramMerge,
            HistogramEvaluate,
            NumberOfHistogramPasses,
            UndefinedHistogramPass = -1
        };
        
    private:

        struct SConstantBufferCS
        {
            unsigned int m_MinPixelCoordX;
            unsigned int m_MinPixelCoordY;
            unsigned int m_MaxPixelCoordX;
            unsigned int m_MaxPixelCoordY;
            unsigned int m_NumberOfThreadGroupsX;
            unsigned int m_NumberOfThreadGroupsY;
            unsigned int m_HistoryIndex;
            unsigned int m_LastHistoryIndex;
            float        m_ReciprocalNumberOfPixels;
            float        m_Time;
            float        m_HistogramLowerBound;
            float        m_HistogramUpperBound;
            float        m_HistogramLogMin;
            float        m_HistogramLogMax;
            float        m_HistogramScale;
            float        m_HistogramReciprocalScale;
            float        m_EyeAdaptionSpeedUp;
            float        m_EyeAdaptionSpeedDown;
            float        m_ExposureCompensation;
            float        m_UseAutoExposure;
            float        m_Aperture;
            float        m_Shuttertime;
            float        m_ISO;
            float        m_ResetEyeAdaption;
        };

        struct SExposureHistoryBuffer
        {
            float m_ExposureHistory[s_HistogramHistorySize];
        };
        
    private:

        SHistogramSettings     m_Settings;

        SConstantBufferCS      m_ConstantBufferPS;

        CShaderPtr             m_HistogramShaderPtrs[NumberOfHistogramPasses];

        CBufferPtr             m_ExposureHistoryBufferPtr;

        CBufferPtr             m_HistogramSettingsBufferPtr;

        CBufferPtr             m_HistogramPerThreadGroupBufferPtr;

        CBufferPtr             m_HistogramBufferPtr;

        unsigned int           m_HistoryIndex;

        unsigned int           m_LastHistoryIndex;
        
        unsigned int           m_FrameOnResetEyeAdaption;
    };
} // namespace

namespace
{
    CGfxHistogramRenderer::CGfxHistogramRenderer()
        : m_Settings                         ()
        , m_ConstantBufferPS                 ()
        , m_HistogramShaderPtrs              ()
        , m_HistogramSettingsBufferPtr       ()
        , m_HistogramPerThreadGroupBufferPtr ()
        , m_ExposureHistoryBufferPtr         ()
        , m_HistogramBufferPtr               ()
        , m_HistoryIndex                     (0)
        , m_LastHistoryIndex                 (s_HistogramHistorySize - 1)
        , m_FrameOnResetEyeAdaption          (0)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxHistogramRenderer::~CGfxHistogramRenderer()
    {
    
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHistogramRenderer::OnStart()
    {
        ResetSettings();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHistogramRenderer::OnExit()
    {
        m_HistogramShaderPtrs[HistogramBuild]    = 0;
        m_HistogramShaderPtrs[HistogramMerge]    = 0;
        m_HistogramShaderPtrs[HistogramEvaluate] = 0;

        m_HistogramSettingsBufferPtr       = 0;
        m_HistogramPerThreadGroupBufferPtr = 0;
        m_HistogramBufferPtr               = 0;
        m_ExposureHistoryBufferPtr         = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHistogramRenderer::OnSetupShader()
    {
        m_HistogramShaderPtrs[HistogramBuild]    = ShaderManager::CompileCS("histogram/cs_histogram_build.glsl"   , "main");
        m_HistogramShaderPtrs[HistogramMerge]    = ShaderManager::CompileCS("histogram/cs_histogram_merge.glsl"   , "main");
        m_HistogramShaderPtrs[HistogramEvaluate] = ShaderManager::CompileCS("histogram/cs_histogram_evaluate.glsl", "main");
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHistogramRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHistogramRenderer::OnSetupRenderTargets()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHistogramRenderer::OnSetupStates()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHistogramRenderer::OnSetupTextures()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHistogramRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstanteBufferDesc;

        // -----------------------------------------------------------------------------
        
        SExposureHistoryBuffer DefaultHistoryData;
        
        DefaultHistoryData.m_ExposureHistory[0] = 1.0f;
        DefaultHistoryData.m_ExposureHistory[1] = 1.0f;
        DefaultHistoryData.m_ExposureHistory[2] = 1.0f;
        DefaultHistoryData.m_ExposureHistory[3] = 1.0f;
        DefaultHistoryData.m_ExposureHistory[4] = 1.0f;
        DefaultHistoryData.m_ExposureHistory[5] = 1.0f;
        DefaultHistoryData.m_ExposureHistory[6] = 1.0f;
        DefaultHistoryData.m_ExposureHistory[7] = 1.0f;
       
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SExposureHistoryBuffer);
        ConstanteBufferDesc.m_pBytes        = &DefaultHistoryData;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_ExposureHistoryBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        BufferManager::SetBufferLabel(m_ExposureHistoryBufferPtr, "Histogram Exposure History");

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SConstantBufferCS);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_HistogramSettingsBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        BufferManager::SetBufferLabel(m_HistogramSettingsBufferPtr, "Histogram Settings");

        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = s_MaxNumberOfThreadGroups * s_HistogramSize * sizeof(float);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_HistogramPerThreadGroupBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        BufferManager::SetBufferLabel(m_HistogramPerThreadGroupBufferPtr, "Histogram Per Thread Group");

        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ResourceBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = s_HistogramSize * sizeof(float);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_HistogramBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        BufferManager::SetBufferLabel(m_HistogramBufferPtr, "Histogram");
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHistogramRenderer::OnSetupResources()
    {        
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHistogramRenderer::OnSetupModels()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHistogramRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHistogramRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHistogramRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHistogramRenderer::OnUnloadMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHistogramRenderer::Update()
    {
        unsigned int NumberOfThreadGroupsX;
        unsigned int NumberOfThreadGroupsY;
        unsigned int Width;
        unsigned int Height;
        unsigned int FrameCounter;
        float        TimeSinceLastFrame;
        CCameraPtr   MainCameraPtr;

        TimeSinceLastFrame = static_cast<float>(Core::Time::GetDeltaTimeLastFrame());
        MainCameraPtr      = ViewManager::GetMainCamera();
        Width              = Main::GetActiveWindowSize()[0];
        Height             = Main::GetActiveWindowSize()[1];
        FrameCounter       = static_cast<unsigned int>(Core::Time::GetNumberOfFrame());

        NumberOfThreadGroupsX = (Width  + s_HistogramTileSize * s_HistogramThreadGroupSizeX - 1) / (s_HistogramTileSize * s_HistogramThreadGroupSizeX);
        NumberOfThreadGroupsY = (Height + s_HistogramTileSize * s_HistogramThreadGroupSizeY - 1) / (s_HistogramTileSize * s_HistogramThreadGroupSizeY);
        
        m_ConstantBufferPS.m_NumberOfThreadGroupsX    = NumberOfThreadGroupsX;
        m_ConstantBufferPS.m_NumberOfThreadGroupsY    = NumberOfThreadGroupsY;
        m_ConstantBufferPS.m_MinPixelCoordX           = 0;
        m_ConstantBufferPS.m_MinPixelCoordY           = 0;
        m_ConstantBufferPS.m_MaxPixelCoordX           = Width;
        m_ConstantBufferPS.m_MaxPixelCoordY           = Height;
        m_ConstantBufferPS.m_ReciprocalNumberOfPixels = 1.0f / static_cast<float>(Width * Height);
        m_ConstantBufferPS.m_HistoryIndex             = m_HistoryIndex;
        m_ConstantBufferPS.m_LastHistoryIndex         = m_LastHistoryIndex;
        m_ConstantBufferPS.m_Time                     = TimeSinceLastFrame;
        m_ConstantBufferPS.m_ExposureCompensation     = MainCameraPtr->GetEC();
        m_ConstantBufferPS.m_UseAutoExposure          = MainCameraPtr->GetCameraMode() == CCamera::Auto ? 1.0f : 0.0f;
        m_ConstantBufferPS.m_Aperture                 = MainCameraPtr->GetAperture();
        m_ConstantBufferPS.m_Shuttertime              = MainCameraPtr->GetShutterSpeed();
        m_ConstantBufferPS.m_ISO                      = MainCameraPtr->GetISO();
        
        if (m_FrameOnResetEyeAdaption + s_HistogramHistorySize ==  FrameCounter)
        {
            m_ConstantBufferPS.m_ResetEyeAdaption = 0.0f;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHistogramRenderer::Render()
    {
        Performance::BeginEvent("Histogram");

        // -----------------------------------------------------------------------------
        // Upload data to buffer
        // -----------------------------------------------------------------------------
        SConstantBufferCS HistogramSettings;

        HistogramSettings.m_MinPixelCoordX           = m_ConstantBufferPS.m_MinPixelCoordX;
        HistogramSettings.m_MinPixelCoordY           = m_ConstantBufferPS.m_MinPixelCoordY;
        HistogramSettings.m_MaxPixelCoordX           = m_ConstantBufferPS.m_MaxPixelCoordX;
        HistogramSettings.m_MaxPixelCoordY           = m_ConstantBufferPS.m_MaxPixelCoordY;
        HistogramSettings.m_NumberOfThreadGroupsX    = m_ConstantBufferPS.m_NumberOfThreadGroupsX;
        HistogramSettings.m_NumberOfThreadGroupsY    = m_ConstantBufferPS.m_NumberOfThreadGroupsY;
        HistogramSettings.m_ReciprocalNumberOfPixels = m_ConstantBufferPS.m_ReciprocalNumberOfPixels;
        HistogramSettings.m_HistoryIndex             = m_ConstantBufferPS.m_HistoryIndex;
        HistogramSettings.m_LastHistoryIndex         = m_ConstantBufferPS.m_LastHistoryIndex;
        HistogramSettings.m_Time                     = m_ConstantBufferPS.m_Time;
        HistogramSettings.m_HistogramLowerBound      = m_ConstantBufferPS.m_HistogramLowerBound;
        HistogramSettings.m_HistogramUpperBound      = m_ConstantBufferPS.m_HistogramUpperBound;
        HistogramSettings.m_HistogramLogMin          = m_ConstantBufferPS.m_HistogramLogMin;
        HistogramSettings.m_HistogramLogMax          = m_ConstantBufferPS.m_HistogramLogMax;
        HistogramSettings.m_HistogramScale           = m_ConstantBufferPS.m_HistogramScale;
        HistogramSettings.m_HistogramReciprocalScale = m_ConstantBufferPS.m_HistogramReciprocalScale;
        HistogramSettings.m_EyeAdaptionSpeedUp       = m_ConstantBufferPS.m_EyeAdaptionSpeedUp   * 20.0f;
        HistogramSettings.m_EyeAdaptionSpeedDown     = m_ConstantBufferPS.m_EyeAdaptionSpeedDown * 20.0f;
        HistogramSettings.m_ExposureCompensation     = m_ConstantBufferPS.m_ExposureCompensation;
        HistogramSettings.m_UseAutoExposure          = m_ConstantBufferPS.m_UseAutoExposure;
        HistogramSettings.m_Aperture                 = m_ConstantBufferPS.m_Aperture;           
        HistogramSettings.m_Shuttertime              = m_ConstantBufferPS.m_Shuttertime;
        HistogramSettings.m_ISO                      = m_ConstantBufferPS.m_ISO;
        HistogramSettings.m_ResetEyeAdaption         = m_ConstantBufferPS.m_ResetEyeAdaption;
        
        BufferManager::UploadBufferData(m_HistogramSettingsBufferPtr, &HistogramSettings);

        // -----------------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------------
        ContextManager::SetResourceBuffer(0, m_HistogramPerThreadGroupBufferPtr);
        ContextManager::SetResourceBuffer(1, m_HistogramBufferPtr);
        ContextManager::SetResourceBuffer(2, m_ExposureHistoryBufferPtr);

        ContextManager::SetConstantBuffer(0, m_HistogramSettingsBufferPtr);

        ContextManager::SetImageTexture(0, TargetSetManager::GetLightAccumulationTargetSet()->GetRenderTarget(0));

        // -----------------------------------------------------------------------------
        // 1. pass: Build partial histograms
        // -----------------------------------------------------------------------------
        {
            ContextManager::SetShaderCS(m_HistogramShaderPtrs[HistogramBuild]);

            ContextManager::Dispatch(HistogramSettings.m_NumberOfThreadGroupsX, HistogramSettings.m_NumberOfThreadGroupsY, 1);
        }
        
        // -----------------------------------------------------------------------------
        // 2. pass: Merge histograms into one final histogram
        // -----------------------------------------------------------------------------
        {
            ContextManager::SetShaderCS(m_HistogramShaderPtrs[HistogramMerge]);
            
            ContextManager::Dispatch(s_HistogramSize, 1, 1);
        }
        
        // -----------------------------------------------------------------------------
        // Eye adapted exposure calculation
        // -----------------------------------------------------------------------------
        {
            ContextManager::SetShaderCS(m_HistogramShaderPtrs[HistogramEvaluate]);
            
            ContextManager::Dispatch(1, 1, 1);
        }

        ContextManager::ResetImageTexture(0);

        ContextManager::ResetConstantBuffer(0);

        ContextManager::ResetResourceBuffer(0);
        ContextManager::ResetResourceBuffer(1);
        ContextManager::ResetResourceBuffer(2);

        ContextManager::ResetShaderCS();

        // -----------------------------------------------------------------------------

        m_LastHistoryIndex = m_HistoryIndex;
        m_HistoryIndex     = (m_HistoryIndex + 1) & (s_HistogramHistorySize - 1);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxHistogramRenderer::ResetSettings()
    {
        ENGINE_CONSOLE_STREAMINFO("Gfx> Loading default settings for histogram");
        
        SHistogramSettings Settings;

        glm::vec2 Bounds = Core::CProgramParameters::GetInstance().Get("graphics:histogram:bounds", glm::vec2(0.80f, 0.983f));
        Settings.m_HistogramLowerBound  = Bounds.x;
        Settings.m_HistogramUpperBound  = Bounds.y;

        glm::vec2 LogRange = Core::CProgramParameters::GetInstance().Get("graphics:histogram:log_range", glm::vec2(-8.00f, 12.00f));
        Settings.m_HistogramLogMin      = LogRange.x;
        Settings.m_HistogramLogMax      = LogRange.y;

        glm::vec2 EyeAdaptionSpeed = Core::CProgramParameters::GetInstance().Get("graphics:histogram:eye_adaption_speeds", glm::vec2(0.25f, 0.25f));
        Settings.m_EyeAdaptionSpeedUp   = EyeAdaptionSpeed.x;
        Settings.m_EyeAdaptionSpeedDown = EyeAdaptionSpeed.y;
        Settings.m_ResetEyeAdaption     = true;

        SetSettings(Settings);
    }

    // -----------------------------------------------------------------------------

    void CGfxHistogramRenderer::SetSettings(const SHistogramSettings& _rSettings)
    {
        m_Settings = _rSettings;

        m_ConstantBufferPS.m_HistogramLowerBound      =  _rSettings.m_HistogramLowerBound;
        m_ConstantBufferPS.m_HistogramUpperBound      =  _rSettings.m_HistogramUpperBound;
        m_ConstantBufferPS.m_HistogramLogMin          =  _rSettings.m_HistogramLogMin;
        m_ConstantBufferPS.m_HistogramLogMax          =  _rSettings.m_HistogramLogMax;
        m_ConstantBufferPS.m_HistogramScale           =  static_cast<float>(s_HistogramSize - 1) / (m_ConstantBufferPS.m_HistogramLogMax - m_ConstantBufferPS.m_HistogramLogMin);
        m_ConstantBufferPS.m_HistogramReciprocalScale =  1.0f / m_ConstantBufferPS.m_HistogramScale;
        m_ConstantBufferPS.m_EyeAdaptionSpeedUp       =  _rSettings.m_EyeAdaptionSpeedUp;
        m_ConstantBufferPS.m_EyeAdaptionSpeedDown     =  _rSettings.m_EyeAdaptionSpeedDown;
        m_ConstantBufferPS.m_ResetEyeAdaption         =  _rSettings.m_ResetEyeAdaption == true ? 1.0f : 0.0f;
        
        if (_rSettings.m_ResetEyeAdaption == true)
        {
            ResetEyeAdaption();
        }
    }

    // -----------------------------------------------------------------------------

    const SHistogramSettings& CGfxHistogramRenderer::GetSettings()
    {
        return m_Settings;
    }

    // -----------------------------------------------------------------------------

    CBufferPtr CGfxHistogramRenderer::GetExposureHistoryBuffer()
    {
        return m_ExposureHistoryBufferPtr;
    }

    // -----------------------------------------------------------------------------

    unsigned int CGfxHistogramRenderer::GetCurrentExposureHistoryIndex()
    {
        return m_HistoryIndex;
    }

    // -----------------------------------------------------------------------------

    unsigned int CGfxHistogramRenderer::GetLastExposureHistoryIndex()
    {
        return m_LastHistoryIndex;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxHistogramRenderer::ResetEyeAdaption()
    {
        m_FrameOnResetEyeAdaption = static_cast<unsigned int>(Core::Time::GetNumberOfFrame());
    }
} // namespace

namespace Gfx
{
namespace HistogramRenderer
{
    void OnStart()
    {
        CGfxHistogramRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxHistogramRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxHistogramRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxHistogramRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxHistogramRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxHistogramRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxHistogramRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxHistogramRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxHistogramRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxHistogramRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxHistogramRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxHistogramRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxHistogramRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxHistogramRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxHistogramRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxHistogramRenderer::GetInstance().Render();
    }

    // -----------------------------------------------------------------------------

    void ResetSettings()
    {
        CGfxHistogramRenderer::GetInstance().ResetSettings();
    }

    // -----------------------------------------------------------------------------

    void SetSettings(const SHistogramSettings& _rSettings)
    {
        CGfxHistogramRenderer::GetInstance().SetSettings(_rSettings);
    }

    // -----------------------------------------------------------------------------

    const SHistogramSettings& GetSettings()
    {
        return CGfxHistogramRenderer::GetInstance().GetSettings();
    }

    // -----------------------------------------------------------------------------

    CBufferPtr GetExposureHistoryBuffer()
    {
        return CGfxHistogramRenderer::GetInstance().GetExposureHistoryBuffer();
    }

    // -----------------------------------------------------------------------------

    unsigned int GetCurrentExposureHistoryIndex()
    {
        return CGfxHistogramRenderer::GetInstance().GetCurrentExposureHistoryIndex();
    }

    // -----------------------------------------------------------------------------

    unsigned int GetLastExposureHistoryIndex()
    {
        return CGfxHistogramRenderer::GetInstance().GetLastExposureHistoryIndex();
    }
    
    // -----------------------------------------------------------------------------
    
    void ResetEyeAdaption()
    {
        CGfxHistogramRenderer::GetInstance().ResetEyeAdaption();
    }
} // namespace HistogramRenderer
} // namespace Gfx
