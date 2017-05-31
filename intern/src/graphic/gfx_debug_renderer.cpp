
#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_entity.h"
#include "data/data_light_type.h"
#include "data/data_map.h"
#include "data/data_model_manager.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_debug_renderer.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

using namespace Gfx;

// -----------------------------------------------------------------------------
// This debug renderer helps by rendering different objects inside the screen
// like a view frustum of camera or a coord system. Moreover we can render 
// texture symbols for camera, light and other necessary informations.
// -----------------------------------------------------------------------------

namespace
{
    class CGfxDebugRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxDebugRenderer)
        
    public:
        
        CGfxDebugRenderer();
        ~CGfxDebugRenderer();
        
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
        void OnResize(unsigned int _Width, unsigned int _Height);
        
        void Update();
        void Render();

        void DrawCamera(CCameraPtr _CameraPtr);
        void DrawGizmo(bool _Flag);
        void DrawTexture(CTextureBasePtr _TexturePtr, const Base::AABB2Float& _rScreenRegion);
        void DrawText(const std::string& _rText, const Base::Float2& _rScreenPosition, const Base::Float4& _rColor, unsigned int m_TextSize);
        
    private:
        
        struct SPerFrameConstantBuffer
        {
            Base::Float4x4 m_ViewProjection;
            Base::Float4x4 m_ModelMatrix;
        };
        
        struct SPerDrawCallConstantBuffer
        {
            Base::Float4x4 m_ModelMatrix;
        };

        struct SPerTextInstanceBuffer
        {
            Base::Float3 m_TextSettings;
            Base::Float3 m_CharSettings;
            Base::Float4 m_CharColor;
        };
        
        struct SProperties
        {
            Base::Float4 m_Color;
        };

        struct SDebugCamera
        {
            CCameraPtr m_CameraPtr;
        };

        struct SDebugTexture
        {
            CTextureSetPtr   m_TexturePtr;
            Base::AABB2Float m_ScreenRegion;
        };

        struct SDebugText
        {
            std::string  m_Text;
            Base::Float2 m_ScreenPosition;
            Base::Float4 m_Color;
            unsigned int m_TextSize;
        };

    private:

        static const unsigned int s_MaxNumberOfInstances = 512 * 512;

    private:

        typedef std::vector<SDebugCamera>  CDebugCameras;
        typedef std::vector<SDebugTexture> CDebugTextures;
        typedef std::vector<SDebugText>    CDebugTexts;
        
    private:
        
        CMeshPtr m_QuadModelPtr;
        CMeshPtr m_GizmoModelPtr;

        CBufferSetPtr     m_ViewModelVSBuffer;
        CBufferSetPtr     m_BaseModelVSBuffer;
        CBufferSetPtr     m_ViewPSBuffer;
        CBufferSetPtr     m_PlaneBuffer;
        CBufferSetPtr     m_DeferredPassPSBuffer;
        CBufferPtr        m_TextInstanceBufferPtr;
        CBufferSetPtr     m_TextInstanceBufferSetPtr;
        CBufferPtr        m_PlaneIndexBuffer;
        CRenderContextPtr m_RenderContextPtr;
        CShaderPtr        m_PositionShaderVSPtr;
        CShaderPtr        m_PositionShaderPSPtr;
        CShaderPtr        m_GizmoShaderVSPtr;
        CShaderPtr        m_GizmoShaderPSPtr;
        CShaderPtr        m_QuadTextureShaderVSPtr;
        CShaderPtr        m_QuadTextureShaderPSPtr;
        CShaderPtr        m_QuadTextShaderVSPtr;
        CShaderPtr        m_QuadTextShaderPSPtr;
        CTextureBasePtr   m_ConsolasTexturePtr;

        CDebugCameras  m_DebugCameras;
        CDebugTextures m_DebugTextures;
        CDebugTexts    m_DebugTexts;
        
        bool m_IsGizmoVisible;
        
    private:
    
        void RenderCameras();
        void RenderGizmo();
        void RenderTextures();
        void RenderTexts();
    };
} // namespace

namespace
{
    CGfxDebugRenderer::CGfxDebugRenderer()
        : m_QuadModelPtr            ()
        , m_GizmoModelPtr           ()
        , m_ViewModelVSBuffer       ()
        , m_BaseModelVSBuffer       ()
        , m_ViewPSBuffer            ()
        , m_PlaneBuffer             ()
        , m_DeferredPassPSBuffer    ()
        , m_TextInstanceBufferPtr   ()
        , m_TextInstanceBufferSetPtr()
        , m_PlaneIndexBuffer        ()
        , m_RenderContextPtr        ()
        , m_PositionShaderVSPtr     ()
        , m_PositionShaderPSPtr     ()
        , m_GizmoShaderVSPtr        ()
        , m_GizmoShaderPSPtr        ()
        , m_QuadTextureShaderVSPtr  ()
        , m_QuadTextureShaderPSPtr  ()
        , m_QuadTextShaderVSPtr     ()
        , m_QuadTextShaderPSPtr     ()
        , m_ConsolasTexturePtr      ()
        , m_DebugTextures           ()
        , m_DebugCameras            ()
        , m_DebugTexts              ()
        , m_IsGizmoVisible          (false)
    {
        // -----------------------------------------------------------------------------
        // Register resize delegate
        // -----------------------------------------------------------------------------
        Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxDebugRenderer::OnResize));
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxDebugRenderer::~CGfxDebugRenderer()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugRenderer::OnStart()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugRenderer::OnExit()
    {
        m_QuadModelPtr             = 0;
        m_GizmoModelPtr            = 0;
        m_ViewModelVSBuffer        = 0;
        m_BaseModelVSBuffer        = 0;
        m_ViewPSBuffer             = 0;
        m_PlaneBuffer              = 0;
        m_DeferredPassPSBuffer     = 0;
        m_TextInstanceBufferPtr    = 0;
        m_TextInstanceBufferSetPtr = 0;
        m_PlaneIndexBuffer         = 0;
        m_RenderContextPtr         = 0;
        m_PositionShaderVSPtr      = 0;
        m_PositionShaderPSPtr      = 0;
        m_GizmoShaderVSPtr         = 0;
        m_GizmoShaderPSPtr         = 0;
        m_QuadTextureShaderVSPtr   = 0;
        m_QuadTextureShaderPSPtr   = 0;
        m_QuadTextShaderVSPtr      = 0;
        m_QuadTextShaderPSPtr      = 0;
        m_ConsolasTexturePtr       = 0;

        m_DebugCameras .clear();
        m_DebugTextures.clear();
        m_DebugTexts   .clear();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugRenderer::OnSetupShader()
    {
        CShaderPtr ShaderVSPtr;
        CShaderPtr ShaderPSPtr;
        
        const SInputElementDescriptor PositionColorInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float3Format, 0, 0 , 24, CInputLayout::PerVertex, 0, },
            { "COLOR"   , 0, CInputLayout::Float3Format, 0, 12, 24, CInputLayout::PerVertex, 0, },
        };
        
        const SInputElementDescriptor PositionInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float3Format, 0, 0, 12, CInputLayout::PerVertex, 0, },
        };

        const SInputElementDescriptor QuadPositionInputLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0, },
        };

        const SInputElementDescriptor TextOffsetInputLayout[] =
        {
            { "POSITION" , 0, CInputLayout::Float2Format, 0,  0,  8, CInputLayout::PerVertex  , 0, },
            { "TEXTSETS" , 0, CInputLayout::Float3Format, 1,  0, 40, CInputLayout::PerInstance, 1, },
            { "CHARSETS" , 0, CInputLayout::Float3Format, 1, 12, 40, CInputLayout::PerInstance, 1, },
            { "CHARCOLOR", 0, CInputLayout::Float4Format, 1, 24, 40, CInputLayout::PerInstance, 1, },
        };
        
        // -----------------------------------------------------------------------------
        // Position
        // -----------------------------------------------------------------------------
        ShaderVSPtr = ShaderManager::CompileVS("vs_non_p.glsl", "main");
        
        ShaderPSPtr = ShaderManager::CompilePS("fs_color_alpha.glsl", "main");
        
        ShaderManager::CreateInputLayout(PositionInputLayout, 1, ShaderVSPtr);
        
        m_PositionShaderVSPtr = ShaderVSPtr;
        m_PositionShaderPSPtr = ShaderPSPtr;
        
        // -----------------------------------------------------------------------------
        // Position + Color
        // -----------------------------------------------------------------------------
        ShaderVSPtr = ShaderManager::CompileVS("vs_vm_p.glsl", "main");
        
        ShaderPSPtr = ShaderManager::CompilePS("fs_color.glsl", "main");
        
        ShaderManager::CreateInputLayout(PositionColorInputLayout, 2, ShaderVSPtr);
        
        m_GizmoShaderVSPtr = ShaderVSPtr;
        m_GizmoShaderPSPtr = ShaderPSPtr;

        // -----------------------------------------------------------------------------
        // Quad + Texture
        // -----------------------------------------------------------------------------
        ShaderVSPtr = ShaderManager::CompileVS("vs_m_p_quad.glsl", "main");

        ShaderPSPtr = ShaderManager::CompilePS("fs_texture.glsl", "main");

        ShaderManager::CreateInputLayout(QuadPositionInputLayout, 1, ShaderVSPtr);

        m_QuadTextureShaderVSPtr = ShaderVSPtr;
        m_QuadTextureShaderPSPtr = ShaderPSPtr;

        // -----------------------------------------------------------------------------
        // Quad (Instanced) + Text
        // -----------------------------------------------------------------------------
        ShaderVSPtr = ShaderManager::CompileVS("vs_text.glsl", "main");
        
        ShaderPSPtr = ShaderManager::CompilePS("fs_text.glsl", "main");

        ShaderManager::CreateInputLayout(TextOffsetInputLayout, 4, ShaderVSPtr);

        m_QuadTextShaderVSPtr = ShaderVSPtr;
        m_QuadTextShaderPSPtr = ShaderPSPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugRenderer::OnSetupKernels()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugRenderer::OnSetupRenderTargets()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugRenderer::OnSetupStates()
    {
        CCameraPtr          CameraPtr        = ViewManager     ::GetMainCamera ();
        CViewPortSetPtr     ViewPortSetPtr   = ViewManager     ::GetViewPortSet();
        CRenderStatePtr     RenderStatePtr   = StateManager    ::GetRenderState(CRenderState::AlphaBlend | CRenderState::NoCull);
        CTargetSetPtr       TargetSetPtr     = TargetSetManager::GetDefaultTargetSet();
        
        CRenderContextPtr RenderContextPtr = ContextManager::CreateRenderContext();
        
        RenderContextPtr->SetCamera(CameraPtr);
        RenderContextPtr->SetViewPortSet(ViewPortSetPtr);
        RenderContextPtr->SetTargetSet(TargetSetPtr);
        RenderContextPtr->SetRenderState(RenderStatePtr);
        
        m_RenderContextPtr = RenderContextPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugRenderer::OnSetupTextures()
    {
        STextureDescriptor TextTextureDescriptor;

        TextTextureDescriptor.m_NumberOfPixelsU  = 1024;
        TextTextureDescriptor.m_NumberOfPixelsV  = 1024;
        TextTextureDescriptor.m_NumberOfPixelsW  = 1;
        TextTextureDescriptor.m_NumberOfMipMaps  = 1;
        TextTextureDescriptor.m_NumberOfTextures = 1;
        TextTextureDescriptor.m_Binding          = CTextureBase::ShaderResource;
        TextTextureDescriptor.m_Access           = CTextureBase::CPUWrite;
        TextTextureDescriptor.m_Format           = CTextureBase::Unknown;
        TextTextureDescriptor.m_Usage            = CTextureBase::GPURead;
        TextTextureDescriptor.m_Semantic         = CTextureBase::Diffuse;
        TextTextureDescriptor.m_pFileName        = "Consolas.tga";
        TextTextureDescriptor.m_pPixels          = 0;
        TextTextureDescriptor.m_Format           = CTextureBase::R8G8B8_UBYTE;

        m_ConsolasTexturePtr = static_cast<CTextureBasePtr>(TextureManager::CreateTexture2D(TextTextureDescriptor));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstanteBufferDesc;
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerFrameConstantBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr ViewBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerDrawCallConstantBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr ModelBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        m_ViewModelVSBuffer = BufferManager::CreateBufferSet(ViewBuffer);
        m_BaseModelVSBuffer = BufferManager::CreateBufferSet(ModelBuffer);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(CMaterial::SMaterialAttributes);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        CBufferPtr MaterialBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);

        m_DeferredPassPSBuffer = BufferManager::CreateBufferSet(MaterialBufferPtr);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SProperties);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        CBufferPtr PropertiesBuffer = BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        m_ViewPSBuffer = BufferManager::CreateBufferSet(PropertiesBuffer);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = CBuffer::GPUReadWrite;
        ConstanteBufferDesc.m_Binding       = CBuffer::VertexBuffer;
        ConstanteBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SPerTextInstanceBuffer) * s_MaxNumberOfInstances;
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;

        m_TextInstanceBufferPtr = BufferManager::CreateBuffer(ConstanteBufferDesc);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugRenderer::OnSetupModels()
    {
        SMeshDescriptor  ModelDesc;
        SBufferDescriptor BufferDesc;

        m_QuadModelPtr = MeshManager::CreateRectangle(0.0f, 0.0f, 1.0f, 1.0f);

        m_TextInstanceBufferSetPtr = BufferManager::CreateVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer()->GetBuffer(0), m_TextInstanceBufferPtr);

        // -----------------------------------------------------------------------------

        Dt::SModelFileDescriptor ModelFileDesc;

        ModelFileDesc.m_pFileName = "gizmo.obj";
        ModelFileDesc.m_GenFlag   = Dt::SGeneratorFlag::DefaultFlipUVs;

        Dt::CModel& rModel = Dt::ModelManager::CreateModel(ModelFileDesc);

        ModelDesc.m_pMesh = &rModel.GetMesh(0);

        m_GizmoModelPtr = MeshManager::CreateMesh(ModelDesc);

        // -----------------------------------------------------------------------------

        static float PlaneVertexBufferData[] =
        {
            0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
        };
        
        static unsigned int PlaneIndexBufferData[] =
        {
            0, 1, 2, 0, 2, 3,
        };
        
         
        
        // -----------------------------------------------------------------------------
        // Engine buffer handling
        // -----------------------------------------------------------------------------
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::VertexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(PlaneVertexBufferData);
        BufferDesc.m_pBytes        = &PlaneVertexBufferData[0];
        BufferDesc.m_pClassKey     = 0;
        
        CBufferPtr PlanePositionBuffer = BufferManager::CreateBuffer(BufferDesc);
        
        m_PlaneBuffer = BufferManager::CreateVertexBufferSet(PlanePositionBuffer);
        
        // -----------------------------------------------------------------------------
        
        BufferDesc.m_Stride        = 0;
        BufferDesc.m_Usage         = CBuffer::GPURead;
        BufferDesc.m_Binding       = CBuffer::IndexBuffer;
        BufferDesc.m_Access        = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(PlaneIndexBufferData);
        BufferDesc.m_pBytes        = &PlaneIndexBufferData[0];
        BufferDesc.m_pClassKey     = 0;
        
        m_PlaneIndexBuffer = BufferManager::CreateBuffer(BufferDesc);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugRenderer::OnResize(unsigned int _Width, unsigned int _Height)
    {
        BASE_UNUSED(_Width);
        BASE_UNUSED(_Height);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugRenderer::Update()
    {
        // -----------------------------------------------------------------------------
        // Update camera depending on main camera
        // -----------------------------------------------------------------------------
        CCameraPtr MainCameraPtr = ViewManager::GetMainCamera();
        CCameraPtr DebugCameraPtr = ViewManager::GetDebugCamera();

        DebugCameraPtr->GetView()->SetPosition(MainCameraPtr->GetView()->GetPosition());
        DebugCameraPtr->GetView()->SetRotationMatrix(MainCameraPtr->GetView()->GetRotationMatrix());

        DebugCameraPtr->Update();

        // -----------------------------------------------------------------------------
        // Release data
        // -----------------------------------------------------------------------------
        m_DebugCameras .clear();
        m_DebugTextures.clear();
        m_DebugTexts   .clear();

        m_IsGizmoVisible = false;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugRenderer::Render()
    {
        Performance::BeginEvent("Debug");

        RenderCameras();
        RenderGizmo();
        RenderTextures();
        RenderTexts();

        Performance::EndEvent();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugRenderer::RenderCameras()
    {
        auto RenderFrustumPlane = [&](const Base::Float3& _rTopLeft, const Base::Float3& _rTopRight, const Base::Float3& _rBottomRight, const Base::Float3& _rBottomLeft, const Base::Float3& _rColor)
        {
            float ViewBuffer[12];

            ViewBuffer[0] = _rTopLeft[0];
            ViewBuffer[1] = _rTopLeft[1];
            ViewBuffer[2] = _rTopLeft[2];

            ViewBuffer[3] = _rTopRight[0];
            ViewBuffer[4] = _rTopRight[1];
            ViewBuffer[5] = _rTopRight[2];

            ViewBuffer[6] = _rBottomRight[0];
            ViewBuffer[7] = _rBottomRight[1];
            ViewBuffer[8] = _rBottomRight[2];

            ViewBuffer[9] = _rBottomLeft[0];
            ViewBuffer[10] = _rBottomLeft[1];
            ViewBuffer[11] = _rBottomLeft[2];

            BufferManager::UploadVertexBufferData(m_PlaneBuffer->GetBuffer(0), ViewBuffer);

            // -----------------------------------------------------------------------------

            SProperties Properties;

            Properties.m_Color[0] = _rColor[0];
            Properties.m_Color[1] = _rColor[1];
            Properties.m_Color[2] = _rColor[2];
            Properties.m_Color[3] = 0.5f;

            BufferManager::UploadConstantBufferData(m_ViewPSBuffer->GetBuffer(0), &Properties);

            // -----------------------------------------------------------------------------

            ContextManager::DrawIndexed(6, 0, 0);
        };

        const unsigned int pOffset[] = {0, 0};
        CDebugCameras::const_iterator CurrentCamera = m_DebugCameras.begin();
        CDebugCameras::const_iterator EndOfCameras  = m_DebugCameras.end();

        if (CurrentCamera == EndOfCameras)
        {
            return;
        }

        Performance::BeginEvent("Cameras");
        
        ContextManager::SetRenderContext(m_RenderContextPtr);
        
        ContextManager::SetVertexBufferSet(m_PlaneBuffer, pOffset);
        
        ContextManager::SetIndexBuffer(m_PlaneIndexBuffer, 0);
        
        ContextManager::SetInputLayout(m_PositionShaderVSPtr->GetInputLayout());
        
        ContextManager::SetTopology(STopology::TriangleList);
        
        ContextManager::SetShaderVS(m_PositionShaderVSPtr);
        
        ContextManager::SetShaderPS(m_PositionShaderPSPtr);
        
        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        
        ContextManager::SetConstantBuffer(1, m_ViewPSBuffer->GetBuffer(0));
        
        

        for (; CurrentCamera != EndOfCameras; ++CurrentCamera)
        {
            // -----------------------------------------------------------------------------
            // Prepare render data
            // -----------------------------------------------------------------------------
            CCameraPtr CameraPtr = CurrentCamera->m_CameraPtr;
            
            const Base::Float3* pWorldSpaceCameraFrustum = CameraPtr->GetWorldSpaceFrustum();
            
            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------
            RenderFrustumPlane(pWorldSpaceCameraFrustum[5], pWorldSpaceCameraFrustum[1], pWorldSpaceCameraFrustum[0], pWorldSpaceCameraFrustum[4], Base::Float3(1.0f, 0.0f, 0.0f)); // Left
            RenderFrustumPlane(pWorldSpaceCameraFrustum[4], pWorldSpaceCameraFrustum[6], pWorldSpaceCameraFrustum[2], pWorldSpaceCameraFrustum[0], Base::Float3(0.0f, 1.0f, 0.0f)); // Bottom
            RenderFrustumPlane(pWorldSpaceCameraFrustum[3], pWorldSpaceCameraFrustum[7], pWorldSpaceCameraFrustum[6], pWorldSpaceCameraFrustum[2], Base::Float3(0.0f, 0.0f, 1.0f)); // Right
            RenderFrustumPlane(pWorldSpaceCameraFrustum[5], pWorldSpaceCameraFrustum[7], pWorldSpaceCameraFrustum[3], pWorldSpaceCameraFrustum[1], Base::Float3(1.0f, 1.0f, 0.0f)); // Top
            
            RenderFrustumPlane(pWorldSpaceCameraFrustum[5], pWorldSpaceCameraFrustum[7], pWorldSpaceCameraFrustum[6], pWorldSpaceCameraFrustum[4], Base::Float3(1.0f, 0.0f, 1.0f)); // Far
            RenderFrustumPlane(pWorldSpaceCameraFrustum[1], pWorldSpaceCameraFrustum[3], pWorldSpaceCameraFrustum[2], pWorldSpaceCameraFrustum[0], Base::Float3(0.0f, 1.0f, 1.0f)); // Near
        }
        
        ContextManager::ResetConstantBuffer(0);
        
        ContextManager::ResetConstantBuffer(1);
        
        ContextManager::ResetTopology();
        
        ContextManager::ResetInputLayout();
        
        ContextManager::ResetIndexBuffer();
        
        ContextManager::ResetVertexBufferSet();
        
        ContextManager::ResetShaderVS();
        
        ContextManager::ResetShaderPS();
        
        ContextManager::ResetRenderContext();

        Performance::EndEvent();
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxDebugRenderer::RenderGizmo()
    {
        if (!m_IsGizmoVisible) return;

        const unsigned int pOffset[] = {0, 0};

        Performance::BeginEvent("Gizmo");

        // -----------------------------------------------------------------------------
        // Per frame: Buffer
        // -----------------------------------------------------------------------------
        CCameraPtr CameraPtr = m_RenderContextPtr->GetCamera();

        SPerFrameConstantBuffer ViewBuffer;

        ViewBuffer.m_ViewProjection = CameraPtr->GetProjectionMatrix();
        ViewBuffer.m_ModelMatrix  = Base::Float4x4::s_Identity;
        ViewBuffer.m_ModelMatrix *= Base::Float4x4().SetTranslation(0.0f, 0.0f, -1.0f);
        ViewBuffer.m_ModelMatrix *= Base::Float4x4().SetScale(0.1f) * ViewManager::GetMainCamera()->GetView()->GetRotationMatrix();

        BufferManager::UploadConstantBufferData(m_ViewModelVSBuffer->GetBuffer(0), &ViewBuffer);

        // -----------------------------------------------------------------------------
        // Per surface
        // -----------------------------------------------------------------------------
        unsigned int NumberOfSurface = m_GizmoModelPtr->GetLOD(0)->GetNumberOfSurfaces();

        for (unsigned int IndexOfSurface = 0; IndexOfSurface < NumberOfSurface; ++ IndexOfSurface)
        {

            // -----------------------------------------------------------------------------
            // Buffer
            // -----------------------------------------------------------------------------
            BufferManager::UploadConstantBufferData(m_DeferredPassPSBuffer->GetBuffer(0), &m_GizmoModelPtr->GetLOD(0)->GetSurface(IndexOfSurface)->GetMaterial()->GetMaterialAttributes());

            // -----------------------------------------------------------------------------
            // Render
            // -----------------------------------------------------------------------------        
            ContextManager::SetRenderContext(m_RenderContextPtr);

            ContextManager::SetVertexBufferSet(m_GizmoModelPtr->GetLOD(0)->GetSurface(IndexOfSurface)->GetVertexBuffer(), pOffset);

            ContextManager::SetIndexBuffer(m_GizmoModelPtr->GetLOD(0)->GetSurface(IndexOfSurface)->GetIndexBuffer(), 0);

            ContextManager::SetInputLayout(m_GizmoModelPtr->GetLOD(0)->GetSurface(IndexOfSurface)->GetShaderVS()->GetInputLayout());

            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::SetShaderVS(m_GizmoShaderVSPtr);

            ContextManager::SetShaderPS(m_GizmoShaderPSPtr);

            ContextManager::SetConstantBuffer(0, m_ViewModelVSBuffer->GetBuffer(0));

            ContextManager::SetConstantBuffer(1, m_DeferredPassPSBuffer->GetBuffer(0));

            ContextManager::DrawIndexed(m_GizmoModelPtr->GetLOD(0)->GetSurface(IndexOfSurface)->GetNumberOfIndices(), 0, 0);

            ContextManager::ResetConstantBuffer(0);

            ContextManager::ResetConstantBuffer(1);

            ContextManager::ResetTopology();

            ContextManager::ResetInputLayout();

            ContextManager::ResetIndexBuffer();

            ContextManager::ResetVertexBufferSet();

            ContextManager::ResetShaderVS();

            ContextManager::ResetShaderPS();

            ContextManager::ResetRenderContext();
        }

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxDebugRenderer::RenderTextures()
    {
        const unsigned int pOffset[] = { 0, 0 };
        CDebugTextures::iterator CurrentTexture = m_DebugTextures.begin();
        CDebugTextures::iterator EndOfTextures  = m_DebugTextures.end();

        if (CurrentTexture == EndOfTextures)
        {
            return;
        }

        Performance::BeginEvent("Textures");

        ContextManager::SetRenderFlags(CRenderState::NoDepth);

        ContextManager::SetRenderContext(m_RenderContextPtr);

        ContextManager::SetVertexBufferSet(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), pOffset);
 
        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_QuadTextureShaderVSPtr->GetInputLayout());

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_QuadTextureShaderVSPtr);
        
        ContextManager::SetShaderPS(m_QuadTextureShaderPSPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetConstantBuffer(1, m_BaseModelVSBuffer->GetBuffer(0));

        for (; CurrentTexture != EndOfTextures; ++ CurrentTexture)
        {
            // -----------------------------------------------------------------------------
            // 
            //   0,0                      1,0
            //    + ---------------------- +
            //    |                        |
            //    |                        |
            //    |                        |
            //    |                        |
            //    + ---------------------- +
            //   0,1                      1,1
            //
            // -----------------------------------------------------------------------------

            Base::Float2 MinPoint   = CurrentTexture->m_ScreenRegion.GetMin();
            Base::Float2 MaxPoint   = CurrentTexture->m_ScreenRegion.GetMax();
            Base::Float2 Difference = MaxPoint - MinPoint;

            SPerDrawCallConstantBuffer ModelBuffer;

            ModelBuffer.m_ModelMatrix = Base::Float4x4::s_Identity;
            ModelBuffer.m_ModelMatrix.SetScale(Difference[0], Difference[1], 1.0f);
            ModelBuffer.m_ModelMatrix.InjectTranslation(MinPoint[0], MinPoint[1], 1.0f);

            BufferManager::UploadConstantBufferData(m_BaseModelVSBuffer->GetBuffer(0), &ModelBuffer);

            // -----------------------------------------------------------------------------

            ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

            ContextManager::SetTexture(0, CurrentTexture->m_TexturePtr->GetTexture(0));

            ContextManager::DrawIndexed(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
        }

        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetConstantBuffer(0);

        ContextManager::ResetConstantBuffer(1);

        ContextManager::ResetTopology();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBufferSet();

        ContextManager::ResetShaderVS();
        
        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        ContextManager::SetRenderFlags(0);

        Performance::EndEvent();

        m_DebugTextures.clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxDebugRenderer::RenderTexts()
    {
        const unsigned int      pOffset[] = { 0, 0 };
        void*                   pInstances;
        SPerTextInstanceBuffer* pInstance;
        unsigned int            NumberOfLetters = 0;
        Base::Int2              ScreenSize = Main::GetActiveWindowSize();

        CDebugTexts::const_iterator CurrentText = m_DebugTexts.begin();
        CDebugTexts::const_iterator EndOfTexts  = m_DebugTexts.end();

        if (CurrentText == EndOfTexts)
        {
            return;
        }

        Performance::BeginEvent("Texts");

        ContextManager::SetRenderFlags(CRenderState::NoDepth);

        ContextManager::SetRenderContext(m_RenderContextPtr);

        ContextManager::SetVertexBufferSet(m_TextInstanceBufferSetPtr, pOffset);

        ContextManager::SetIndexBuffer(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), 0);

        ContextManager::SetInputLayout(m_QuadTextShaderVSPtr->GetInputLayout());

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetShaderVS(m_QuadTextShaderVSPtr);

        ContextManager::SetShaderPS(m_QuadTextShaderPSPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipPointClamp));

        ContextManager::SetTexture(0, m_ConsolasTexturePtr);

        pInstances = BufferManager::MapVertexBuffer(m_TextInstanceBufferSetPtr->GetBuffer(1), CBuffer::Write);

        assert(pInstances != 0);

        for (; CurrentText != EndOfTexts; ++CurrentText)
        {
            Base::Float2 Start    = CurrentText->m_ScreenPosition * Base::Float2(static_cast<float>(ScreenSize[0]), static_cast<float>(ScreenSize[1]));
            Base::Float2 Padding  = Start;
            std::string  Text     = CurrentText->m_Text;
            float        TextSize = static_cast<float>(CurrentText->m_TextSize);

            for (unsigned int IndexOfLetter = 0; IndexOfLetter < Text.length(); ++ IndexOfLetter)
            {
                unsigned char CurrentLetter = Text[IndexOfLetter];

                if (CurrentLetter == '\n')
                {
                    Padding[0]  = Start[0];
                    Padding[1] += TextSize;

                    continue;
                }

                unsigned char DrawLetter = CurrentLetter - 32;

                Base::Float2 LetterPosition;

                LetterPosition[0] = static_cast<float>(DrawLetter % 16);
                LetterPosition[1] = static_cast<float>(DrawLetter / 16);

                pInstance = &(static_cast<SPerTextInstanceBuffer*>(pInstances))[NumberOfLetters++];

                pInstance->m_TextSettings = Base::Float3(TextSize, Padding[0], Padding[1]);
                pInstance->m_CharSettings = Base::Float3(1.0f / 16.0f, LetterPosition[0], LetterPosition[1]);
                pInstance->m_CharColor    = CurrentText->m_Color;

                if (NumberOfLetters == s_MaxNumberOfInstances)
                {
                    BufferManager::UnmapVertexBuffer(m_TextInstanceBufferSetPtr->GetBuffer(1));

                    ContextManager::DrawIndexedInstanced(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), NumberOfLetters, 0, 0, 0);

                    pInstances = BufferManager::MapVertexBuffer(m_TextInstanceBufferSetPtr->GetBuffer(1), CBuffer::Write);

                    NumberOfLetters = 0;
                }

                Padding[0] += TextSize;
            }
        }

        if (NumberOfLetters > 0)
        {
            BufferManager::UnmapVertexBuffer(m_TextInstanceBufferSetPtr->GetBuffer(1));

            ContextManager::DrawIndexedInstanced(m_QuadModelPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), NumberOfLetters, 0, 0, 0);

            NumberOfLetters = 0;
        }

        ContextManager::ResetTexture(0);

        ContextManager::ResetSampler(0);

        ContextManager::ResetConstantBuffer(0);

        ContextManager::ResetTopology();

        ContextManager::ResetInputLayout();

        ContextManager::ResetIndexBuffer();

        ContextManager::ResetVertexBufferSet();

        ContextManager::ResetShaderVS();

        ContextManager::ResetShaderPS();

        ContextManager::ResetRenderContext();

        ContextManager::SetRenderFlags(0);

        Performance::EndEvent();

        m_DebugTexts.clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxDebugRenderer::DrawCamera(CCameraPtr _CameraPtr)
    {
        SDebugCamera NewDebugCamera;

        NewDebugCamera.m_CameraPtr = _CameraPtr;

        m_DebugCameras.push_back(NewDebugCamera);
    }

    // -----------------------------------------------------------------------------

    void CGfxDebugRenderer::DrawGizmo(bool _Flag)
    {
        m_IsGizmoVisible = _Flag;
    }

    // -----------------------------------------------------------------------------

    void CGfxDebugRenderer::DrawTexture(CTextureBasePtr _TexturePtr, const Base::AABB2Float& _rScreenRegion)
    {
        SDebugTexture NewDebugTexture;

        NewDebugTexture.m_TexturePtr   = TextureManager::CreateTextureSet(static_cast<CTextureBasePtr>(_TexturePtr));
        NewDebugTexture.m_ScreenRegion = _rScreenRegion;

        m_DebugTextures.push_back(NewDebugTexture);
    }

    // -----------------------------------------------------------------------------

    void CGfxDebugRenderer::DrawText(const std::string& _rText, const Base::Float2& _rScreenPosition, const Base::Float4& _rColor, unsigned int _TextSize)
    {
        SDebugText NewDebugText;

        NewDebugText.m_Text           = _rText;
        NewDebugText.m_ScreenPosition = _rScreenPosition;
        NewDebugText.m_Color          = _rColor;
        NewDebugText.m_TextSize       = _TextSize;

        m_DebugTexts.push_back(NewDebugText);
    }
} // namespace


namespace Gfx
{
namespace DebugRenderer
{
    void OnStart()
    {
        CGfxDebugRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxDebugRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    
    void OnSetupShader()
    {
        CGfxDebugRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxDebugRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxDebugRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxDebugRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxDebugRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxDebugRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxDebugRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxDebugRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxDebugRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxDebugRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxDebugRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxDebugRenderer::GetInstance().Render();
    }

    // -----------------------------------------------------------------------------

    void DrawCamera(CCameraPtr _CameraPtr)
    {
        CGfxDebugRenderer::GetInstance().DrawCamera(_CameraPtr);
    }

    // -----------------------------------------------------------------------------

    void DrawGizmo(bool _Flag)
    {
        CGfxDebugRenderer::GetInstance().DrawGizmo(_Flag);
    }

    // -----------------------------------------------------------------------------

    void DrawTexture(CTextureBasePtr _TexturePtr, const Base::AABB2Float& _rScreenRegion)
    {
        CGfxDebugRenderer::GetInstance().DrawTexture(_TexturePtr, _rScreenRegion);
    }

    // -----------------------------------------------------------------------------

    void DrawText(const std::string& _rText, const Base::Float2& _rScreenPosition, const Base::Float4& _rColor, unsigned int _TextSize)
    {
        CGfxDebugRenderer::GetInstance().DrawText(_rText, _rScreenPosition, _rColor, _TextSize);
    }
} // namespace DebugRenderer
} // namespace Gfx
