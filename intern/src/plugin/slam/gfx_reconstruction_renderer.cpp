
#include "plugin/slam/slam_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/camera/cam_control_manager.h"
#include "engine/camera/cam_editor_control.h"

#include "engine/core/core_program_parameters.h"
#include "engine/core/core_time.h"

#include "engine/data/data_component_manager.h"
#include "engine/data/data_script_component.h"

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

#include "engine/script/script_slam.h"

#include "plugin/slam/gfx_reconstruction_renderer.h"
#include "plugin/slam/mr_slam_reconstructor.h"

#include "GL/glew.h"

#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

using namespace Base;
using namespace Gfx;

namespace
{
    const int g_TileSize2D = 16;

    int DivUp(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }

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
        void RenderHitProxy();
        void ChangeCamera(bool _IsTrackingCamera);

        void SetReconstructor(MR::CSLAMReconstructor& _rReconstructor);
        void SetDeviceResolution(const glm::ivec2& _Resolution);

        glm::vec3 Pick(const glm::ivec2& _rCursorPosition);

        void UpdateSelectionBox();
        void AddPositionToSelection(const glm::vec3& _rWSPosition);
        void ResetSelection();
        void SetInpaintedPlane(Gfx::CTexturePtr _Texture, const Base::AABB3Float& _rAABB);
        CTexturePtr GetInpaintedRendering(const glm::mat4& _rPoseMatrix, const Base::AABB3Float& _rAABB, CTexturePtr _BackgroundTexturePtr);

        const Base::AABB3Float& GetSelectionBox();

    private:

        glm::ivec2 m_DeviceResolution;

        // -----------------------------------------------------------------------------
        // Stuff for selection box
        // -----------------------------------------------------------------------------
        enum class ESelection
        {
            NOSELECTION,
            SELECTED,
        };

        // -----------------------------------------------------------------------------
        // Stuff for inpainted plane
        // -----------------------------------------------------------------------------
        Gfx::CTexturePtr m_InpaintedPlaneTexture;
        Base::AABB3Float m_InpaintedPlaneAABB;
        float m_InpaintedPlaneScale;

    private:

        void Initialize();
        
        void RaycastVolume();
        void RaycastVolumeWithHighlight();

        void RenderBackgroundImage(CTexturePtr _Background, bool _IsFlipped = false);
        void CombineDiminishedImage(CTexturePtr _Background, CTexturePtr _Diminished, bool _IsFlipped = false);
        void RaycastVolumeDiminished(const glm::mat4& _rPoseMatrix, const Base::AABB3Float& _rAABB, CTexturePtr _BackgroundTexture = nullptr);
        void RenderInpaintedPlane(const glm::mat4& _rPoseMatrix, const Base::AABB3Float& _rAABB);
        
        void CreateMembrane(CTexturePtr _BackgroundTexturePtr, CTexturePtr _Diminished);

        void RenderQueuedRootVolumes();
        void RenderQueuedLevel1Grids();
        void RenderQueuedLevel2Grids();

        void RenderCamera();

        void RenderPlanes();

        void RenderVertexMap();

        void RenderSelectionBox();

    private:

        MR::CSLAMReconstructor* m_pReconstructor;
        
        CShaderPtr m_OutlineVSPtr;
        CShaderPtr m_OutlineFSPtr;

        CShaderPtr m_VertexMapVSPtr;
        CShaderPtr m_VertexMapFSPtr;

        CShaderPtr m_OutlineLevel1VSPtr;
        CShaderPtr m_OutlineLevel1FSPtr;
        CShaderPtr m_OutlineLevel2VSPtr;
        CShaderPtr m_OutlineLevel2FSPtr;
        
        CShaderPtr m_RaycastVSPtr;
        CShaderPtr m_RaycastDiminishedVSPtr;
        CShaderPtr m_RaycastFSPtr;
        CShaderPtr m_RaycastHighlightFSPtr;
        CShaderPtr m_RaycastDiminishedFSPtr;
        CShaderPtr m_RaycastHitProxyFSPtr;

        CShaderPtr m_InpaintedPlaneVSPtr;
        CShaderPtr m_InpaintedPlaneFSPtr;

        CShaderPtr m_BackgroundVSPtr;
        CShaderPtr m_BackgroundFSPtr;
        CShaderPtr m_CombineDiminishedFSPtr;
        
        CShaderPtr m_MembranePatchesCSPtr;
        CShaderPtr m_MembraneBorderCSPtr;
        CShaderPtr m_MembraneEvalBorderCSPtr;
        CShaderPtr m_MembranePropagateGridCSPtr;
        CShaderPtr m_MembranePropagatePixelsCSPtr;

        CBufferPtr m_RaycastConstantBufferPtr;
        CBufferPtr m_RaycastHitProxyBufferPtr;
        CBufferPtr m_RaycastHighLightConstantBufferPtr;
        CBufferPtr m_DrawCallConstantBufferPtr;

        CBufferPtr m_MembraneIndirectBufferPtr;
        CBufferPtr m_MembranePatchBufferPtr;
                
        CMeshPtr m_CameraMeshPtr;
        CInputLayoutPtr m_CameraInputLayoutPtr;

        CMeshPtr m_CubeOutlineMeshPtr;
        CInputLayoutPtr m_CubeOutlineInputLayoutPtr;

        CMeshPtr m_VolumeMeshPtr;        
        CInputLayoutPtr m_VolumeInputLayoutPtr;

        CMeshPtr m_InpaintedPlaneMeshPtr;
        CInputLayoutPtr m_InpaintedPlaneLayoutPtr;

        CMeshPtr m_FullscreenQuadMeshPtr;
        CInputLayoutPtr m_FullscreenQuadLayoutPtr;

        CRenderContextPtr m_OutlineRenderContextPtr;

        CMeshPtr m_PlaneMeshPtr;
        
        CShaderPtr m_PointCloudVSPtr;
        CShaderPtr m_PointCloudFSPtr;

        CShaderPtr m_PickingCSPtr;

        CTexturePtr m_DiminishedRaycastTargetPtr;
        CTexturePtr m_DiminishedPlaneTargetPtr;
        CTexturePtr m_DiminishedFinalTargetPtr;

        CTargetSetPtr m_DiminishedRaycastTargetSetPtr;
        CTargetSetPtr m_DiminishedPlaneTargetSetPtr;
        CTargetSetPtr m_DiminishedFinalTargetSetPtr;

        CViewPortSetPtr m_DiminishedViewPortSetPtr;

        CTexturePtr m_MembranePatchesTexturePtr; 
        CTexturePtr m_MembraneBordersTexturePtr;
        CTexturePtr m_MembraneTexturePtr;

        CBufferPtr m_PickingBuffer;

        bool m_UseTrackingCamera;

        bool m_RenderVolume;
        bool m_RenderVertexMap;
        bool m_RenderRootQueue;
        bool m_RenderLevel1Queue;
        bool m_RenderLevel2Queue;
        bool m_RenderBackSides;
        bool m_RenderPlanes;

        glm::mat4 m_SelectionTransform;
        ESelection m_SelectionState;

        Base::AABB3Float m_SelectionBox;

        bool m_IsInitialized;

        Gfx::Main::CResizeDelegate::HandleType m_ResizeDelegate;

        int m_PatchCount;
        int m_PatchSize;
        int m_MaxBorderPatchCount;

        bool m_IsInpainting = false;
    };
} // namespace

namespace
{
    using namespace Base;

    CGfxReconstructionRenderer::CGfxReconstructionRenderer()
        : m_UseTrackingCamera    (true)
        , m_RenderVolume         (true)
        , m_RenderVertexMap      (false)
        , m_RenderRootQueue      (false)
        , m_RenderLevel1Queue    (false)
        , m_RenderLevel2Queue    (false)
        , m_SelectionBox         ()
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

        m_ResizeDelegate = Gfx::Main::RegisterResizeHandler(std::bind(&CGfxReconstructionRenderer::OnResize, this, std::placeholders::_1, std::placeholders::_2));
                                
        m_UseTrackingCamera   = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:use_tracking_camera", true);
        m_RenderVolume        = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:volume"             , true);
        m_RenderVertexMap     = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:vertex_map"         , false);
        m_RenderRootQueue     = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:queues:root"        , false);
        m_RenderLevel1Queue   = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:queues:level1"      , false);
        m_RenderLevel2Queue   = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:queues:level2"      , false);
        m_RenderBackSides     = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:backsides"          , true);
        m_RenderPlanes        = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:planes"             , false);
        m_InpaintedPlaneScale = Core::CProgramParameters::GetInstance().Get("mr:diminished_reality:inpainted_plane:scale", 2.0f);

        if (Core::CProgramParameters::GetInstance().Get("mr:diminished_reality:aabb:load", false))
        {
            m_SelectionState = ESelection::SELECTED;
            m_SelectionBox.SetMin(Core::CProgramParameters::GetInstance().Get("mr:diminished_reality:aabb:min", glm::vec3(0.0f)));
            m_SelectionBox.SetMax(Core::CProgramParameters::GetInstance().Get("mr:diminished_reality:aabb:max", glm::vec3(0.0f)));
        }

        m_IsInitialized = false;

        m_MaxBorderPatchCount = Core::CProgramParameters::GetInstance().Get("mr:diminished_reality:rendering:max_border_patch_count", 128);
        m_PatchCount = Core::CProgramParameters::GetInstance().Get("mr:diminished_reality:rendering:patch_count", 16);
        m_PatchSize = Core::CProgramParameters::GetInstance().Get("mr:diminished_reality:rendering:patch_size", 16);
    }

    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::Initialize()
    {
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
        if (Core::CProgramParameters::GetInstance().Get("mr:diminished_reality:aabb:store", false))
        {
            Core::CProgramParameters::GetInstance().Add("mr:diminished_reality:aabb:min", m_SelectionBox.GetMin());
            Core::CProgramParameters::GetInstance().Add("mr:diminished_reality:aabb:max", m_SelectionBox.GetMax());
        }

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
        m_RaycastHitProxyFSPtr = 0;
        m_PickingCSPtr = 0;
        
        m_VertexMapVSPtr = 0;
        m_VertexMapFSPtr = 0;

        m_BackgroundVSPtr = 0;
        m_BackgroundFSPtr = 0;
        m_CombineDiminishedFSPtr = 0;

        m_MembranePatchesCSPtr = 0;
        m_MembraneBorderCSPtr = 0;
        m_MembraneEvalBorderCSPtr = 0;
        m_MembranePropagateGridCSPtr = 0;
        m_MembranePropagatePixelsCSPtr = 0;
        
        m_PickingBuffer = 0;

        m_MembranePatchesTexturePtr = 0;
        m_MembraneBordersTexturePtr = 0;
        m_MembraneTexturePtr = 0;

        m_RaycastConstantBufferPtr = 0;
        m_RaycastHitProxyBufferPtr = 0;
        m_RaycastHighLightConstantBufferPtr = 0;
        m_DrawCallConstantBufferPtr = 0;

        m_MembranePatchBufferPtr = 0;
        m_MembraneIndirectBufferPtr = 0;

        m_CameraMeshPtr = 0;
        m_VolumeMeshPtr = 0;
        m_InpaintedPlaneMeshPtr = 0;
        m_FullscreenQuadMeshPtr = 0;
        m_CubeOutlineMeshPtr = 0;
        m_PlaneMeshPtr = 0;
        m_CameraInputLayoutPtr = 0;
        m_VolumeInputLayoutPtr = 0;
        m_InpaintedPlaneLayoutPtr = 0;
        m_CubeOutlineInputLayoutPtr = 0;

        m_OutlineRenderContextPtr = 0;

        m_pReconstructor = nullptr;

        m_PointCloudVSPtr = 0;
        m_PointCloudFSPtr = 0;

        m_InpaintedPlaneVSPtr = 0;
        m_InpaintedPlaneFSPtr = 0;

        m_DiminishedViewPortSetPtr = nullptr;

        m_DiminishedRaycastTargetPtr = nullptr;
        m_DiminishedFinalTargetPtr = nullptr;
        m_DiminishedPlaneTargetPtr = nullptr;

        m_DiminishedRaycastTargetSetPtr = nullptr;
        m_DiminishedPlaneTargetSetPtr = nullptr;
        m_DiminishedFinalTargetSetPtr = nullptr;

        m_InpaintedPlaneTexture = nullptr;

        m_IsInitialized = false;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupShader()
    {
        MR::SReconstructionSettings Settings;

        const std::string InternalFormatString = Core::CProgramParameters::GetInstance().Get("mr:slam:map_format", "rgba16f");

        m_pReconstructor->GetReconstructionSettings(&Settings);

        glm::ivec2 DepthImageSize = m_pReconstructor->GetDepthImageSize();

        std::stringstream DefineStream;

        DefineStream
            << "#define TILE_SIZE_2D " << g_TileSize2D << " \n"
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
            << "#define MIN_TREE_WEIGHT " << Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:min_weight", 30) << " \n"
            << "#define MAX_BORDER_PATCH_COUNT " << m_MaxBorderPatchCount << " \n"
            << "#define PATCH_COUNT " << m_PatchCount << " \n"
            << "#define PATCH_SIZE " << m_PatchSize << " \n";

        if (Settings.m_CaptureColor)
        {
            DefineStream << "#define CAPTURE_COLOR\n";
        }
        if (m_RenderBackSides)
        {
            DefineStream << "#define RAYCAST_BACKSIDES\n";
        }

        std::string DefineString = DefineStream.str();

        m_OutlineVSPtr = ShaderManager::CompileVS("../../plugins/slam/scalable/rendering/vs_outline.glsl", "main", DefineString.c_str());
        m_OutlineFSPtr = ShaderManager::CompilePS("../../plugins/slam/scalable/rendering/fs_outline.glsl", "main", DefineString.c_str());
        m_OutlineLevel1VSPtr = ShaderManager::CompileVS("../../plugins/slam/scalable/rendering/vs_outline_level1.glsl", "main", DefineString.c_str());
        m_OutlineLevel1FSPtr = ShaderManager::CompilePS("../../plugins/slam/scalable/rendering/fs_outline_level1.glsl", "main", DefineString.c_str());
        m_OutlineLevel2VSPtr = ShaderManager::CompileVS("../../plugins/slam/scalable/rendering/vs_outline_level2.glsl", "main", DefineString.c_str());
        m_OutlineLevel2FSPtr = ShaderManager::CompilePS("../../plugins/slam/scalable/rendering/fs_outline_level2.glsl", "main", DefineString.c_str());

        m_PointCloudVSPtr = ShaderManager::CompileVS("../../plugins/slam/scalable/rendering/vs_point_cloud.glsl", "main", DefineString.c_str());
        m_PointCloudFSPtr = ShaderManager::CompilePS("../../plugins/slam/scalable/rendering/fs_point_cloud.glsl", "main", DefineString.c_str());

        m_RaycastVSPtr = ShaderManager::CompileVS("../../plugins/slam/scalable/rendering/vs_raycast.glsl", "main", DefineString.c_str());
        m_RaycastDiminishedVSPtr = ShaderManager::CompileVS("../../plugins/slam/scalable/rendering/vs_raycast_diminished.glsl", "main", DefineString.c_str());
        m_RaycastFSPtr = ShaderManager::CompilePS("../../plugins/slam/scalable/rendering/fs_raycast.glsl", "main", DefineString.c_str());
        m_RaycastHighlightFSPtr = ShaderManager::CompilePS("../../plugins/slam/scalable/rendering/fs_raycast_highlight.glsl", "main", DefineString.c_str());
        m_RaycastDiminishedFSPtr = ShaderManager::CompilePS("../../plugins/slam/scalable/rendering/fs_raycast_diminished.glsl", "main", DefineString.c_str());
        m_RaycastHitProxyFSPtr = ShaderManager::CompilePS("../../plugins/slam/scalable/rendering/fs_raycast_hitproxy.glsl", "main", DefineString.c_str());

        m_VertexMapVSPtr = ShaderManager::CompileVS("../../plugins/slam/scalable/rendering/vs_vertex_map.glsl", "main", DefineString.c_str());
        m_VertexMapFSPtr = ShaderManager::CompilePS("../../plugins/slam/scalable/rendering/fs_vertex_map.glsl", "main", DefineString.c_str());
        
        m_PickingCSPtr = ShaderManager::CompileCS("../../plugins/slam/scalable/cs_picking.glsl", "main", DefineString.c_str());

        m_InpaintedPlaneVSPtr = ShaderManager::CompileVS("../../plugins/slam/scalable/rendering/vs_inpainted_plane.glsl", "main", DefineString.c_str());
        m_InpaintedPlaneFSPtr = ShaderManager::CompilePS("../../plugins/slam/scalable/rendering/fs_inpainted_plane.glsl", "main", DefineString.c_str());

        m_BackgroundVSPtr = ShaderManager::CompileVS("../../plugins/slam/scalable/rendering/vs_background.glsl", "main", DefineString.c_str());
        m_BackgroundFSPtr = ShaderManager::CompilePS("../../plugins/slam/scalable/rendering/fs_background.glsl", "main", DefineString.c_str());

        m_CombineDiminishedFSPtr = ShaderManager::CompilePS("../../plugins/slam/scalable/rendering/fs_combine_diminished.glsl", "main", DefineString.c_str());

        m_MembranePatchesCSPtr = ShaderManager::CompileCS("../../plugins/slam/scalable/rendering/cs_membrane_patches.glsl", "main", DefineString.c_str());
        m_MembraneBorderCSPtr = ShaderManager::CompileCS("../../plugins/slam/scalable/rendering/cs_membrane_border.glsl", "main", DefineString.c_str());
        m_MembraneEvalBorderCSPtr = ShaderManager::CompileCS("../../plugins/slam/scalable/rendering/cs_membrane_eval_border.glsl", "main", DefineString.c_str());
        m_MembranePropagateGridCSPtr = ShaderManager::CompileCS("../../plugins/slam/scalable/rendering/cs_membrane_propagate_grid.glsl", "main", DefineString.c_str());
        m_MembranePropagatePixelsCSPtr = ShaderManager::CompileCS("../../plugins/slam/scalable/rendering/cs_membrane_propagate_pixels.glsl", "main", DefineString.c_str());

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

        SInputElementDescriptor QuadLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 16, CInputLayout::PerVertex, 0 },
            { "TEXCOORD", 1, CInputLayout::Float2Format, 0, 8, 16, CInputLayout::PerVertex, 0 },
        };

        m_InpaintedPlaneLayoutPtr = ShaderManager::CreateInputLayout(QuadLayout, sizeof(QuadLayout) / sizeof(QuadLayout[0]), m_InpaintedPlaneVSPtr);

        SInputElementDescriptor FullscreenQuadLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float2Format, 0, 0, 8, CInputLayout::PerVertex, 0 },
        };

        m_FullscreenQuadLayoutPtr = ShaderManager::CreateInputLayout(FullscreenQuadLayout, sizeof(FullscreenQuadLayout) / sizeof(FullscreenQuadLayout[0]), m_BackgroundVSPtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupKernels()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupRenderTargets()
    {
        assert(m_DeviceResolution.x != 0 && m_DeviceResolution.y != 0);

        STextureDescriptor TextureDescriptor = {};

        TextureDescriptor.m_NumberOfPixelsU  = m_DeviceResolution.x;
        TextureDescriptor.m_NumberOfPixelsV  = m_DeviceResolution.y;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_GenerateAllMipMaps;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding          = CTexture::RenderTarget | CTexture::ShaderResource;
        TextureDescriptor.m_Access           = CTexture::EAccess::CPURead;
        TextureDescriptor.m_Usage            = CTexture::EUsage::GPUToCPU;
        TextureDescriptor.m_Semantic         = CTexture::UndefinedSemantic;
        TextureDescriptor.m_Format           = CTexture::R8G8B8A8_BYTE;

        m_DiminishedRaycastTargetPtr = TextureManager::CreateTexture2D(TextureDescriptor);
        m_DiminishedPlaneTargetPtr = TextureManager::CreateTexture2D(TextureDescriptor);
        m_DiminishedFinalTargetPtr = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureManager::SetTextureLabel(m_DiminishedRaycastTargetPtr, "Diminished Raycast Texture");
        TextureManager::SetTextureLabel(m_DiminishedPlaneTargetPtr, "Diminished Plane Texture");
        TextureManager::SetTextureLabel(m_DiminishedFinalTargetPtr, "Diminished Final Texture");

        m_DiminishedRaycastTargetSetPtr = TargetSetManager::CreateTargetSet(m_DiminishedRaycastTargetPtr);
        m_DiminishedPlaneTargetSetPtr = TargetSetManager::CreateTargetSet(m_DiminishedPlaneTargetPtr);
        m_DiminishedFinalTargetSetPtr = TargetSetManager::CreateTargetSet(m_DiminishedFinalTargetPtr);

        TargetSetManager::SetTargetSetLabel(m_DiminishedRaycastTargetSetPtr, "Diminished Raycast Target Set");
        TargetSetManager::SetTargetSetLabel(m_DiminishedPlaneTargetSetPtr, "Diminished Plane Target Set");
        TargetSetManager::SetTargetSetLabel(m_DiminishedFinalTargetSetPtr, "Diminished Final Target Set");
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupStates()
    {
        assert(m_DeviceResolution.x != 0 && m_DeviceResolution.y != 0);

        Gfx::SViewPortDescriptor Desc;
        Desc.m_Width = static_cast<float>(m_DeviceResolution.x);
        Desc.m_Height = static_cast<float>(m_DeviceResolution.y);
        Desc.m_TopLeftX = 0;
        Desc.m_TopLeftY = 0;
        Desc.m_MinDepth = 0.0f;
        Desc.m_MaxDepth = 0.0f;
        CViewPortPtr ViewPort = ViewManager::CreateViewPort(Desc);
        m_DiminishedViewPortSetPtr = ViewManager::CreateViewPortSet(ViewPort);

        m_OutlineRenderContextPtr = ContextManager::CreateRenderContext();
        m_OutlineRenderContextPtr->SetCamera(ViewManager::GetMainCamera());
        m_OutlineRenderContextPtr->SetViewPortSet(ViewManager::GetViewPortSet());
        m_OutlineRenderContextPtr->SetTargetSet(TargetSetManager::GetLightAccumulationTargetSet());
        m_OutlineRenderContextPtr->SetRenderState(StateManager::GetRenderState(CRenderState::NoCull | CRenderState::Wireframe));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupTextures()
    {
        STextureDescriptor TextureDescriptor = {};

        TextureDescriptor.m_NumberOfPixelsU = m_PatchSize * m_PatchCount;
        TextureDescriptor.m_NumberOfPixelsV = m_PatchSize * m_PatchCount;
        TextureDescriptor.m_NumberOfPixelsW = 1;
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding = CTexture::RenderTarget | CTexture::ShaderResource;
        TextureDescriptor.m_Access = CTexture::EAccess::CPURead;
        TextureDescriptor.m_Usage = CTexture::EUsage::GPUToCPU;
        TextureDescriptor.m_Semantic = CTexture::UndefinedSemantic;
        TextureDescriptor.m_Format = CTexture::R16G16B16A16_FLOAT;

        m_MembranePatchesTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);
        m_MembraneBordersTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureManager::SetTextureLabel(m_MembranePatchesTexturePtr, "Membrane Patches Texture");
        TextureManager::SetTextureLabel(m_MembraneBordersTexturePtr, "Membrane Borders Texture");

        TextureDescriptor.m_NumberOfPixelsU = m_PatchCount;
        TextureDescriptor.m_NumberOfPixelsV = m_PatchCount;

        m_MembraneTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);
        TextureManager::SetTextureLabel(m_MembraneTexturePtr, "Membrane Final Texture");

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

        ConstantBufferDesc.m_NumberOfBytes = sizeof(int);

        m_RaycastHitProxyBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

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

        ConstantBufferDesc.m_Stride = 0;
        ConstantBufferDesc.m_Usage = CBuffer::GPURead;
        ConstantBufferDesc.m_Binding = CBuffer::ResourceBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = m_MaxBorderPatchCount * 2 * sizeof(glm::vec4);
        ConstantBufferDesc.m_pBytes = nullptr;
        ConstantBufferDesc.m_pClassKey = 0;

        m_MembranePatchBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_NumberOfBytes = 4 * sizeof(int32_t);

        m_MembraneIndirectBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
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

        glm::ivec2 DepthSize = m_pReconstructor->GetDepthImageSize();
        glm::vec4 Intrinsics = m_pReconstructor->GetDepthIntrinsics();
        
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

        m_CameraMeshPtr = MeshManager::CreateMesh(CameraLines, sizeof(CameraLines) / sizeof(CameraLines[0]), sizeof(CameraVertices[0]), nullptr, 0);

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
        
        m_VolumeMeshPtr = MeshManager::CreateMesh(g_CubeVertices, sizeof(g_CubeVertices) / sizeof(g_CubeVertices[0]), sizeof(g_CubeVertices), CubeIndices, sizeof(CubeIndices) / sizeof(CubeIndices[0]));

        ////////////////////////////////////////////////////////////////////////////////
        // Create cube outline mesh
        ////////////////////////////////////////////////////////////////////////////////

        m_CubeOutlineMeshPtr = MeshManager::CreateMesh(CubeLines, sizeof(CubeLines) / sizeof(CubeLines[0]), sizeof(CubeLines[0]), nullptr, 0);

        ////////////////////////////////////////////////////////////////////////////////
        // Create plane mesh
        ////////////////////////////////////////////////////////////////////////////////

        glm::vec3 QuadLines[4] =
        {
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 1.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f),
        };

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

        int PlaneVertexCount = static_cast<int>(PlaneVertices.size());
        int PlaneIndexCount = static_cast<int>(PlaneVertices.size());
        m_PlaneMeshPtr = MeshManager::CreateMesh(PlaneVertices.data(), PlaneVertexCount, sizeof(PlaneVertices[0]), Indices.data(), PlaneIndexCount);

        ////////////////////////////////////////////////////////////////////////////////
        // Create quad mesh
        ////////////////////////////////////////////////////////////////////////////////
        struct SQuadVertex
        {
            glm::vec2 m_Pos;
            glm::vec2 m_Tex;
        };

        SQuadVertex Quad[4] =
        {
            { glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 0.0f) },
            { glm::vec2( 1.0f, -1.0f), glm::vec2(1.0f, 0.0f) },
            { glm::vec2(-1.0f,  1.0f), glm::vec2(0.0f, 1.0f) },
            { glm::vec2( 1.0f,  1.0f), glm::vec2(1.0f, 1.0f) },
        };

        m_InpaintedPlaneMeshPtr = MeshManager::CreateMesh(Quad, sizeof(Quad) / sizeof(Quad[0]), sizeof(Quad[0]), nullptr, 0);

        m_FullscreenQuadMeshPtr = MeshManager::CreateRectangle(-1.0f, -1.0f, 2.0f, 2.0f);
    }

    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnReload()
    {
        m_pReconstructor->ResetReconstruction();

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
        BASE_UNUSED(_Width);
        BASE_UNUSED(_Height);
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::Update()
    {
        UpdateSelectionBox();
    }
    
    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderSelectionBox()
    {
        if (m_SelectionState == ESelection::NOSELECTION)
        {
            return;
        }

        Performance::BeginEvent("Render selection box");

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

        Performance::EndEvent();
    }
        
    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RaycastVolume()
    {
        Performance::BeginEvent("Raycasting for rendering");

        ContextManager::SetTargetSet(TargetSetManager::GetDeferredTargetSet());

        MR::CSLAMReconstructor::SSLAMVolume& rVolume = m_pReconstructor->GetVolume();

        MR::SReconstructionSettings Settings;
        m_pReconstructor->GetReconstructionSettings(&Settings);

        glm::mat4 PoseMatrix = m_pReconstructor->GetPoseMatrix();

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
            Color = m_pReconstructor->IsTrackingLost() ? glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }
        else
        {
            Color = m_pReconstructor->IsTrackingLost() ? glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) : glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
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

    void CGfxReconstructionRenderer::RaycastVolumeWithHighlight()
    {
        glm::mat4 ReconstructionToSaltwater = glm::mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, -1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );

        Performance::BeginEvent("Raycasting with hightlighting for rendering");

        ContextManager::SetTargetSet(TargetSetManager::GetDeferredTargetSet());

        MR::CSLAMReconstructor::SSLAMVolume& rVolume = m_pReconstructor->GetVolume();

        MR::SReconstructionSettings Settings;
        m_pReconstructor->GetReconstructionSettings(&Settings);

        glm::mat4 PoseMatrix = m_pReconstructor->GetPoseMatrix();

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

    void CGfxReconstructionRenderer::RaycastVolumeDiminished(const glm::mat4& _rPoseMatrix, const Base::AABB3Float& _rAABB, CTexturePtr _BackgroundTexturePtr)
    {
        glm::mat4 ReconstructionToSaltwater = glm::mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, -1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );

        MR::CSLAMReconstructor::SSLAMVolume& rVolume = m_pReconstructor->GetVolume();
        
        ContextManager::SetShaderVS(m_RaycastDiminishedVSPtr);
        ContextManager::SetShaderPS(m_RaycastDiminishedFSPtr);

        ContextManager::SetTexture(0, _BackgroundTexturePtr);

        ContextManager::SetImageTexture(0, m_DiminishedPlaneTargetPtr);

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

        const glm::vec3 Min = _rAABB.GetMin() - 20.0f;
        const glm::vec3 Max = _rAABB.GetMax() + 20.0f;

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
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderBackgroundImage(CTexturePtr _Background, bool _IsFlipped)
    {
        assert(_Background != nullptr);

        Performance::BeginEvent("Render background");

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetShaderVS(m_BackgroundVSPtr);
        ContextManager::SetShaderPS(m_BackgroundFSPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetTexture(0, _Background);

        SDrawCallConstantBuffer BufferData = {};
        BufferData.m_Color = glm::vec4(_IsFlipped ? 1.0f : 0.0f);

        BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

        ContextManager::SetConstantBuffer(0, m_DrawCallConstantBufferPtr);

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_FullscreenQuadMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_FullscreenQuadMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);

        ContextManager::SetInputLayout(m_FullscreenQuadLayoutPtr);
        ContextManager::SetTopology(STopology::TriangleStrip);

        ContextManager::DrawIndexed(m_FullscreenQuadMeshPtr->GetLOD(0)->GetSurface()->GetNumberOfIndices(), 0, 0);

        ContextManager::ResetTexture(0);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::CombineDiminishedImage(CTexturePtr _Background, CTexturePtr _Diminished, bool _IsFlipped)
    {
        assert(_Background != nullptr);
        assert(_Diminished != nullptr);

        Performance::BeginEvent("Combine diminished image");

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetShaderVS(m_BackgroundVSPtr);
        ContextManager::SetShaderPS(m_CombineDiminishedFSPtr);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(CSampler::MinMagMipLinearClamp));

        ContextManager::SetTexture(0, _Background);        
        ContextManager::SetTexture(1, _Diminished);
        ContextManager::SetTexture(2, m_MembraneTexturePtr);

        SDrawCallConstantBuffer BufferData = {};
        BufferData.m_Color = glm::vec4(_IsFlipped ? 1.0f : 0.0f);

        BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

        ContextManager::SetConstantBuffer(0, m_DrawCallConstantBufferPtr);

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_FullscreenQuadMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_FullscreenQuadMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);

        ContextManager::SetInputLayout(m_FullscreenQuadLayoutPtr);
        ContextManager::SetTopology(STopology::TriangleStrip);

        ContextManager::DrawIndexed(m_FullscreenQuadMeshPtr->GetLOD(0)->GetSurface()->GetNumberOfIndices(), 0, 0);

        ContextManager::ResetTexture(0);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::CreateMembrane(CTexturePtr _BackgroundTexturePtr, CTexturePtr _Diminished)
    {
        // -----------------------------------------------------------------------------
        // Prepare membrane data
        // -----------------------------------------------------------------------------

        const int WorkGroupsX = m_PatchCount;
        const int WorkGroupsY = m_PatchCount;

        TextureManager::ClearTexture(m_MembranePatchesTexturePtr);
        TextureManager::ClearTexture(m_MembraneBordersTexturePtr);
        TextureManager::ClearTexture(m_MembraneTexturePtr);

        uint32_t Indirect[] = {0, 1, 1};
        BufferManager::UploadBufferData(m_MembraneIndirectBufferPtr, &Indirect, 0, sizeof(Indirect));

        ContextManager::SetImageTexture(0, _Diminished);
        ContextManager::SetImageTexture(1, _BackgroundTexturePtr);
        ContextManager::SetImageTexture(2, m_MembranePatchesTexturePtr);
        ContextManager::SetImageTexture(3, m_MembraneBordersTexturePtr);
        ContextManager::SetImageTexture(4, m_MembraneTexturePtr);
        ContextManager::SetResourceBuffer(0, m_MembraneIndirectBufferPtr);
        ContextManager::SetResourceBuffer(1, m_MembranePatchBufferPtr);

        // -----------------------------------------------------------------------------
        // Find inner membrane patches
        // -----------------------------------------------------------------------------

        ContextManager::SetShaderCS(m_MembranePatchesCSPtr);

        ContextManager::Barrier();
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        // -----------------------------------------------------------------------------
        // Find outer membrane patches / the membrane border
        // -----------------------------------------------------------------------------

        ContextManager::SetShaderCS(m_MembraneBorderCSPtr);

        ContextManager::Barrier();
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        // -----------------------------------------------------------------------------
        // Evaluate color differences at the membrane border
        // -----------------------------------------------------------------------------

        ContextManager::SetShaderCS(m_MembraneEvalBorderCSPtr);

        ContextManager::Barrier();
        ContextManager::DispatchIndirect(m_MembraneIndirectBufferPtr, 0);

        // -----------------------------------------------------------------------------
        // Propagate differences to grid
        // -----------------------------------------------------------------------------

        ContextManager::SetShaderCS(m_MembranePropagateGridCSPtr);

        ContextManager::Barrier();
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        // -----------------------------------------------------------------------------
        // Propagate differences to pixels
        // -----------------------------------------------------------------------------

        ContextManager::SetShaderCS(m_MembranePropagatePixelsCSPtr);

        ContextManager::Barrier();
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------

        ContextManager::ResetShaderCS();

        ContextManager::ResetImageTexture(0);
        ContextManager::ResetImageTexture(1);
        ContextManager::ResetImageTexture(2);
        ContextManager::ResetImageTexture(3);
        ContextManager::ResetResourceBuffer(0);
        ContextManager::ResetResourceBuffer(1);
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderInpaintedPlane(const glm::mat4& _rPoseMatrix, const Base::AABB3Float& _rAABB)
    {
        assert(m_InpaintedPlaneTexture != nullptr);

        Performance::BeginEvent("Render inpainted plane");

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));
        
        ContextManager::SetShaderVS(m_InpaintedPlaneVSPtr);
        ContextManager::SetShaderPS(m_InpaintedPlaneFSPtr);

        ContextManager::SetTexture(0, m_InpaintedPlaneTexture);

        glm::vec3 Min = _rAABB.GetMin();
        glm::vec3 Max = _rAABB.GetMax();

        glm::vec3 MinAnchor = Min;
        glm::vec3 MaxAnchor = Max;
        Max.z = Min.z;

        float SelectionWidth = glm::max(MaxAnchor.x - MinAnchor.x, MaxAnchor.y - MinAnchor.y);
        float Scale = m_InpaintedPlaneScale * SelectionWidth * 0.5f;

        glm::vec3 MiddlePoint = (Min + Max) / 2.0f;
        
        SDrawCallConstantBuffer BufferData;
        
        BufferData.m_WorldMatrix = glm::translate(MiddlePoint) * glm::scale(glm::vec3(Scale));
        BufferData.m_Color = glm::vec4(Scale, m_InpaintedPlaneScale, 0.0f, 0.0f);

        BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_InpaintedPlaneMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_InpaintedPlaneMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);

        ContextManager::SetInputLayout(m_InpaintedPlaneLayoutPtr);
        ContextManager::SetTopology(STopology::TriangleStrip);

        ContextManager::Draw(m_InpaintedPlaneMeshPtr->GetLOD(0)->GetSurface()->GetNumberOfVertices(), 0);

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

        const auto& GridSizes = m_pReconstructor->GetVolumeSizes();

        for (auto& rPair : m_pReconstructor->GetRootVolumeMap())
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

        const auto& VolumeSizes = m_pReconstructor->GetVolumeSizes();

        for (auto& rPair : m_pReconstructor->GetRootVolumeMap())
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

                ContextManager::DrawIndirect(rRootGrid.m_IndirectLevel1Buffer, MR::CSLAMReconstructor::SIndirectBuffers::s_DrawOffset);
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

        const auto& VolumeSizes = m_pReconstructor->GetVolumeSizes();

        for (auto& rPair : m_pReconstructor->GetRootVolumeMap())
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

                ContextManager::DrawIndirect(rRootGrid.m_IndirectLevel2Buffer, MR::CSLAMReconstructor::SIndirectBuffers::s_DrawOffset);
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

        glm::mat4 PoseMatrix = m_pReconstructor->GetPoseMatrix();
        
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
        const auto& rPlanes = m_pReconstructor->GetPlanes();

        if (rPlanes.empty())
        {
            return;
        }

        Performance::BeginEvent("Plane rendering");
        
        ContextManager::SetRenderContext(m_OutlineRenderContextPtr);
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
        
        for (const auto& Plane : rPlanes)
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

        glm::mat4 PoseMatrix = m_pReconstructor->GetPoseMatrix();
        
        BufferData.m_WorldMatrix = glm::eulerAngleX(glm::half_pi<float>()) * PoseMatrix;
        BufferData.m_Color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

        BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);

        ContextManager::SetImageTexture(0, m_pReconstructor->GetVertexMap());
        ContextManager::SetImageTexture(1, m_pReconstructor->GetColorMap());

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_CameraMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_CameraMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);

        ContextManager::SetInputLayout(m_CameraInputLayoutPtr);
        ContextManager::SetTopology(STopology::PointList);

        glm::ivec2 DepthSize = m_pReconstructor->GetDepthImageSize();

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

        MR::CSLAMReconstructor::SSLAMVolume& rVolume = m_pReconstructor->GetVolume();

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

    void CGfxReconstructionRenderer::UpdateSelectionBox()
    {
        if (m_SelectionState == ESelection::NOSELECTION) return;

        // -----------------------------------------------------------------------------
        // Get minimum and maximum
        // -----------------------------------------------------------------------------
        glm::vec3 Min = m_SelectionBox.GetMin();
        glm::vec3 Max = m_SelectionBox.GetMax();

        // -----------------------------------------------------------------------------
        // Calculate box w/ transform in WS
        // -----------------------------------------------------------------------------
        glm::mat4 Scaling     = glm::scale(glm::vec3(m_SelectionBox.GetSize()));
        glm::mat4 Translation = glm::translate(Min);

        m_SelectionTransform = Translation * Scaling;
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::AddPositionToSelection(const glm::vec3& _rWSPosition)
    {
        if (m_SelectionBox.GetMin() == glm::vec3(0.0f) && m_SelectionBox.GetMax() == glm::vec3(0.0f))
        {
            m_SelectionBox.SetMin(_rWSPosition);
            m_SelectionBox.SetMax(_rWSPosition);
        }
        else
        {
            m_SelectionBox.StickyExtend(_rWSPosition, 0.0035f);
        }

        m_SelectionState = ESelection::SELECTED;
    }

    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::SetInpaintedPlane(Gfx::CTexturePtr _Texture, const Base::AABB3Float& _rAABB)
    {
        m_InpaintedPlaneTexture = _Texture;
        m_InpaintedPlaneAABB = _rAABB;
    }

    // -----------------------------------------------------------------------------
    
    CTexturePtr CGfxReconstructionRenderer::GetInpaintedRendering(const glm::mat4& _rPoseMatrix, const Base::AABB3Float& _rAABB, CTexturePtr _BackgroundTexturePtr)
    {
        m_IsInpainting = true;

        if (m_InpaintedPlaneTexture != nullptr)
        {
            Performance::BeginEvent("Render diminished reality");

            Gfx::TargetSetManager::ClearTargetSet(m_DiminishedRaycastTargetSetPtr);
            Gfx::TargetSetManager::ClearTargetSet(m_DiminishedPlaneTargetSetPtr);
            Gfx::TargetSetManager::ClearTargetSet(m_DiminishedFinalTargetSetPtr);

            ContextManager::SetTargetSet(m_DiminishedPlaneTargetSetPtr);
            ContextManager::SetViewPortSet(m_DiminishedViewPortSetPtr);

            //glEnable(GL_BLEND);
            //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            RenderInpaintedPlane(_rPoseMatrix, _rAABB);

            ContextManager::SetTargetSet(m_DiminishedRaycastTargetSetPtr);

            Performance::BeginEvent("Raycasting for diminishing");

            RaycastVolumeDiminished(_rPoseMatrix, _rAABB);

            Performance::EndEvent();

            //glDisable(GL_BLEND);

            Performance::BeginEvent("Create membrane");

            CreateMembrane(_BackgroundTexturePtr, m_DiminishedRaycastTargetPtr);

            Performance::EndEvent();

            if (_BackgroundTexturePtr != nullptr)
            {
                Performance::BeginEvent("Combine images");

                ContextManager::SetTargetSet(m_DiminishedFinalTargetSetPtr);

                Gfx::TextureManager::UpdateMipmap(m_DiminishedRaycastTargetPtr);

                CombineDiminishedImage(_BackgroundTexturePtr, m_DiminishedRaycastTargetPtr);

                Performance::EndEvent();
            }

            Performance::EndEvent();

            return m_DiminishedFinalTargetPtr;
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::ResetSelection()
    {
        m_SelectionBox.Set(glm::vec3(0.0f), glm::vec3(0.0f));

        m_SelectionState = ESelection::NOSELECTION;
    }

    // -----------------------------------------------------------------------------

    const Base::AABB3Float& CGfxReconstructionRenderer::GetSelectionBox()
    {
        return m_SelectionBox;
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::Render()
    {
        if (!m_IsInitialized && !m_pReconstructor->IsInitialized())
        {
            return;
        }

        if (!m_IsInitialized && m_pReconstructor->IsInitialized())
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
                RaycastVolume();
            }
            else
            {
                if (!m_IsInpainting)
                {
                    RaycastVolumeWithHighlight();
                }
            }
        }

        Performance::EndEvent();

        ContextManager::ResetShaderVS();
        ContextManager::ResetShaderPS();
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderForward()
    {
        if (!m_IsInitialized && !m_pReconstructor->IsInitialized())
        {
            return;
        }
        
        if (m_RenderVertexMap)
        {
            RenderVertexMap();
        }

        if (!m_UseTrackingCamera)
        {
            RenderCamera();
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

        if (m_IsInpainting)
        {
            ContextManager::SetTargetSet(TargetSetManager::GetLightAccumulationTargetSet());
            RenderBackgroundImage(m_DiminishedFinalTargetPtr, true);
        }

        ContextManager::ResetShaderVS();
        ContextManager::ResetShaderPS();
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderHitProxy()
    {
        if (!m_IsInitialized)
        {
            return;
        }

        // -----------------------------------------------------------------------------
        // Get ID
        // -----------------------------------------------------------------------------
        Dt::CEntity::BID ID = Dt::CEntity::s_InvalidID;

        auto& rComponentManager = Dt::CComponentManager::GetInstance();

        auto& rComponents = rComponentManager.GetComponents<Dt::CScriptComponent>();

        for (auto Component : rComponents)
        {
            auto ScriptComponent = static_cast<Dt::CScriptComponent*>(Component);

            if (ScriptComponent->IsActiveAndUsable() == false) continue;

            if (ScriptComponent->GetScriptTypeID() == Base::CTypeInfo::GetTypeID<Scpt::CSLAMScript>())
            {
                ID = ScriptComponent->GetHostEntity()->GetID();

                break;
            }
        }

        Performance::BeginEvent("Voxel Hit Proxy");

        if (m_RenderVolume && ID != Dt::CEntity::s_InvalidID)
        {
            MR::CSLAMReconstructor::SSLAMVolume& rVolume = m_pReconstructor->GetVolume();

            MR::SReconstructionSettings Settings;
            m_pReconstructor->GetReconstructionSettings(&Settings);

            glm::mat4 PoseMatrix = m_pReconstructor->GetPoseMatrix();

            ContextManager::SetShaderVS(m_RaycastVSPtr);
            ContextManager::SetShaderPS(m_RaycastHitProxyFSPtr);

            ContextManager::SetResourceBuffer(0, rVolume.m_RootVolumePoolPtr);
            ContextManager::SetResourceBuffer(1, rVolume.m_RootGridPoolPtr);
            ContextManager::SetResourceBuffer(2, rVolume.m_Level1PoolPtr);
            ContextManager::SetResourceBuffer(3, rVolume.m_TSDFPoolPtr);
            ContextManager::SetResourceBuffer(6, rVolume.m_RootVolumePositionBufferPtr);

            ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
            ContextManager::SetConstantBuffer(1, m_RaycastHitProxyBufferPtr);
            ContextManager::SetConstantBuffer(2, rVolume.m_AABBBufferPtr);

            ContextManager::Barrier();

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

            BufferManager::UploadBufferData(m_RaycastHitProxyBufferPtr, &ID);

            BufferManager::UploadBufferData(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer(), &Vertices);

            const unsigned int Offset = 0;
            ContextManager::SetVertexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
            ContextManager::SetIndexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);
            ContextManager::SetInputLayout(m_VolumeInputLayoutPtr);

            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::DrawIndexed(36, 0, 0);

            ContextManager::ResetShaderVS();
            ContextManager::ResetShaderPS();
        }

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::ChangeCamera(bool _IsTrackingCamera)
    {
        m_UseTrackingCamera = _IsTrackingCamera;
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::SetReconstructor(MR::CSLAMReconstructor& _rReconstructor)
    {
        m_pReconstructor = &_rReconstructor;
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::SetDeviceResolution(const glm::ivec2& _Resolution)
    {
        m_DeviceResolution = _Resolution;
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

    void RenderHitProxy()
    {
        CGfxReconstructionRenderer::GetInstance().RenderHitProxy();
    }
    
    // -----------------------------------------------------------------------------

    void ChangeCamera(bool _IsTrackingCamera)
    {
        CGfxReconstructionRenderer::GetInstance().ChangeCamera(_IsTrackingCamera);
    }
    
    // -----------------------------------------------------------------------------

    void SetReconstructor(MR::CSLAMReconstructor& _rReconstructor)
    {
        CGfxReconstructionRenderer::GetInstance().SetReconstructor(_rReconstructor);
    }

    // -----------------------------------------------------------------------------

    void SetDeviceResolution(const glm::ivec2& _Resolution)
    {
        CGfxReconstructionRenderer::GetInstance().SetDeviceResolution(_Resolution);
    }

    // -----------------------------------------------------------------------------

    glm::vec3 Pick(const glm::ivec2& _rCursor)
    {
        return CGfxReconstructionRenderer::GetInstance().Pick(_rCursor);
    }

    // -----------------------------------------------------------------------------

    void UpdateSelectionBox()
    {
        CGfxReconstructionRenderer::GetInstance().UpdateSelectionBox();
    }

    // -----------------------------------------------------------------------------

    void AddPositionToSelection(const glm::vec3& _rWSPosition)
    {
        CGfxReconstructionRenderer::GetInstance().AddPositionToSelection(_rWSPosition);
    }

    // -----------------------------------------------------------------------------

    void ResetSelection()
    {
        CGfxReconstructionRenderer::GetInstance().ResetSelection();
    }

    // -----------------------------------------------------------------------------

    void SetInpaintedPlane(Gfx::CTexturePtr _Texture, const Base::AABB3Float& _rAABB)
    {
        CGfxReconstructionRenderer::GetInstance().SetInpaintedPlane(_Texture, _rAABB);
    }

    // -----------------------------------------------------------------------------

    CTexturePtr GetInpaintedRendering(const glm::mat4& _rPoseMatrix, const Base::AABB3Float& _rAABB, CTexturePtr _BackgroundTexturePtr)
    {
        return CGfxReconstructionRenderer::GetInstance().GetInpaintedRendering(_rPoseMatrix, _rAABB, _BackgroundTexturePtr);
    }

    // -----------------------------------------------------------------------------

    const Base::AABB3Float& GetSelectionBox()
    {
        return CGfxReconstructionRenderer::GetInstance().GetSelectionBox();
    }

} // namespace ReconstructionRenderer
} // namespace Gfx

