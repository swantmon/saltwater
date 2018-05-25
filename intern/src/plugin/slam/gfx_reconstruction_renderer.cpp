
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
#include "plugin/slam/mr_slam_reconstructor.h"
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
        void Render(int _Pass);
        void PauseIntegration(bool _Paused);
        void PauseTracking(bool _Paused);
        void ChangeCamera(bool _IsTrackingCamera);
        float GetReconstructionSize();

        void OnReconstructionUpdate(const MR::SReconstructionSettings& _Settings);

    private:

        void RenderVolumeVertexMap();
		void RaycastVolume();

		void RaycastScalableVolume();
        
        void RenderQueuedRootVolumes();
        void RenderQueuedLevel1Grids();
        void RenderQueuedLevel2Grids();

        void RenderCamera();

        void RenderVertexMap();

    private:

		std::unique_ptr<MR::CSLAMReconstructor> m_pReconstructor;
		std::unique_ptr<MR::CScalableSLAMReconstructor> m_pScalableReconstructor;
        
        CShaderPtr m_OutlineVSPtr;
        CShaderPtr m_OutlineFSPtr;

        CShaderPtr m_VolumeVertexMapVSPtr;
        CShaderPtr m_VolumeVertexMapFSPtr;

        CShaderPtr m_OutlineLevel1VSPtr;
        CShaderPtr m_OutlineLevel1FSPtr;
        CShaderPtr m_OutlineLevel2VSPtr;
        CShaderPtr m_OutlineLevel2FSPtr;
        
        CShaderPtr m_RaycastVSPtr;
        CShaderPtr m_RaycastFSPtr;

        CShaderPtr m_CopyRaycastVSPtr;
        CShaderPtr m_CopyRaycastFSPtr;

        CShaderPtr m_HistogramVSPtr;
        CShaderPtr m_HistogramFSPtr;

        CBufferPtr m_RaycastConstantBufferPtr;
        CBufferPtr m_DrawCallConstantBufferPtr;
                
        CMeshPtr m_CameraMeshPtr;
		CInputLayoutPtr m_CameraInputLayoutPtr;

		CMeshPtr m_CubeOutlineMeshPtr;
		CInputLayoutPtr m_CubeOutlineInputLayoutPtr;

        CMeshPtr m_VolumeMeshPtr;        
        CInputLayoutPtr m_VolumeInputLayoutPtr;

        CMeshPtr m_QuadMeshPtr;
        CInputLayoutPtr m_QuadInputLayoutPtr;

        CRenderContextPtr m_OutlineRenderContextPtr;

        CMeshPtr m_PlaneMeshPtr;
        
        CShaderPtr m_PointCloudVSPtr;
        CShaderPtr m_PointCloudFSPtr;

        CTexturePtr m_IntermediateTargetPtr0;
        CTexturePtr m_IntermediateTargetPtr1;
        CTargetSetPtr m_IntermediateTargetSetPtr;

        bool m_UseTrackingCamera;

        bool m_RenderVolumeVertexMap;
        bool m_RenderVolume;
        bool m_RenderVertexMap;
        bool m_RenderRootQueue;
        bool m_RenderLevel1Queue;
        bool m_RenderLevel2Queue;
        bool m_RenderHistogram;
        bool m_RenderPlanes;
        bool m_RenderBackSides;
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
        , m_RenderHistogram      (false)
        , m_RenderPlanes         (false)
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
        
		MR::SReconstructionSettings DefaultSettings;
        MR::SReconstructionSettings::SetDefaultSettings(DefaultSettings);

		if (DefaultSettings.m_IsScalable)
		{
			m_pScalableReconstructor.reset(new MR::CScalableSLAMReconstructor);
			m_pReconstructor = nullptr;
		}
		else
		{
			m_pReconstructor.reset(new MR::CSLAMReconstructor);
			m_pScalableReconstructor = nullptr;
		}

        m_UseTrackingCamera     = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:use_tracking_camera", true);
        m_RenderVolume          = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:volume"             , true);
        m_RenderVolumeVertexMap = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:volume_vertex_map"  , false);
        m_RenderVertexMap       = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:vertex_map"         , false);
        m_RenderRootQueue       = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:queues:root"        , false);
        m_RenderLevel1Queue     = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:queues:level1"      , false);
        m_RenderLevel2Queue     = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:queues:level2"      , false);
        m_RenderHistogram       = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:histogram"          , false);
        m_RenderPlanes          = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:planes"             , false);
        m_RenderBackSides       = Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:backsides"          , true);
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
        m_RaycastFSPtr = 0;
        m_CopyRaycastVSPtr = 0;
        m_CopyRaycastFSPtr = 0;
        m_HistogramVSPtr = 0;
        m_HistogramFSPtr = 0;

        m_VolumeVertexMapVSPtr = 0;
        m_VolumeVertexMapFSPtr = 0;
                
        m_RaycastConstantBufferPtr = 0;
        m_DrawCallConstantBufferPtr = 0;
                
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

		m_pReconstructor = nullptr;
		m_pScalableReconstructor = nullptr;

        m_PointCloudVSPtr = 0;
        m_PointCloudFSPtr = 0;

        m_IntermediateTargetPtr0 = nullptr;
        m_IntermediateTargetPtr1 = nullptr;
        m_IntermediateTargetSetPtr = nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupShader()
    {
		MR::SReconstructionSettings Settings;

        const std::string InternalFormatString = Core::CProgramParameters::GetInstance().Get("mr:slam:map_format", "rgba16f");

		if (m_pScalableReconstructor != nullptr)
		{
			m_pScalableReconstructor->GetReconstructionSettings(&Settings);

            glm::ivec2 DepthImageSize = m_pScalableReconstructor->GetDepthImageSize();

			std::stringstream DefineStream;

            DefineStream
                << "#define TRUNCATED_DISTANCE "     << Settings.m_TruncatedDistance / 1000.0f << " \n"
                << "#define VOLUME_SIZE "            << Settings.m_VolumeSize << " \n"
                << "#define VOXEL_SIZE "             << Settings.m_VoxelSize << " \n"
                << "#define MAX_INTEGRATION_WEIGHT " << Settings.m_MaxIntegrationWeight << '\n'
                << "#define DEPTH_IMAGE_WIDTH "      << DepthImageSize.x << '\n'
                << "#define DEPTH_IMAGE_HEIGHT "     << DepthImageSize.y << '\n'
                << "#define ROOT_RESOLUTION "        << Settings.m_GridResolutions[0] << '\n'
                << "#define LEVEL1_RESOLUTION "      << Settings.m_GridResolutions[1] << '\n'
                << "#define LEVEL2_RESOLUTION "      << Settings.m_GridResolutions[2] << '\n'
                << "#define VOXELS_PER_ROOTGRID "    << Settings.m_VoxelsPerGrid[0] << " \n"
                << "#define VOXELS_PER_LEVEL1GRID "  << Settings.m_VoxelsPerGrid[1] << " \n"
                << "#define VOXELS_PER_LEVEL2GRID "  << Settings.m_VoxelsPerGrid[2] << " \n"
                << "#define MAP_TEXTURE_FORMAT "     << InternalFormatString << " \n"
                << "#define RAYCAST_NEAR "           << 0.0f << " \n"
                << "#define RAYCAST_FAR "            << 1000.0f << " \n"
                << "#define MIN_TREE_WEIGHT "        << Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:min_weight", 30) << " \n";
            
            if (Settings.m_CaptureColor)
            {
                DefineStream << "#define CAPTURE_COLOR\n";
            }
            if (m_RenderBackSides)
            {
                DefineStream << "#define RAYCAST_BACKSIDES\n";
            }

			std::string DefineString = DefineStream.str();

			m_OutlineVSPtr       = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rendering\\vs_outline.glsl"       , "main", DefineString.c_str());
			m_OutlineFSPtr       = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rendering\\fs_outline.glsl"       , "main", DefineString.c_str());
            m_OutlineLevel1VSPtr = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rendering\\vs_outline_level1.glsl", "main", DefineString.c_str());
            m_OutlineLevel1FSPtr = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rendering\\fs_outline_level1.glsl", "main", DefineString.c_str());
            m_OutlineLevel2VSPtr = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rendering\\vs_outline_level2.glsl", "main", DefineString.c_str());
            m_OutlineLevel2FSPtr = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rendering\\fs_outline_level2.glsl", "main", DefineString.c_str());
			
            m_PointCloudVSPtr = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rendering\\vs_point_cloud.glsl", "main", DefineString.c_str());
            m_PointCloudFSPtr = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rendering\\fs_point_cloud.glsl", "main", DefineString.c_str());
            
            m_RaycastVSPtr = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rendering\\vs_raycast.glsl", "main", DefineString.c_str());
            m_RaycastFSPtr = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rendering\\fs_raycast.glsl", "main", DefineString.c_str());

            m_CopyRaycastVSPtr = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rendering\\vs_copy_raycast.glsl", "main", DefineString.c_str());
            m_CopyRaycastFSPtr = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rendering\\fs_copy_raycast.glsl", "main", DefineString.c_str());

            m_HistogramVSPtr = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rendering\\vs_histogram.glsl", "main", DefineString.c_str());
            m_HistogramFSPtr = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rendering\\fs_histogram.glsl", "main", DefineString.c_str());

            m_VolumeVertexMapVSPtr = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rendering\\vs_volume_vertex_map.glsl", "main", DefineString.c_str());
            m_VolumeVertexMapFSPtr = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rendering\\fs_volume_vertex_map.glsl", "main", DefineString.c_str());
        }
		else
		{
			m_pReconstructor->GetReconstructionSettings(&Settings);

			std::stringstream DefineStream;

			DefineStream
				<< "#define VOLUME_RESOLUTION "  << Settings.m_VolumeResolution << " \n"
				<< "#define TRUNCATED_DISTANCE " << Settings.m_TruncatedDistance / 1000.0f << " \n"
				<< "#define VOLUME_SIZE "        << Settings.m_VolumeSize << " \n"
				<< "#define VOXEL_SIZE "         << Settings.m_VolumeSize / Settings.m_VolumeResolution << " \n"
                << "#define MAP_TEXTURE_FORMAT " << InternalFormatString << " \n";

			if (Settings.m_CaptureColor)
			{
				DefineStream << "#define CAPTURE_COLOR\n";
			}

			std::string DefineString = DefineStream.str();

			m_OutlineVSPtr = ShaderManager::CompileVS("slam\\kinect_fusion\\vs_outline.glsl", "main", DefineString.c_str());
			m_OutlineFSPtr = ShaderManager::CompilePS("slam\\kinect_fusion\\fs_outline.glsl", "main", DefineString.c_str());
			m_RaycastVSPtr = ShaderManager::CompileVS("slam\\kinect_fusion\\vs_raycast.glsl", "main", DefineString.c_str());
			m_RaycastFSPtr = ShaderManager::CompilePS("slam\\kinect_fusion\\fs_raycast.glsl", "main", DefineString.c_str());
		}
        
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

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupStates()
    {
        m_OutlineRenderContextPtr = ContextManager::CreateRenderContext();

        m_OutlineRenderContextPtr->SetCamera(ViewManager::GetMainCamera());
        m_OutlineRenderContextPtr->SetViewPortSet(ViewManager::GetViewPortSet());
        m_OutlineRenderContextPtr->SetTargetSet(TargetSetManager::GetDeferredTargetSet());
        m_OutlineRenderContextPtr->SetRenderState(StateManager::GetRenderState(CRenderState::NoCull | CRenderState::Wireframe));
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupTextures()
    {
        glm::ivec2 Size = Main::GetActiveWindowSize();

        STextureDescriptor TextureDescriptor = {};

        TextureDescriptor.m_NumberOfPixelsU = Size.x;
        TextureDescriptor.m_NumberOfPixelsV = Size.y;
        TextureDescriptor.m_NumberOfPixelsW = 1;
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding = CTexture::ShaderResource | CTexture::RenderTarget;
        TextureDescriptor.m_Access = CTexture::CPUWrite;
        TextureDescriptor.m_Usage = CTexture::GPUReadWrite;
        TextureDescriptor.m_Semantic = CTexture::UndefinedSemantic;
        TextureDescriptor.m_Format = CTexture::R16G16B16A16_FLOAT;

        m_IntermediateTargetPtr0 = TextureManager::CreateTexture2D(TextureDescriptor);
        m_IntermediateTargetPtr1 = TextureManager::CreateTexture2D(TextureDescriptor);

        m_IntermediateTargetSetPtr = TargetSetManager::CreateTargetSet(m_IntermediateTargetPtr0, m_IntermediateTargetPtr1);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstantBufferDesc = {};

        ConstantBufferDesc.m_Stride = 0;
        ConstantBufferDesc.m_Usage = CBuffer::EUsage::GPURead;
        ConstantBufferDesc.m_Binding = CBuffer::ConstantBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(glm::vec4) * 2;
        ConstantBufferDesc.m_pBytes = nullptr;
        ConstantBufferDesc.m_pClassKey = 0;

        m_RaycastConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SDrawCallConstantBuffer);
        m_DrawCallConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
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

        //Todo: remove magic numbers (focal length/point, max/min depth)

        float Focalx = (-0.50602675f) / 0.72113f;
        float Focaly = (-0.499133f) / 0.870799f;

        glm::vec3 CameraVertices[] =
        {
            glm::vec3(-Focalx * 8.0f, -Focaly * 8.0f, 8.0f),
            glm::vec3( Focalx * 8.0f, -Focaly * 8.0f, 8.0f),
            glm::vec3( Focalx * 8.0f,  Focaly * 8.0f, 8.0f),
            glm::vec3(-Focalx * 8.0f,  Focaly * 8.0f, 8.0f),
            glm::vec3(          0.0f,           0.0f, 0.0f),
            glm::vec3(-Focalx * 0.5f, -Focaly * 0.5f, 0.5f),
            glm::vec3( Focalx * 0.5f, -Focaly * 0.5f, 0.5f),
            glm::vec3( Focalx * 0.5f,  Focaly * 0.5f, 0.5f),
            glm::vec3(-Focalx * 0.5f,  Focaly * 0.5f, 0.5f),
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

        ////////////////////////////////////////////////////////////////////////////////
        // Create cube mesh
        ////////////////////////////////////////////////////////////////////////////////

        Dt::CSurface* pSurface = new Dt::CSurface;
        Dt::CLOD* pLOD = new Dt::CLOD;
        Dt::CMesh* pMesh = new Dt::CMesh;

        pSurface->SetPositions(CameraLines);
        pSurface->SetNumberOfVertices(sizeof(CameraLines) / sizeof(CameraLines[0]));
        pSurface->SetIndices(nullptr);
        pSurface->SetNumberOfIndices(0);
        pSurface->SetElements(0);

        pLOD->SetSurface(0, pSurface);
        pLOD->SetNumberOfSurfaces(1);

        pMesh->SetLOD(0, pLOD);
        pMesh->SetNumberOfLODs(1);

        SMeshDescriptor MeshDesc =
        {
            pMesh
        };

        m_CameraMeshPtr = MeshManager::CreateMesh(MeshDesc);

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

        unsigned int CubeIndices[] =
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

        pSurface = new Dt::CSurface;
        pLOD = new Dt::CLOD;
        pMesh = new Dt::CMesh;

        pSurface->SetPositions(g_CubeVertices);
        pSurface->SetNumberOfVertices(sizeof(g_CubeVertices) / sizeof(g_CubeVertices[0]));
        pSurface->SetIndices(CubeIndices);
        pSurface->SetNumberOfIndices(sizeof(CubeIndices) / sizeof(CubeIndices[0]));
        pSurface->SetElements(0);

        pLOD->SetSurface(0, pSurface);
        pLOD->SetNumberOfSurfaces(1);

        pMesh->SetLOD(0, pLOD);
        pMesh->SetNumberOfLODs(1);

        MeshDesc.m_pMesh = pMesh;

        m_VolumeMeshPtr = MeshManager::CreateMesh(MeshDesc);

        ////////////////////////////////////////////////////////////////////////////////
        // Create cube outline mesh
        ////////////////////////////////////////////////////////////////////////////////

        pSurface = new Dt::CSurface;
        pLOD = new Dt::CLOD;
        pMesh = new Dt::CMesh;

        pSurface->SetPositions(CubeLines);
        pSurface->SetNumberOfVertices(sizeof(CubeLines) / sizeof(CubeLines[0]));
        pSurface->SetIndices(nullptr);
        pSurface->SetNumberOfIndices(0);
        pSurface->SetElements(0);

        pLOD->SetSurface(0, pSurface);
        pLOD->SetNumberOfSurfaces(1);

        pMesh->SetLOD(0, pLOD);
        pMesh->SetNumberOfLODs(1);

        MeshDesc.m_pMesh = pMesh;

        m_CubeOutlineMeshPtr = MeshManager::CreateMesh(MeshDesc);

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

        pSurface = new Dt::CSurface;
        pLOD = new Dt::CLOD;
        pMesh = new Dt::CMesh;

        pSurface->SetPositions(QuadLines);
        pSurface->SetNumberOfVertices(sizeof(QuadLines) / sizeof(QuadLines[0]));
        pSurface->SetIndices(nullptr);
        pSurface->SetNumberOfIndices(0);
        pSurface->SetElements(0);

        pLOD->SetSurface(0, pSurface);
        pLOD->SetNumberOfSurfaces(1);

        pMesh->SetLOD(0, pLOD);
        pMesh->SetNumberOfLODs(1);

        MeshDesc.m_pMesh = pMesh;

        m_QuadMeshPtr = MeshManager::CreateMesh(MeshDesc);

        ////////////////////////////////////////////////////////////////////////////////
        // Create plane mesh
        ////////////////////////////////////////////////////////////////////////////////

        std::vector<glm::vec3> PlaneVertices;

        const int PlaneSize = 3;

        for (int x = -PlaneSize; x <= PlaneSize; ++x)
        {
            for (int y = -PlaneSize; y <= PlaneSize; ++y)
            {
                glm::vec3 NewVertices[4] =
                {
                    QuadLines[0],
                    QuadLines[1],
                    QuadLines[2],
                    QuadLines[3],
                };

                for (int i = 0; i < 4; ++i)
                {
                    NewVertices[i][0] += x;
                    NewVertices[i][1] += y;

                    PlaneVertices.push_back(NewVertices[i]);
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

        pSurface = new Dt::CSurface;
        pLOD = new Dt::CLOD;
        pMesh = new Dt::CMesh;

        pSurface->SetPositions(PlaneVertices.data());
        pSurface->SetNumberOfVertices(int(PlaneVertices.size()));
        pSurface->SetIndices(&Indices[0]);
        pSurface->SetNumberOfIndices(static_cast<int>(Indices.size()));
        pSurface->SetElements(0);

        pLOD->SetSurface(0, pSurface);
        pLOD->SetNumberOfSurfaces(1);

        pMesh->SetLOD(0, pLOD);
        pMesh->SetNumberOfLODs(1);

        MeshDesc.m_pMesh = pMesh;

        m_PlaneMeshPtr = MeshManager::CreateMesh(MeshDesc);
    }

    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnReload()
    {
        if (m_pScalableReconstructor != nullptr)
        {
			m_pScalableReconstructor->ResetReconstruction();
        }
		else
		{
			m_pReconstructor->ResetReconstruction();
		}

        OnSetupShader();
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::OnReconstructionUpdate(const MR::SReconstructionSettings& _Settings)
    {
		if (_Settings.m_IsScalable && m_pScalableReconstructor == nullptr)
		{
			m_pReconstructor = nullptr;
			m_pScalableReconstructor.reset(new MR::CScalableSLAMReconstructor);
		}
		else if (!_Settings.m_IsScalable && m_pReconstructor == nullptr)
		{
			m_pScalableReconstructor = nullptr;
			m_pReconstructor.reset(new MR::CSLAMReconstructor);
		}

		if (m_pScalableReconstructor != nullptr)
		{
			m_pScalableReconstructor->ResetReconstruction(&_Settings);
		}
		else
		{
			m_pReconstructor->ResetReconstruction(&_Settings);
		}

        BufferManager::UploadBufferData(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer(), &g_CubeVertices);
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
        STextureDescriptor TextureDescriptor = {};

        TextureDescriptor.m_NumberOfPixelsU = _Width;
        TextureDescriptor.m_NumberOfPixelsV = _Height;
        TextureDescriptor.m_NumberOfPixelsW = 1;
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding = CTexture::ShaderResource | CTexture::RenderTarget;
        TextureDescriptor.m_Access = CTexture::CPUWrite;
        TextureDescriptor.m_Usage = CTexture::GPUReadWrite;
        TextureDescriptor.m_Semantic = CTexture::UndefinedSemantic;
        TextureDescriptor.m_Format = CTexture::R16G16B16A16_FLOAT;

        m_IntermediateTargetPtr0 = TextureManager::CreateTexture2D(TextureDescriptor);
        m_IntermediateTargetPtr1 = TextureManager::CreateTexture2D(TextureDescriptor);

        m_IntermediateTargetSetPtr = TargetSetManager::CreateTargetSet(m_IntermediateTargetPtr0, m_IntermediateTargetPtr1);
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::Update()
    {
        if (false) // TODO: use script to update renderer
        {
            Cam::CControl& rControl = static_cast<Cam::CEditorControl&>(Cam::ControlManager::GetActiveControl());
            
            glm::mat4 PoseMatrix = ((m_pScalableReconstructor != nullptr) ? m_pScalableReconstructor->GetPoseMatrix() : m_pReconstructor->GetPoseMatrix());
            PoseMatrix = glm::eulerAngleX(glm::radians(90.0f)) * PoseMatrix;

            glm::vec3 Eye = PoseMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            glm::vec3 At = PoseMatrix * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
            glm::vec3 Up = PoseMatrix * glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);

            glm::mat4 View = glm::lookAtRH(Eye, At, Up);

            rControl.SetPosition(glm::vec4(Eye, 1.0f));
            rControl.SetRotation(glm::mat4(glm::inverse(glm::mat3(View))));
            rControl.Update();
        }
        glEnable(GL_PROGRAM_POINT_SIZE);
    }
    
    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderVolumeVertexMap()
    {
        //GLint OldViewPort[4];
        //glGetIntegerv(GL_VIEWPORT, OldViewPort);
        //
        //glViewport(0, 0, 512, 424);

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

        //glViewport(OldViewPort[0], OldViewPort[1], OldViewPort[2], OldViewPort[3]);
    }

    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::RaycastVolume()
    {
        MR::SReconstructionSettings Settings;
        m_pReconstructor->GetReconstructionSettings(&Settings);

        glm::mat4 PoseMatrix = m_pReconstructor->GetPoseMatrix();

        glm::vec4 RaycastData[2];
        PoseMatrix = glm::translate(glm::vec3(RaycastData[0][0], RaycastData[0][1], RaycastData[0][2]));
        
        RaycastData[0][3] = 1.0f;
        if (Settings.m_CaptureColor)
        {
            RaycastData[1] = m_pReconstructor->IsTrackingLost() ? glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }
        else
        {
            RaycastData[1] = m_pReconstructor->IsTrackingLost() ? glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) : glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        }

        BufferManager::UploadBufferData(m_RaycastConstantBufferPtr, RaycastData);
        
        ContextManager::SetShaderVS(m_RaycastVSPtr);
        ContextManager::SetShaderPS(m_RaycastFSPtr);

        ContextManager::SetTexture(0, m_pReconstructor->GetTSDFVolume());
        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::ESampler::MinMagMipLinearClamp));

        if (Settings.m_CaptureColor)
        {
            ContextManager::SetTexture(1, m_pReconstructor->GetColorVolume());
            ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::ESampler::MinMagMipLinearClamp));
        }

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_RaycastConstantBufferPtr);

        ContextManager::Barrier();

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::Default));
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));
        
        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);
        ContextManager::SetInputLayout(m_VolumeInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::DrawIndexed(36, 0, 0);

		// Render volume box

		ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Wireframe));

		ContextManager::SetRenderContext(m_OutlineRenderContextPtr);
		ContextManager::SetShaderVS(m_OutlineVSPtr);
		ContextManager::SetShaderPS(m_OutlineFSPtr);

		SDrawCallConstantBuffer BufferData;

		BufferData.m_WorldMatrix = glm::scale(glm::vec3(Settings.m_VolumeSize));
		BufferData.m_Color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

		BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

		ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
		ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);
		
		ContextManager::SetVertexBuffer(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
		ContextManager::SetInputLayout(m_CubeOutlineInputLayoutPtr);

		ContextManager::SetTopology(STopology::LineList);

		ContextManager::Draw(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface()->GetNumberOfVertices(), 0);
    }

	// -----------------------------------------------------------------------------

	void CGfxReconstructionRenderer::RaycastScalableVolume()
	{
        Performance::BeginEvent("Raycasting for rendering");

        //ContextManager::SetTargetSet(m_IntermediateTargetSetPtr);
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
            glm::vec3(glm::eulerAngleX(glm::radians(90.0f)) * glm::vec4(Min[0], Min[1], Min[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::radians(90.0f)) * glm::vec4(Max[0], Min[1], Min[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::radians(90.0f)) * glm::vec4(Max[0], Max[1], Min[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::radians(90.0f)) * glm::vec4(Min[0], Max[1], Min[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::radians(90.0f)) * glm::vec4(Min[0], Min[1], Max[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::radians(90.0f)) * glm::vec4(Max[0], Min[1], Max[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::radians(90.0f)) * glm::vec4(Max[0], Max[1], Max[2], 1.0f)),
            glm::vec3(glm::eulerAngleX(glm::radians(90.0f)) * glm::vec4(Min[0], Max[1], Max[2], 1.0f))
        };

        /*glm::vec3 Vertices[8] =
        {
            glm::vec3(Min[0], Min[1], Min[2]),
            glm::vec3(Max[0], Min[1], Min[2]),
            glm::vec3(Max[0], Max[1], Min[2]),
            glm::vec3(Min[0], Max[1], Min[2]),
            glm::vec3(Min[0], Min[1], Max[2]),
            glm::vec3(Max[0], Min[1], Max[2]),
            glm::vec3(Max[0], Max[1], Max[2]),
            glm::vec3(Min[0], Max[1], Max[2])
        };*/

        glm::vec4 RaycastData[2];
        PoseMatrix = glm::translate(glm::vec3(RaycastData[0][0], RaycastData[0][1], RaycastData[0][2]));
        RaycastData[0][3] = 1.0f;
        if (Settings.m_CaptureColor)
        {
            RaycastData[1] = m_pScalableReconstructor->IsTrackingLost() ? glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }
        else
        {
            RaycastData[1] = m_pScalableReconstructor->IsTrackingLost() ? glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) : glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        }
        BufferManager::UploadBufferData(m_RaycastConstantBufferPtr, RaycastData);

        BufferManager::UploadBufferData(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer(), &Vertices);

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);
        ContextManager::SetInputLayout(m_VolumeInputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::DrawIndexed(36, 0, 0);

        Performance::EndEvent();

        /*ContextManager::SetTargetSet(TargetSetManager::GetDeferredTargetSet());

        ContextManager::SetShaderVS(m_CopyRaycastVSPtr);
        ContextManager::SetShaderPS(m_CopyRaycastFSPtr);

        ContextManager::SetVertexBuffer(m_QuadMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_QuadMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);
        ContextManager::SetInputLayout(m_QuadInputLayoutPtr);

        ContextManager::SetImageTexture(0, m_IntermediateTargetPtr0);
        ContextManager::SetImageTexture(1, m_IntermediateTargetPtr1);

        ContextManager::SetTopology(STopology::TriangleStrip);

        ContextManager::Draw(4, 0);*/
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

		BufferData.m_WorldMatrix = (m_pScalableReconstructor != nullptr) ? m_pScalableReconstructor->GetPoseMatrix() : m_pReconstructor->GetPoseMatrix();
        BufferData.m_WorldMatrix = glm::eulerAngleX(glm::radians(90.0f)) * BufferData.m_WorldMatrix;
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

    void CGfxReconstructionRenderer::RenderVertexMap()
    {
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));
        ContextManager::SetRenderContext(m_OutlineRenderContextPtr);

        ContextManager::SetShaderVS(m_PointCloudVSPtr);
        ContextManager::SetShaderPS(m_PointCloudFSPtr);

        SDrawCallConstantBuffer BufferData;

        BufferData.m_WorldMatrix = glm::mat4(1);
        BufferData.m_WorldMatrix = BufferData.m_WorldMatrix * glm::eulerAngleX(glm::radians(90.0f));
        BufferData.m_Color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);

        BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);

        ContextManager::SetImageTexture(0, m_pScalableReconstructor->GetVertexMap());

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_CameraMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_CameraMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);

        ContextManager::SetInputLayout(m_CameraInputLayoutPtr);
        ContextManager::SetTopology(STopology::PointList);

        ContextManager::Draw(512 * 424, 0);
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::Render(int _Pass)
    {
		if (_Pass == 0)
		{
            if (m_pScalableReconstructor != nullptr)
            {
                m_pScalableReconstructor->Update();
            }
            else
            {
                m_pReconstructor->Update();
            }

            Performance::BeginEvent("SLAM Reconstruction Rendering");

            ContextManager::SetViewPortSet(ViewManager::GetViewPortSet());
            ContextManager::SetTargetSet(TargetSetManager::GetDeferredTargetSet());
            //glm::vec4 ClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            //TargetSetManager::ClearTargetSet(TargetSetManager::GetDeferredTargetSet(), ClearColor);

            if (!m_UseTrackingCamera)
            {
                RenderCamera();
            }

            if (m_pScalableReconstructor != nullptr)
            {
                if (m_RenderVolumeVertexMap)
                {
                    RenderVolumeVertexMap();
                }

                if (m_RenderVolume)
                {
                    RaycastScalableVolume();
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
            }
            else
            {
                RaycastVolume();
            }

            Performance::EndEvent();
		}
        else
        {
            if (m_pScalableReconstructor != nullptr)
            {
                if (m_RenderVolumeVertexMap)
                {
                    RenderVolumeVertexMap();
                }
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::PauseIntegration(bool _Paused)
    {
		if (m_pScalableReconstructor != nullptr)
		{
			m_pScalableReconstructor->PauseIntegration(_Paused);
		}
		else
		{
			m_pReconstructor->PauseIntegration(_Paused);
		}
    }
    
    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::PauseTracking(bool _Paused)
    {
		if (m_pScalableReconstructor != nullptr)
		{
			m_pScalableReconstructor->PauseTracking(_Paused);
		}
		else
		{
			m_pReconstructor->PauseTracking(_Paused);
		}        
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::ChangeCamera(bool _IsTrackingCamera)
    {
        m_UseTrackingCamera = _IsTrackingCamera;
    }

    float CGfxReconstructionRenderer::GetReconstructionSize()
    {
        if (m_pScalableReconstructor != nullptr)
        {
            return m_pScalableReconstructor->GetReconstructionSize();
        }
        else
        {
            MR::SReconstructionSettings Settings;
            m_pReconstructor->GetReconstructionSettings(&Settings);

            int Resolution = Settings.m_VolumeResolution;
            return Resolution * Resolution * Resolution * sizeof(uint32_t) / (1024.0f * 1024.0f);
        }
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

    void OnReconstructionUpdate(const MR::SReconstructionSettings& _Settings)
    {
        CGfxReconstructionRenderer::GetInstance().OnReconstructionUpdate(_Settings);
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
        CGfxReconstructionRenderer::GetInstance().Render(0);
    }
    
    // -----------------------------------------------------------------------------

    void PauseIntegration(bool _Paused)
    {
        CGfxReconstructionRenderer::GetInstance().PauseIntegration(_Paused);
    }

    // -----------------------------------------------------------------------------

    void PauseTracking(bool _Paused)
    {
        CGfxReconstructionRenderer::GetInstance().PauseTracking(_Paused);
    }

    // -----------------------------------------------------------------------------

    void ChangeCamera(bool _IsTrackingCamera)
    {
        CGfxReconstructionRenderer::GetInstance().ChangeCamera(_IsTrackingCamera);
    }

    // -----------------------------------------------------------------------------

    float GetReconstructionSize()
    {
        return CGfxReconstructionRenderer::GetInstance().GetReconstructionSize();
    }

} // namespace Voxel
} // namespace Gfx

