
#include "plugin/slam/slam_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/camera/cam_control_manager.h"
#include "engine/camera/cam_editor_control.h"

#include "engine/core/core_program_parameters.h"
#include "engine/core/core_time.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

#include "plugin/slam/gfx_reconstruction_renderer.h"
#include "plugin/slam/mr_scalable_slam_reconstructor.h"

#include "GL/glew.h"

#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

using namespace Base;
using namespace Gfx;

namespace
{
    glm::vec3 g_CubeVertices[] =
    {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(1.0f, 0.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(0.0f, 1.0f, 1.0f),
    };

	struct SDrawCallConstantBuffer
	{
		glm::mat4 m_WorldMatrix;
		glm::vec4 m_Color;
	};

    struct SPickingBuffer
    {
        glm::vec4 m_RayStart;
        glm::vec4 m_RayDirection;
        glm::vec4 m_WorldHitPosition;
    };

    struct SIntrinsics
    {
        glm::mat4 m_KMatrix;
        glm::mat4 m_InvKMatrix;
        glm::vec2 m_FocalPoint;
        glm::vec2 m_FocalLength;
        glm::vec2 m_InvFocalLength;
        glm::vec2 Padding;
    };

    class CGfxReconstructionRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxReconstructionRenderer)
        
    public:

        CGfxReconstructionRenderer();
        ~CGfxReconstructionRenderer();
        
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
        void ChangeCamera(bool _IsTrackingCamera);

        void SetReconstructor(MR::CScalableSLAMReconstructor& _rReconstructor);

        glm::vec3 Pick(const glm::ivec2& _rCursorPosition);

        void SetSelectionBox(const glm::vec3& _rAnchor0, const glm::vec3& _rAnchor1, float _Height, int _State);

    private:

        // -----------------------------------------------------------------------------
        // Stuff for selection box
        // -----------------------------------------------------------------------------
        enum class ESelection
        {
            NOSELECTION,
            FIRSTPRESS,
            FIRSTRELEASE,
            SECONDPRESS,
            SECONDRELEASE
        };

    private:

        void Initialize();

        void RenderVolumeVertexMap();

		void RaycastScalableVolume();
        void RaycastScalableVolumeWithHighlight();
        void RaycastScalableVolumeDiminished();
        
        void RenderQueuedRootVolumes();
        void RenderQueuedLevel1Grids();
        void RenderQueuedLevel2Grids();

        void RenderCamera();

        void RenderPlanes();

        void RenderVertexMap();

        void RenderSelectionBox();

        void SetIntrinsics(const glm::vec2& _FocalLength, const glm::vec2& _FocalPoint);

    private:

		MR::CScalableSLAMReconstructor* m_pScalableReconstructor;

        SIntrinsics m_Intrinsics;
        
        CShaderPtr m_OutlineVSPtr;
        CShaderPtr m_OutlineFSPtr;

        CShaderPtr m_VolumeVertexMapVSPtr;
        CShaderPtr m_VolumeVertexMapFSPtr;

        CShaderPtr m_OutlineLevel1VSPtr;
        CShaderPtr m_OutlineLevel1FSPtr;
        CShaderPtr m_OutlineLevel2VSPtr;
        CShaderPtr m_OutlineLevel2FSPtr;
        
        CShaderPtr m_RaycastVSPtr;
        CShaderPtr m_RaycastDiminishedVSPtr;
        CShaderPtr m_RaycastFSPtr;
        CShaderPtr m_RaycastHighlightFSPtr;
        CShaderPtr m_RaycastDiminishedFSPtr;

        CShaderPtr m_CopyRaycastVSPtr;
        CShaderPtr m_CopyRaycastFSPtr;

        CBufferPtr m_RaycastConstantBufferPtr;
        CBufferPtr m_RaycastHighLightConstantBufferPtr;
        CBufferPtr m_DrawCallConstantBufferPtr;
        CBufferPtr m_IntrinsicsConstantBufferPtr;
                
        CMeshPtr m_CameraMeshPtr;
		CInputLayoutPtr m_CameraInputLayoutPtr;

		CMeshPtr m_CubeOutlineMeshPtr;
		CInputLayoutPtr m_CubeOutlineInputLayoutPtr;

        CMeshPtr m_VolumeMeshPtr;        
        CInputLayoutPtr m_VolumeInputLayoutPtr;

        CMeshPtr m_QuadMeshPtr;
        CInputLayoutPtr m_QuadInputLayoutPtr;

        CRenderContextPtr m_OutlineRenderContextPtr;
        CRenderContextPtr m_PlaneRenderContextPtr;

        CMeshPtr m_PlaneMeshPtr;
        
        CShaderPtr m_PointCloudVSPtr;
        CShaderPtr m_PointCloudFSPtr;

        CShaderPtr m_PickingCSPtr;

        CTexturePtr m_DiminishedTargetPtr;
        CTargetSetPtr m_DiminishedTargetSetPtr;

        CBufferPtr m_PickingBuffer;

        bool m_UseTrackingCamera;

        bool m_RenderVolumeVertexMap;
        bool m_RenderVolume;
        bool m_RenderVertexMap;
        bool m_RenderRootQueue;
        bool m_RenderLevel1Queue;
        bool m_RenderLevel2Queue;
        bool m_RenderBackSides;
        bool m_RenderPlanes;

        glm::vec3 m_SelectionBoxMin;
        glm::vec3 m_SelectionBoxMax;
        glm::mat4 m_SelectionTransform;

        ESelection m_SelectionState;

        bool m_IsInitialized;
    };
} // namespace

namespace
{
    using namespace Base;

    CGfxReconstructionRenderer::CGfxReconstructionRenderer()
        : m_UseTrackingCamera    (true)
        , m_RenderVolume         (true)
        , m_RenderVolumeVertexMap(false)
        , m_RenderVertexMap      (false)
        , m_RenderRootQueue      (false)
        , m_RenderLevel1Queue    (false)
        , m_RenderLevel2Queue    (false)
        , m_IsInitialized        (false)
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxReconstructionRenderer::~CGfxReconstructionRenderer()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnStart()
    {
        assert(Main::GetGraphicsAPI().m_GraphicsAPI == CGraphicsInfo::OpenGL);

        Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxReconstructionRenderer::OnResize));
                                
        m_UseTrackingCamera     = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:use_tracking_camera", true);
        m_RenderVolume          = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:volume"             , true);
        m_RenderVolumeVertexMap = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:volume_vertex_map"  , false);
        m_RenderVertexMap       = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:vertex_map"         , false);
        m_RenderRootQueue       = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:queues:root"        , false);
        m_RenderLevel1Queue     = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:queues:level1"      , false);
        m_RenderLevel2Queue     = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:queues:level2"      , false);
        m_RenderBackSides       = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:backsides"          , true);
        m_RenderPlanes          = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:planes"             , false);

        m_IsInitialized = false;
    }

    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::Initialize()
    {
        // -----------------------------------------------------------------------------
        // Intrinsics data
        // -----------------------------------------------------------------------------
        glm::ivec2 DepthImageSize;
        glm::ivec2 ColorImageSize;
        glm::vec2 FocalLength;
        glm::vec2 FocalPoint;

        m_pScalableReconstructor->GetImageSizes(DepthImageSize, ColorImageSize);
        m_pScalableReconstructor->GetIntrinsics(FocalLength, FocalPoint);

        FocalLength[0] = FocalLength[0] / DepthImageSize[0] * 1280;
        FocalLength[1] = FocalLength[1] / DepthImageSize[1] * 720;
        FocalPoint[0] = FocalPoint[0] / DepthImageSize[0] * 1280;
        FocalPoint[1] = FocalPoint[1] / DepthImageSize[1] * 720;

        SetIntrinsics(FocalLength, FocalPoint);

        // -----------------------------------------------------------------------------
        // Rest
        // -----------------------------------------------------------------------------
        OnSetupShader();
        OnSetupKernels();
        OnSetupRenderTargets();
        OnSetupStates();
        OnSetupTextures();
        OnSetupBuffers();
        OnSetupResources();
        OnSetupModels();
        OnSetupEnd();

        m_IsInitialized = true;
    }

    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnExit()
    {
        m_OutlineVSPtr = 0;
        m_OutlineFSPtr = 0;
        m_OutlineLevel1VSPtr = 0;
        m_OutlineLevel1FSPtr = 0;
        m_OutlineLevel2VSPtr = 0;
        m_OutlineLevel2FSPtr = 0;
        m_RaycastVSPtr = 0;
        m_RaycastDiminishedVSPtr = 0;
        m_RaycastFSPtr = 0;
        m_RaycastHighlightFSPtr = 0;
        m_RaycastDiminishedFSPtr = 0;
        m_CopyRaycastVSPtr = 0;
        m_CopyRaycastFSPtr = 0;
        m_PickingCSPtr = 0;

        m_VolumeVertexMapVSPtr = 0;
        m_VolumeVertexMapFSPtr = 0;
        
        m_PickingBuffer = 0;

        m_RaycastConstantBufferPtr = 0;
        m_RaycastHighLightConstantBufferPtr = 0;
        m_DrawCallConstantBufferPtr = 0;
        m_IntrinsicsConstantBufferPtr = 0;
        
        m_CameraMeshPtr = 0;
        m_VolumeMeshPtr = 0;
        m_QuadMeshPtr = 0;
		m_CubeOutlineMeshPtr = 0;
        m_PlaneMeshPtr = 0;
        m_CameraInputLayoutPtr = 0;
        m_VolumeInputLayoutPtr = 0;
        m_QuadInputLayoutPtr = 0;
		m_CubeOutlineInputLayoutPtr = 0;

        m_OutlineRenderContextPtr = 0;
        m_PlaneRenderContextPtr = 0;

		m_pScalableReconstructor = nullptr;

        m_PointCloudVSPtr = 0;
        m_PointCloudFSPtr = 0;

        m_DiminishedTargetPtr = nullptr;
        m_DiminishedTargetSetPtr = nullptr;

        m_IsInitialized = false;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupShader()
    {
		MR::SReconstructionSettings Settings;

        const std::string InternalFormatString = Core::CProgramParameters::GetInstance().Get("mr:slam:map_format", "rgba16f");

        m_pScalableReconstructor->GetReconstructionSettings(&Settings);

        glm::ivec2 DepthImageSize = m_pScalableReconstructor->GetDepthImageSize();

        std::stringstream DefineStream;

        DefineStream
            << "#define TRUNCATED_DISTANCE " << Settings.m_TruncatedDistance / 1000.0f << " \n"
            << "#define VOLUME_SIZE " << Settings.m_VolumeSize << " \n"
            << "#define VOXEL_SIZE " << Settings.m_VoxelSize << " \n"
            << "#define MAX_INTEGRATION_WEIGHT " << Settings.m_MaxIntegrationWeight << '\n'
            << "#define DEPTH_IMAGE_WIDTH " << DepthImageSize.x << '\n'
            << "#define DEPTH_IMAGE_HEIGHT " << DepthImageSize.y << '\n'
            << "#define ROOT_RESOLUTION " << Settings.m_GridResolutions[0] << '\n'
            << "#define LEVEL1_RESOLUTION " << Settings.m_GridResolutions[1] << '\n'
            << "#define LEVEL2_RESOLUTION " << Settings.m_GridResolutions[2] << '\n'
            << "#define VOXELS_PER_ROOTGRID " << Settings.m_VoxelsPerGrid[0] << " \n"
            << "#define VOXELS_PER_LEVEL1GRID " << Settings.m_VoxelsPerGrid[1] << " \n"
            << "#define VOXELS_PER_LEVEL2GRID " << Settings.m_VoxelsPerGrid[2] << " \n"
            << "#define MAP_TEXTURE_FORMAT " << InternalFormatString << " \n"
            << "#define RAYCAST_NEAR " << 0.0f << " \n"
            << "#define RAYCAST_FAR " << 1000.0f << " \n"
            << "#define MIN_TREE_WEIGHT " << Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:min_weight", 30) << " \n";

        if (Settings.m_CaptureColor)
        {
            DefineStream << "#define CAPTURE_COLOR\n";
        }
        if (m_RenderBackSides)
        {
            DefineStream << "#define RAYCAST_BACKSIDES\n";
        }

        std::string DefineString = DefineStream.str();

        m_OutlineVSPtr = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rendering\\vs_outline.glsl", "main", DefineString.c_str());
        m_OutlineFSPtr = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rendering\\fs_outline.glsl", "main", DefineString.c_str());
        m_OutlineLevel1VSPtr = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rendering\\vs_outline_level1.glsl", "main", DefineString.c_str());
        m_OutlineLevel1FSPtr = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rendering\\fs_outline_level1.glsl", "main", DefineString.c_str());
        m_OutlineLevel2VSPtr = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rendering\\vs_outline_level2.glsl", "main", DefineString.c_str());
        m_OutlineLevel2FSPtr = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rendering\\fs_outline_level2.glsl", "main", DefineString.c_str());

        m_PointCloudVSPtr = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rendering\\vs_point_cloud.glsl", "main", DefineString.c_str());
        m_PointCloudFSPtr = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rendering\\fs_point_cloud.glsl", "main", DefineString.c_str());

        m_RaycastVSPtr = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rendering\\vs_raycast.glsl", "main", DefineString.c_str());
        m_RaycastDiminishedVSPtr = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rendering\\vs_raycast_diminished.glsl", "main", DefineString.c_str());
        m_RaycastFSPtr = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rendering\\fs_raycast.glsl", "main", DefineString.c_str());
        m_RaycastHighlightFSPtr = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rendering\\fs_raycast_highlight.glsl", "main", DefineString.c_str());
        m_RaycastDiminishedFSPtr = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rendering\\fs_raycast_diminished.glsl", "main", DefineString.c_str());

        m_CopyRaycastVSPtr = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rendering\\vs_copy_raycast.glsl", "main", DefineString.c_str());
        m_CopyRaycastFSPtr = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rendering\\fs_copy_raycast.glsl", "main", DefineString.c_str());

        m_VolumeVertexMapVSPtr = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rendering\\vs_volume_vertex_map.glsl", "main", DefineString.c_str());
        m_VolumeVertexMapFSPtr = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rendering\\fs_volume_vertex_map.glsl", "main", DefineString.c_str());
        
        m_PickingCSPtr = ShaderManager::CompileCS("slam\\scalable_kinect_fusion\\cs_picking.glsl", "main", DefineString.c_str());

        SInputElementDescriptor InputLayoutDesc = {};

        InputLayoutDesc.m_pSemanticName        = "POSITION";
        InputLayoutDesc.m_SemanticIndex        = 0;
        InputLayoutDesc.m_Format               = CInputLayout::Float3Format;
        InputLayoutDesc.m_InputSlot            = 0;
        InputLayoutDesc.m_AlignedByteOffset    = 0;
        InputLayoutDesc.m_Stride               = 12;
        InputLayoutDesc.m_InputSlotClass       = CInputLayout::PerVertex;
        InputLayoutDesc.m_InstanceDataStepRate = 0;

        m_CameraInputLayoutPtr = ShaderManager::CreateInputLayout(&InputLayoutDesc, 1, m_OutlineVSPtr);
        m_VolumeInputLayoutPtr = ShaderManager::CreateInputLayout(&InputLayoutDesc, 1, m_RaycastVSPtr);
        m_CubeOutlineInputLayoutPtr = ShaderManager::CreateInputLayout(&InputLayoutDesc, 1, m_OutlineVSPtr);
        m_QuadInputLayoutPtr = ShaderManager::CreateInputLayout(&InputLayoutDesc, 1, m_OutlineVSPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupKernels()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupRenderTargets()
    {
        STextureDescriptor TextureDescriptor = {};

        TextureDescriptor.m_NumberOfPixelsU  = 1280;
        TextureDescriptor.m_NumberOfPixelsV  = 720;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTexture::ShaderResource | CTexture::RenderTarget;
        TextureDescriptor.m_Access           = CTexture::CPUWrite;
        TextureDescriptor.m_Usage            = CTexture::GPUReadWrite;
        TextureDescriptor.m_Semantic         = CTexture::UndefinedSemantic;
        TextureDescriptor.m_Format           = CTexture::R16G16B16A16_FLOAT;

        m_DiminishedTargetPtr = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureManager::SetTextureLabel(m_DiminishedTargetPtr, "Diminished Texture");

        m_DiminishedTargetSetPtr = TargetSetManager::CreateTargetSet(m_DiminishedTargetPtr);

        TargetSetManager::SetTargetSetLabel(m_DiminishedTargetSetPtr, "Diminished Target Set");
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupStates()
    {
        m_OutlineRenderContextPtr = ContextManager::CreateRenderContext();
        m_OutlineRenderContextPtr->SetCamera(ViewManager::GetMainCamera());
        m_OutlineRenderContextPtr->SetViewPortSet(ViewManager::GetViewPortSet());
        m_OutlineRenderContextPtr->SetTargetSet(TargetSetManager::GetLightAccumulationTargetSet());
        m_OutlineRenderContextPtr->SetRenderState(StateManager::GetRenderState(CRenderState::NoCull | CRenderState::Wireframe));

        m_PlaneRenderContextPtr = ContextManager::CreateRenderContext();
        m_PlaneRenderContextPtr->SetCamera(ViewManager::GetMainCamera());
        m_PlaneRenderContextPtr->SetViewPortSet(ViewManager::GetViewPortSet());
        m_PlaneRenderContextPtr->SetTargetSet(TargetSetManager::GetLightAccumulationTargetSet());
        m_PlaneRenderContextPtr->SetRenderState(StateManager::GetRenderState(CRenderState::NoCull | CRenderState::AlphaBlend));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupTextures()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstantBufferDesc = {};

        ConstantBufferDesc.m_Stride = 0;
        ConstantBufferDesc.m_Usage = CBuffer::GPURead;
        ConstantBufferDesc.m_Binding = CBuffer::ConstantBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(glm::vec4) * 2;
        ConstantBufferDesc.m_pBytes = nullptr;
        ConstantBufferDesc.m_pClassKey = 0;

        m_RaycastConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_NumberOfBytes = sizeof(glm::mat4);

        m_RaycastHighLightConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SDrawCallConstantBuffer);
        m_DrawCallConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_Stride = 0;
        ConstantBufferDesc.m_Usage = CBuffer::GPUToCPU;
        ConstantBufferDesc.m_Binding = CBuffer::ResourceBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUReadWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SPickingBuffer);
        ConstantBufferDesc.m_pBytes = nullptr;
        ConstantBufferDesc.m_pClassKey = 0;

        m_PickingBuffer = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_Stride        = 0;
        ConstantBufferDesc.m_Usage         = CBuffer::GPURead;
        ConstantBufferDesc.m_Binding       = CBuffer::ConstantBuffer;
        ConstantBufferDesc.m_Access        = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SIntrinsics);
        ConstantBufferDesc.m_pBytes        = &m_Intrinsics;
        ConstantBufferDesc.m_pClassKey     = 0;

        m_IntrinsicsConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        BufferManager::SetBufferLabel(m_IntrinsicsConstantBufferPtr, "Camera Intrinsics");
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupModels()
    {
        ////////////////////////////////////////////////////////////////////////////////
        // Create camera frustum mesh
        ////////////////////////////////////////////////////////////////////////////////

        glm::ivec2 DepthSize = m_pScalableReconstructor->GetDepthImageSize();
        glm::vec4 Intrinsics = m_pScalableReconstructor->GetDepthIntrinsics();
        
        float Focalx = (-Intrinsics.z / DepthSize.x) / (Intrinsics.x / DepthSize.x);
        float Focaly = (-Intrinsics.w / DepthSize.y) / (Intrinsics.y / DepthSize.y);
        
        MR::SReconstructionSettings Settings;
        MR::SReconstructionSettings::SetDefaultSettings(Settings);

        float Min = Settings.m_DepthThreshold.x / 1000.f;
        float Max = Settings.m_DepthThreshold.y / 1000.f;

        glm::vec3 CameraVertices[] =
        {
            glm::vec3(-Focalx * Max, -Focaly * Max, Max),
            glm::vec3( Focalx * Max, -Focaly * Max, Max),
            glm::vec3( Focalx * Max,  Focaly * Max, Max),
            glm::vec3(-Focalx * Max,  Focaly * Max, Max),
            glm::vec3(         0.0f,          0.0f, 0.0f),
            glm::vec3(-Focalx * Min, -Focaly * Min, Min),
            glm::vec3( Focalx * Min, -Focaly * Min, Min),
            glm::vec3( Focalx * Min,  Focaly * Min, Min),
            glm::vec3(-Focalx * Min,  Focaly * Min, Min),
        };

        glm::vec3 CameraLines[24] =
        {
            CameraVertices[4], CameraVertices[0],
            CameraVertices[4], CameraVertices[1],
            CameraVertices[4], CameraVertices[2],
            CameraVertices[4], CameraVertices[3],

            CameraVertices[0], CameraVertices[1],
            CameraVertices[1], CameraVertices[2],
            CameraVertices[2], CameraVertices[3],
            CameraVertices[3], CameraVertices[0],

            CameraVertices[5], CameraVertices[6],
            CameraVertices[6], CameraVertices[7],
            CameraVertices[7], CameraVertices[8],
            CameraVertices[8], CameraVertices[5],
        };

        m_CameraMeshPtr = MeshManager::CreateMesh(CameraLines, sizeof(CameraLines) / sizeof(CameraLines[0]), nullptr, 0);

        ////////////////////////////////////////////////////////////////////////////////
        // Create cube mesh
        ////////////////////////////////////////////////////////////////////////////////

        glm::vec3 CubeLines[24] =
        {
            g_CubeVertices[0], g_CubeVertices[1],
            g_CubeVertices[1], g_CubeVertices[2],
            g_CubeVertices[2], g_CubeVertices[3],
            g_CubeVertices[3], g_CubeVertices[0],

            g_CubeVertices[0], g_CubeVertices[4],
            g_CubeVertices[1], g_CubeVertices[5],
            g_CubeVertices[2], g_CubeVertices[6],
            g_CubeVertices[3], g_CubeVertices[7],

            g_CubeVertices[4], g_CubeVertices[5],
            g_CubeVertices[5], g_CubeVertices[6],
            g_CubeVertices[6], g_CubeVertices[7],
            g_CubeVertices[7], g_CubeVertices[4],
        };

        uint32_t CubeIndices[] =
        {
            0, 1, 2,
            0, 2, 3,

            5, 2, 1,
            5, 6, 2,

            4, 5, 1,
            4, 1, 0,

            4, 0, 7,
            0, 3, 7,

            7, 2, 6,
            7, 3, 2,

            4, 7, 6,
            4, 6, 5,
        };
        
        m_VolumeMeshPtr = MeshManager::CreateMesh(g_CubeVertices, sizeof(g_CubeVertices) / sizeof(g_CubeVertices[0]), CubeIndices, sizeof(CubeIndices) / sizeof(CubeIndices[0]));

        ////////////////////////////////////////////////////////////////////////////////
        // Create cube outline mesh
        ////////////////////////////////////////////////////////////////////////////////

        m_CubeOutlineMeshPtr = MeshManager::CreateMesh(CubeLines, sizeof(CubeLines) / sizeof(CubeLines[0]), nullptr, 0);

        ////////////////////////////////////////////////////////////////////////////////
        // Create quad mesh
        ////////////////////////////////////////////////////////////////////////////////

        glm::vec3 QuadLines[4] =
        {
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 1.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f),
        };

        m_QuadMeshPtr = MeshManager::CreateMesh(QuadLines, sizeof(QuadLines) / sizeof(QuadLines[0]), nullptr, 0);

        ////////////////////////////////////////////////////////////////////////////////
        // Create plane mesh
        ////////////////////////////////////////////////////////////////////////////////

        std::vector<glm::vec3> PlaneVertices;

        const int PlaneSize = 3;

        for (int x = -PlaneSize; x <= PlaneSize; ++ x)
        {
            for (int y = -PlaneSize; y <= PlaneSize; ++ y)
            {
                glm::vec3 NewVertices[4] =
                {
                    glm::vec3(QuadLines[0].x, QuadLines[0].z, QuadLines[0].y),  // convert from x-y to x-z planes
                    glm::vec3(QuadLines[1].x, QuadLines[1].z, QuadLines[1].y),
                    glm::vec3(QuadLines[2].x, QuadLines[2].z, QuadLines[2].y),
                    glm::vec3(QuadLines[3].x, QuadLines[3].z, QuadLines[3].y),
                };

                for (int i = 0; i < 4; ++ i)
                {
                    NewVertices[i][0] += x;
                    NewVertices[i][2] += y;

                    PlaneVertices.push_back(NewVertices[i] / PlaneSize);
                }
            }
        }

        uint32_t BaseIndices[] =
        {
            0, 1, 2,
            1, 2, 3,
        };

        std::vector<uint32_t> Indices;

        for (int i = 0; i < static_cast<int>(PlaneVertices.size()) / 4; ++i)
        {
            for (int j = 0; j < 6; ++j)
            {
                Indices.push_back(BaseIndices[j] + i * 4);
            }
        }

        m_PlaneMeshPtr = MeshManager::CreateMesh(PlaneVertices, Indices);
    }

    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnReload()
    {
        m_pScalableReconstructor->ResetReconstruction();

        OnSetupShader();
    }

    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnUnloadMap()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::OnResize(unsigned int _Width, unsigned int _Height)
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::Update()
    {

    }
    
    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderSelectionBox()
    {
        if (m_SelectionState == ESelection::NOSELECTION)
        {
            return;
        }

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetRenderContext(m_OutlineRenderContextPtr);
        ContextManager::SetShaderVS(m_OutlineVSPtr);
        ContextManager::SetShaderPS(m_OutlineFSPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);

        SDrawCallConstantBuffer BufferData;

        ContextManager::SetVertexBuffer(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetInputLayout(m_CubeOutlineInputLayoutPtr);

        ContextManager::SetTopology(STopology::LineList);

        BufferData.m_WorldMatrix = m_SelectionTransform;
        BufferData.m_Color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

        BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

        ContextManager::Draw(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface()->GetNumberOfVertices(), 0);
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderVolumeVertexMap()
    {
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetRenderContext(m_OutlineRenderContextPtr);
        ContextManager::SetShaderVS(m_VolumeVertexMapVSPtr);
        ContextManager::SetShaderPS(m_VolumeVertexMapFSPtr);

        ContextManager::SetImageTexture(0, m_pScalableReconstructor->GetVertexMap());
        ContextManager::SetImageTexture(1, m_pScalableReconstructor->GetNormalMap());

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_QuadMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_QuadMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);

        ContextManager::SetInputLayout(m_QuadInputLayoutPtr);
        ContextManager::SetTopology(STopology::TriangleStrip);

        ContextManager::Draw(4, 0);
    }
    
	// -----------------------------------------------------------------------------

	void CGfxReconstructionRenderer::RaycastScalableVolume()
	{
        Performance::BeginEvent("Raycasting for rendering");

        ContextManager::SetTargetSet(TargetSetManager::GetDeferredTargetSet());

        MR::CScalableSLAMReconstructor::SScalableVolume& rVolume = m_pScalableReconstructor->GetVolume();

        MR::SReconstructionSettings Settings;
        m_pScalableReconstructor->GetReconstructionSettings(&Settings);

        glm::mat4 PoseMatrix = m_pScalableReconstructor->GetPoseMatrix();

        ContextManager::SetShaderVS(m_RaycastVSPtr);
        ContextManager::SetShaderPS(m_RaycastFSPtr);

        ContextManager::SetResourceBuffer(0, rVolume.m_RootVolumePoolPtr);
        ContextManager::SetResourceBuffer(1, rVolume.m_RootGridPoolPtr);
        ContextManager::SetResourceBuffer(2, rVolume.m_Level1PoolPtr);
        ContextManager::SetResourceBuffer(3, rVolume.m_TSDFPoolPtr);
        ContextManager::SetResourceBuffer(6, rVolume.m_RootVolumePositionBufferPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_RaycastConstantBufferPtr);
        ContextManager::SetConstantBuffer(2, rVolume.m_AABBBufferPtr);

        ContextManager::Barrier();

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::Default));
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        const glm::vec3 Min = glm::vec3(
            rVolume.m_MinOffset[0] * Settings.m_VolumeSize,
            rVolume.m_MinOffset[1] * Settings.m_VolumeSize,
            rVolume.m_MinOffset[2] * Settings.m_VolumeSize
        );

        const glm::vec3 Max = glm::vec3(
            (rVolume.m_MaxOffset[0] + 1.0f) * Settings.m_VolumeSize, // Add 1.0f because MaxOffset stores the max volume offset
            (rVolume.m_MaxOffset[1] + 1.0f) * Settings.m_VolumeSize, // and we have to consider the volume size
            (rVolume.m_MaxOffset[2] + 1.0f) * Settings.m_VolumeSize
        );

        glm::vec3 Vertices[8] =
        {
            glm::vec3(glm::eulerAngleX(glm::half_pi<float>()) * glm::vec4(Min[0], Min[1], Min[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::half_pi<float>()) * glm::vec4(Max[0], Min[1], Min[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::half_pi<float>()) * glm::vec4(Max[0], Max[1], Min[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::half_pi<float>()) * glm::vec4(Min[0], Max[1], Min[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::half_pi<float>()) * glm::vec4(Min[0], Min[1], Max[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::half_pi<float>()) * glm::vec4(Max[0], Min[1], Max[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::half_pi<float>()) * glm::vec4(Max[0], Max[1], Max[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::half_pi<float>()) * glm::vec4(Min[0], Max[1], Max[2], 1.0f))
        };

        glm::vec4 Color;
        if (Settings.m_CaptureColor)
        {
            Color = m_pScalableReconstructor->IsTrackingLost() ? glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }
        else
        {
            Color = m_pScalableReconstructor->IsTrackingLost() ? glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) : glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        }
        BufferManager::UploadBufferData(m_RaycastConstantBufferPtr, &Color);

        BufferManager::UploadBufferData(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer(), &Vertices);

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);
        ContextManager::SetInputLayout(m_VolumeInputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::DrawIndexed(36, 0, 0);

        Performance::EndEvent();
	}

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RaycastScalableVolumeWithHighlight()
    {
        glm::mat4 ReconstructionToSaltwater = glm::mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, -1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );

        Performance::BeginEvent("Raycasting with hightlighting for rendering");

        ContextManager::SetTargetSet(TargetSetManager::GetDeferredTargetSet());

        MR::CScalableSLAMReconstructor::SScalableVolume& rVolume = m_pScalableReconstructor->GetVolume();

        MR::SReconstructionSettings Settings;
        m_pScalableReconstructor->GetReconstructionSettings(&Settings);

        glm::mat4 PoseMatrix = m_pScalableReconstructor->GetPoseMatrix();

        ContextManager::SetShaderVS(m_RaycastVSPtr);
        ContextManager::SetShaderPS(m_RaycastHighlightFSPtr);

        ContextManager::SetResourceBuffer(0, rVolume.m_RootVolumePoolPtr);
        ContextManager::SetResourceBuffer(1, rVolume.m_RootGridPoolPtr);
        ContextManager::SetResourceBuffer(2, rVolume.m_Level1PoolPtr);
        ContextManager::SetResourceBuffer(3, rVolume.m_TSDFPoolPtr);
        ContextManager::SetResourceBuffer(6, rVolume.m_RootVolumePositionBufferPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_RaycastHighLightConstantBufferPtr);
        ContextManager::SetConstantBuffer(2, rVolume.m_AABBBufferPtr);

        ContextManager::Barrier();

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::Default));
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        const glm::vec3 Min = glm::vec3(
            rVolume.m_MinOffset[0] * Settings.m_VolumeSize,
            rVolume.m_MinOffset[1] * Settings.m_VolumeSize,
            rVolume.m_MinOffset[2] * Settings.m_VolumeSize
        );

        const glm::vec3 Max = glm::vec3(
            (rVolume.m_MaxOffset[0] + 1.0f) * Settings.m_VolumeSize, // Add 1.0f because MaxOffset stores the max volume offset
            (rVolume.m_MaxOffset[1] + 1.0f) * Settings.m_VolumeSize, // and we have to consider the volume size
            (rVolume.m_MaxOffset[2] + 1.0f) * Settings.m_VolumeSize
        );

        glm::vec3 Vertices[8] =
        {
            glm::vec3(glm::eulerAngleX(glm::half_pi<float>()) * glm::vec4(Min[0], Min[1], Min[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::half_pi<float>()) * glm::vec4(Max[0], Min[1], Min[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::half_pi<float>()) * glm::vec4(Max[0], Max[1], Min[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::half_pi<float>()) * glm::vec4(Min[0], Max[1], Min[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::half_pi<float>()) * glm::vec4(Min[0], Min[1], Max[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::half_pi<float>()) * glm::vec4(Max[0], Min[1], Max[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::half_pi<float>()) * glm::vec4(Max[0], Max[1], Max[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::half_pi<float>()) * glm::vec4(Min[0], Max[1], Max[2], 1.0f))
        };

        glm::mat4 InvOBBMatrix = glm::inverse(m_SelectionTransform) * ReconstructionToSaltwater;
        
        BufferManager::UploadBufferData(m_RaycastHighLightConstantBufferPtr, &InvOBBMatrix);

        BufferManager::UploadBufferData(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer(), &Vertices);

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);
        ContextManager::SetInputLayout(m_VolumeInputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::DrawIndexed(36, 0, 0);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RaycastScalableVolumeDiminished()
    {
        if (m_SelectionState == ESelection::NOSELECTION) return;

        glm::mat4 ReconstructionToSaltwater = glm::mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, -1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );

        Performance::BeginEvent("Raycasting for diminishing");

        ContextManager::SetTargetSet(m_DiminishedTargetSetPtr);

        MR::CScalableSLAMReconstructor::SScalableVolume& rVolume = m_pScalableReconstructor->GetVolume();
        
        ContextManager::SetShaderVS(m_RaycastDiminishedVSPtr);
        ContextManager::SetShaderPS(m_RaycastDiminishedFSPtr);

        ContextManager::SetResourceBuffer(0, rVolume.m_RootVolumePoolPtr);
        ContextManager::SetResourceBuffer(1, rVolume.m_RootGridPoolPtr);
        ContextManager::SetResourceBuffer(2, rVolume.m_Level1PoolPtr);
        ContextManager::SetResourceBuffer(3, rVolume.m_TSDFPoolPtr);
        ContextManager::SetResourceBuffer(6, rVolume.m_RootVolumePositionBufferPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_RaycastHighLightConstantBufferPtr);

        ContextManager::SetConstantBuffer(2, rVolume.m_AABBBufferPtr);
        ContextManager::SetConstantBuffer(3, m_IntrinsicsConstantBufferPtr);

        ContextManager::Barrier();

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::Default));
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        const glm::vec3 Min = glm::translate(glm::vec3(-0.7f)) * glm::vec4(m_SelectionBoxMin, 1.0f);
        const glm::vec3 Max = glm::translate(glm::vec3(+0.7f)) * glm::vec4(m_SelectionBoxMax, 1.0f);

        glm::vec3 Vertices[8] =
        {
            glm::vec3(glm::vec4(Min[0], Min[1], Min[2], 1.0f)),
            glm::vec3(glm::vec4(Max[0], Min[1], Min[2], 1.0f)),
            glm::vec3(glm::vec4(Max[0], Max[1], Min[2], 1.0f)),
            glm::vec3(glm::vec4(Min[0], Max[1], Min[2], 1.0f)),
            glm::vec3(glm::vec4(Min[0], Min[1], Max[2], 1.0f)),
            glm::vec3(glm::vec4(Max[0], Min[1], Max[2], 1.0f)),
            glm::vec3(glm::vec4(Max[0], Max[1], Max[2], 1.0f)),
            glm::vec3(glm::vec4(Min[0], Max[1], Max[2], 1.0f))
        };

        glm::mat4 InvOBBMatrix = glm::inverse(m_SelectionTransform) * ReconstructionToSaltwater;

        BufferManager::UploadBufferData(m_RaycastHighLightConstantBufferPtr, &InvOBBMatrix);

        BufferManager::UploadBufferData(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer(), &Vertices);

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);
        ContextManager::SetInputLayout(m_VolumeInputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::DrawIndexed(36, 0, 0);

        Performance::EndEvent();
    }

	// -----------------------------------------------------------------------------

	void CGfxReconstructionRenderer::RenderQueuedRootVolumes()
	{
		ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

		ContextManager::SetRenderContext(m_OutlineRenderContextPtr);
		ContextManager::SetShaderVS(m_OutlineVSPtr);
		ContextManager::SetShaderPS(m_OutlineFSPtr);

		ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
		ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);

		SDrawCallConstantBuffer BufferData;

		ContextManager::SetVertexBuffer(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
		ContextManager::SetInputLayout(m_CubeOutlineInputLayoutPtr);

		ContextManager::SetTopology(STopology::LineList);

        glm::vec3 Position;
		glm::mat4 Scaling;
		glm::mat4 Translation;

        const auto& GridSizes = m_pScalableReconstructor->GetVolumeSizes();

		for (auto& rPair : m_pScalableReconstructor->GetRootVolumeMap())
		{
			auto& rRootGrid = rPair.second;

			if (rRootGrid.m_IsVisible)
			{
                Position[0] = static_cast<float>(rRootGrid.m_Offset[0]);
                Position[1] = static_cast<float>(rRootGrid.m_Offset[1]);
                Position[2] = static_cast<float>(rRootGrid.m_Offset[2]);

                Position = Position * GridSizes[0];

                Scaling = glm::scale(glm::vec3(GridSizes[0]));
                Translation = glm::translate(Position);

                BufferData.m_WorldMatrix = Translation * Scaling;
                BufferData.m_WorldMatrix = glm::eulerAngleX(glm::radians(90.0f)) * BufferData.m_WorldMatrix;
                BufferData.m_Color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

                BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

                ContextManager::Draw(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface()->GetNumberOfVertices(), 0);
			}
		}
	}

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderQueuedLevel1Grids()
    {
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetRenderContext(m_OutlineRenderContextPtr);
        ContextManager::SetShaderVS(m_OutlineLevel1VSPtr);
        ContextManager::SetShaderPS(m_OutlineLevel1FSPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);

        SDrawCallConstantBuffer BufferData;

        ContextManager::SetVertexBuffer(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetInputLayout(m_CubeOutlineInputLayoutPtr);

        ContextManager::SetTopology(STopology::LineList);

        glm::vec3 Position;
        glm::mat4 Scaling;
        glm::mat4 Translation;

        const auto& VolumeSizes = m_pScalableReconstructor->GetVolumeSizes();

        for (auto& rPair : m_pScalableReconstructor->GetRootVolumeMap())
        {
            auto& rRootGrid = rPair.second;

            if (rRootGrid.m_IsVisible)
            {
                Position[0] = static_cast<float>(rRootGrid.m_Offset[0]);
                Position[1] = static_cast<float>(rRootGrid.m_Offset[1]);
                Position[2] = static_cast<float>(rRootGrid.m_Offset[2]);

                Position = Position * VolumeSizes[0];

                Scaling = glm::scale(glm::vec3(VolumeSizes[1]));
                Translation = glm::translate(Position);

                BufferData.m_WorldMatrix = Translation * Scaling;
                BufferData.m_WorldMatrix = glm::eulerAngleX(glm::radians(90.0f)) * BufferData.m_WorldMatrix;
                BufferData.m_Color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

                BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

                assert(rRootGrid.m_Level1QueuePtr != nullptr);
                ContextManager::SetResourceBuffer(2, rRootGrid.m_Level1QueuePtr);

                int VertexCount = m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface()->GetNumberOfVertices();
                BufferManager::UploadBufferData(rRootGrid.m_IndirectLevel1Buffer, &VertexCount, 0, sizeof(uint32_t));

                ContextManager::DrawIndirect(rRootGrid.m_IndirectLevel1Buffer, MR::CScalableSLAMReconstructor::SIndirectBuffers::s_DrawOffset);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderQueuedLevel2Grids()
    {
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetRenderContext(m_OutlineRenderContextPtr);
        ContextManager::SetShaderVS(m_OutlineLevel2VSPtr);
        ContextManager::SetShaderPS(m_OutlineLevel2FSPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);

        SDrawCallConstantBuffer BufferData;

        ContextManager::SetVertexBuffer(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetInputLayout(m_CubeOutlineInputLayoutPtr);

        ContextManager::SetTopology(STopology::LineList);

        glm::vec3 Position;
        glm::mat4 Translation;

        const auto& VolumeSizes = m_pScalableReconstructor->GetVolumeSizes();

        for (auto& rPair : m_pScalableReconstructor->GetRootVolumeMap())
        {
            auto& rRootGrid = rPair.second;

            if (rRootGrid.m_IsVisible)
            {
                Position[0] = static_cast<float>(rRootGrid.m_Offset[0]);
                Position[1] = static_cast<float>(rRootGrid.m_Offset[1]);
                Position[2] = static_cast<float>(rRootGrid.m_Offset[2]);

                Position = Position * VolumeSizes[0];

                Translation = glm::translate(Position);

                BufferData.m_WorldMatrix = Translation;
                BufferData.m_WorldMatrix = glm::eulerAngleX(glm::radians(90.0f)) * BufferData.m_WorldMatrix;
                BufferData.m_Color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

                BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

                assert(rRootGrid.m_Level2QueuePtr != nullptr);
                ContextManager::SetResourceBuffer(2, rRootGrid.m_Level2QueuePtr);

                int VertexCount = m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface()->GetNumberOfVertices();
                BufferManager::UploadBufferData(rRootGrid.m_IndirectLevel2Buffer, &VertexCount, 0, sizeof(uint32_t));

                ContextManager::DrawIndirect(rRootGrid.m_IndirectLevel2Buffer, MR::CScalableSLAMReconstructor::SIndirectBuffers::s_DrawOffset);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderCamera()
    {
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetRenderContext(m_OutlineRenderContextPtr);
        ContextManager::SetShaderVS(m_OutlineVSPtr);
        ContextManager::SetShaderPS(m_OutlineFSPtr);

		SDrawCallConstantBuffer BufferData;

        glm::mat4 PoseMatrix = m_pScalableReconstructor->GetPoseMatrix();
        
        BufferData.m_WorldMatrix = glm::eulerAngleX(glm::half_pi<float>()) * PoseMatrix;
		BufferData.m_Color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);

		BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_CameraMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_CameraMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);

        ContextManager::SetInputLayout(m_CameraInputLayoutPtr);
        ContextManager::SetTopology(STopology::LineList);

        ContextManager::Draw(m_CameraMeshPtr->GetLOD(0)->GetSurface()->GetNumberOfVertices(), 0);
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderPlanes()
    {
        if (m_pScalableReconstructor->GetPlanes().empty())
        {
            return;
        }

        Performance::BeginEvent("Plane rendering");
        
        ContextManager::SetRenderContext(m_PlaneRenderContextPtr);
        ContextManager::SetShaderVS(m_OutlineVSPtr);
        ContextManager::SetShaderPS(m_OutlineFSPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_PlaneMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_PlaneMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);

        ContextManager::SetInputLayout(m_CameraInputLayoutPtr);
        ContextManager::SetTopology(STopology::TriangleList);

        SDrawCallConstantBuffer BufferData;
        
        for (const auto& Plane : m_pScalableReconstructor->GetPlanes())
        {
            BufferData.m_WorldMatrix = Plane.second.m_Transform * glm::scale(glm::vec3(Plane.second.m_Extent));
            BufferData.m_Color = glm::vec4(1.0f, 1.0f, 0.0f, 0.3f);

            BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

            ContextManager::DrawIndexed(m_PlaneMeshPtr->GetLOD(0)->GetSurface()->GetNumberOfIndices(), 0, 0);
        }

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderVertexMap()
    {
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));
        ContextManager::SetRenderContext(m_OutlineRenderContextPtr);

        ContextManager::SetShaderVS(m_PointCloudVSPtr);
        ContextManager::SetShaderPS(m_PointCloudFSPtr);

        SDrawCallConstantBuffer BufferData;

        glm::mat4 PoseMatrix = m_pScalableReconstructor->GetPoseMatrix();
        
        BufferData.m_WorldMatrix = glm::eulerAngleX(glm::half_pi<float>()) * PoseMatrix;
        BufferData.m_Color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

        BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);

        ContextManager::SetImageTexture(0, m_pScalableReconstructor->GetVertexMap());
        ContextManager::SetImageTexture(1, m_pScalableReconstructor->GetColorMap());

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_CameraMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_CameraMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);

        ContextManager::SetInputLayout(m_CameraInputLayoutPtr);
        ContextManager::SetTopology(STopology::PointList);

        glm::ivec2 DepthSize = m_pScalableReconstructor->GetDepthImageSize();

        ContextManager::Draw(DepthSize.x * DepthSize.y, 0);
    }

    // -----------------------------------------------------------------------------

    glm::vec3 CGfxReconstructionRenderer::Pick(const glm::ivec2& _rCursorPosition)
    {
        Performance::BeginEvent("Picking TSDF");
        
        glm::mat3 SaltwaterToReconstruction = glm::mat3(
            1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 1.0f, 0.0f
        );

        glm::mat3 ReconstructionToSaltwater = glm::mat3(
            1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, -1.0f, 0.0f
        );

        MR::CScalableSLAMReconstructor::SScalableVolume& rVolume = m_pScalableReconstructor->GetVolume();

        const glm::ivec2 WindowSize = Gfx::Main::GetActiveWindowSize();
        const glm::vec3 CameraPosition = Gfx::ViewManager::GetMainCamera()->GetView()->GetPosition();
        const glm::mat4 ViewProjectionMatrix = Gfx::ViewManager::GetMainCamera()->GetViewProjectionMatrix();

        glm::ivec2 Cursor;
        Cursor.x = _rCursorPosition.y;
        Cursor.y = WindowSize.y - _rCursorPosition.x;

        glm::vec4 CSCursorPosition = glm::vec4(glm::vec2(Cursor) / glm::vec2(WindowSize) * 2.0f - 1.0f, 1.0f, 1.0f);
        glm::mat4 InvViewProjectionMatrix = glm::inverse(ViewProjectionMatrix);

        glm::vec4 WSCursorPosition = InvViewProjectionMatrix * CSCursorPosition;
        WSCursorPosition /= WSCursorPosition.w;

        SPickingBuffer PickingData;
        PickingData.m_RayStart = glm::vec4(SaltwaterToReconstruction * CameraPosition, 1.0f);
        PickingData.m_RayDirection = glm::vec4(SaltwaterToReconstruction * glm::normalize(glm::vec3(WSCursorPosition) - CameraPosition), 0.0f);
        PickingData.m_WorldHitPosition = glm::vec4(0.0f);
        BufferManager::UploadBufferData(m_PickingBuffer, &PickingData);

        ContextManager::SetShaderCS(m_PickingCSPtr);

        ContextManager::SetResourceBuffer(0, rVolume.m_RootVolumePoolPtr);
        ContextManager::SetResourceBuffer(1, rVolume.m_RootGridPoolPtr);
        ContextManager::SetResourceBuffer(2, rVolume.m_Level1PoolPtr);
        ContextManager::SetResourceBuffer(3, rVolume.m_TSDFPoolPtr);
        ContextManager::SetResourceBuffer(6, rVolume.m_RootVolumePositionBufferPtr);
        ContextManager::SetResourceBuffer(7, m_PickingBuffer);

        ContextManager::SetConstantBuffer(2, rVolume.m_AABBBufferPtr);

        ContextManager::Barrier();
        
        ContextManager::Dispatch(1, 1, 1);

        Performance::EndEvent();

        void* pBufferData = BufferManager::MapBuffer(m_PickingBuffer, Gfx::CBuffer::Read);
        PickingData = *(static_cast<SPickingBuffer*>(pBufferData));
        BufferManager::UnmapBuffer(m_PickingBuffer);

        ContextManager::ResetShaderCS();

        return glm::vec3(ReconstructionToSaltwater * PickingData.m_WorldHitPosition);
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::SetSelectionBox(const glm::vec3& _rAnchor0, const glm::vec3& _rAnchor1, float _Height, int _State)
    {
        BASE_UNUSED(_Height);

        glm::vec3 Diagonal = _rAnchor1 - _rAnchor0;
        glm::vec3 NDiagonal = glm::normalize(Diagonal);

        glm::vec3 Position = _rAnchor0;
        glm::mat4 Scaling;
        glm::mat4 Translation;
        glm::mat4 Rotation;

        Scaling = glm::scale(glm::vec3(glm::length(Diagonal) / glm::sqrt(2.0f)));
        Translation = glm::translate(Position);

        glm::vec3 Direction = glm::mat3(glm::eulerAngleZ(-glm::pi<float>() / 4.0f)) * NDiagonal;
        float Angle = std::atan2(Direction.y, Direction.x); // TODO: find out why glm::atan2 does lead to a compiler error
        Rotation = glm::eulerAngleZ(Angle);

        m_SelectionBoxMin = _rAnchor0;
        m_SelectionBoxMax = _rAnchor1;
        m_SelectionBoxMax.z += glm::length(Diagonal) / glm::sqrt(2.0f);

        m_SelectionTransform = Translation * Scaling * Rotation;
        m_SelectionState = static_cast<ESelection>(_State);
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::Render()
    {
        if (!m_IsInitialized && !m_pScalableReconstructor->IsInitialized())
        {
            return;
        }

        if (!m_IsInitialized && m_pScalableReconstructor->IsInitialized())
        {
            Initialize();
        }

        glEnable(GL_PROGRAM_POINT_SIZE);
        Performance::BeginEvent("SLAM Reconstruction Rendering");

        ContextManager::SetViewPortSet(ViewManager::GetViewPortSet());
        ContextManager::SetTargetSet(TargetSetManager::GetDeferredTargetSet());
        //glm::vec4 ClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        //TargetSetManager::ClearTargetSet(TargetSetManager::GetDeferredTargetSet(), ClearColor);

        if (m_RenderVolume)
        {
            if (m_SelectionState == ESelection::NOSELECTION)
            {
                RaycastScalableVolume();
            }
            else
            {
                RaycastScalableVolumeWithHighlight();
            }

            RaycastScalableVolumeDiminished();
        }

        Performance::EndEvent();

        ContextManager::ResetShaderVS();
        ContextManager::ResetShaderPS();
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderForward()
    {
        if (!m_IsInitialized && !m_pScalableReconstructor->IsInitialized())
        {
            return;
        }
        
        if (m_RenderVolumeVertexMap)
        {
            RenderVolumeVertexMap();
        }

        if (!m_UseTrackingCamera)
        {
            RenderCamera();
        }

        if (m_RenderVolumeVertexMap)
        {
            RenderVolumeVertexMap();
        }

        if (m_RenderVertexMap)
        {
            RenderVertexMap();
        }

        if (m_RenderRootQueue)
        {
            RenderQueuedRootVolumes();
        }

        if (m_RenderLevel1Queue)
        {
            RenderQueuedLevel1Grids();
        }

        if (m_RenderLevel2Queue)
        {
            RenderQueuedLevel2Grids();
        }

        RenderSelectionBox();

        if (m_RenderPlanes)
        {
            RenderPlanes();
        }

        ContextManager::ResetShaderVS();
        ContextManager::ResetShaderPS();
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::ChangeCamera(bool _IsTrackingCamera)
    {
        m_UseTrackingCamera = _IsTrackingCamera;
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::SetReconstructor(MR::CScalableSLAMReconstructor& _rReconstructor)
    {
        m_pScalableReconstructor = &_rReconstructor;
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::SetIntrinsics(const glm::vec2& _FocalLength, const glm::vec2& _FocalPoint)
    {
        const float FocalLengthX = _FocalLength.x;
        const float FocalLengthY = _FocalLength.y;
        const float FocalPointX  = _FocalPoint.x;
        const float FocalPointY  = _FocalPoint.y;

        glm::mat4 KMatrix(
            FocalLengthX, 0.0f        , 0.0f, 0.0f,
            0.0f        , FocalLengthY, 0.0f, 0.0f,
            FocalPointX , FocalPointY , 1.0f, 0.0f,
            0.0f        , 0.0f        , 0.0f, 1.0f
        );

        m_Intrinsics.m_FocalPoint     = glm::vec2(FocalPointX, FocalPointY);
        m_Intrinsics.m_FocalLength    = glm::vec2(FocalLengthX, FocalLengthY);
        m_Intrinsics.m_InvFocalLength = glm::vec2(1.0f / FocalLengthX, 1.0f / FocalLengthY);
        m_Intrinsics.m_KMatrix        = KMatrix;
        m_Intrinsics.m_InvKMatrix     = glm::inverse(KMatrix);
    }

} // namespace

namespace Gfx
{
namespace ReconstructionRenderer
{
    void OnStart()
    {
        CGfxReconstructionRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxReconstructionRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxReconstructionRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxReconstructionRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxReconstructionRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxReconstructionRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxReconstructionRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxReconstructionRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxReconstructionRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxReconstructionRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxReconstructionRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxReconstructionRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxReconstructionRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxReconstructionRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxReconstructionRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxReconstructionRenderer::GetInstance().Render();
    }

    // -----------------------------------------------------------------------------

    void RenderForward()
    {
        CGfxReconstructionRenderer::GetInstance().RenderForward();
    }
    
    // -----------------------------------------------------------------------------

    void ChangeCamera(bool _IsTrackingCamera)
    {
        CGfxReconstructionRenderer::GetInstance().ChangeCamera(_IsTrackingCamera);
    }
    
    // -----------------------------------------------------------------------------

    void SetReconstructor(MR::CScalableSLAMReconstructor& _rReconstructor)
    {
        CGfxReconstructionRenderer::GetInstance().SetReconstructor(_rReconstructor);
    }

    // -----------------------------------------------------------------------------

    glm::vec3 Pick(const glm::ivec2& _rCursor)
    {
        return CGfxReconstructionRenderer::GetInstance().Pick(_rCursor);
    }

    // -----------------------------------------------------------------------------

    void SetSelectionBox(const glm::vec3& _rAnchor0, const glm::vec3& _rAnchor1, float _Height, int _State)
    {
        CGfxReconstructionRenderer::GetInstance().SetSelectionBox(_rAnchor0, _rAnchor1, _Height, _State);
    }
} // namespace ReconstructionRenderer
} // namespace Gfx

