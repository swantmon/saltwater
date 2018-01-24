
#include "graphic/gfx_precompiled.h"

#include "base/base_vector3.h"
#include "base/base_matrix4x4.h"
#include "base/base_program_parameters.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "camera/cam_control_manager.h"
#include "camera/cam_editor_control.h"

#include "core/core_time.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_reconstruction_renderer.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

#include "mr/mr_slam_reconstructor.h"
#include "mr/mr_scalable_slam_reconstructor.h"

#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

using namespace Base;
using namespace Gfx;

namespace
{
    Float3 g_CubeVertices[] =
    {
        Float3(0.0f, 0.0f, 0.0f),
        Float3(1.0f, 0.0f, 0.0f),
        Float3(1.0f, 1.0f, 0.0f),
        Float3(0.0f, 1.0f, 0.0f),
        Float3(0.0f, 0.0f, 1.0f),
        Float3(1.0f, 0.0f, 1.0f),
        Float3(1.0f, 1.0f, 1.0f),
        Float3(0.0f, 1.0f, 1.0f),
    };

	struct SDrawCallConstantBuffer
	{
		Float4x4 m_WorldMatrix;
		Float4 m_Color;
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
        void PauseIntegration(bool _Paused);
        void PauseTracking(bool _Paused);
        void ChangeCamera(bool _IsTrackingCamera);

        void OnReconstructionUpdate(const MR::SReconstructionSettings& _Settings);

    private:

		void RaycastVolume();

		void RaycastScalableVolume();

        void RaycastRootGrids();
        void RaycastLevel1Grids();

        void RenderQueuedRootVolumes();
        void RenderQueuedLevel1Grids();
        void RenderQueuedLevel2Grids();

        void RenderHONV();

        void RenderCamera();

        void RenderVertexMap();
        
    private:

		std::unique_ptr<MR::CSLAMReconstructor> m_pReconstructor;
		std::unique_ptr<MR::CScalableSLAMReconstructor> m_pScalableReconstructor;
        
        CShaderPtr m_OutlineVSPtr;
        CShaderPtr m_OutlineFSPtr;

        CShaderPtr m_OutlineLevel1VSPtr;
        CShaderPtr m_OutlineLevel1FSPtr;
        CShaderPtr m_OutlineLevel2VSPtr;
        CShaderPtr m_OutlineLevel2FSPtr;

        CShaderPtr m_RaycastRootGridsVSPtr;
        CShaderPtr m_RaycastRootGridsFSPtr;

        CShaderPtr m_RaycastLevel1VSPtr;
        CShaderPtr m_RaycastLevel1FSPtr;

        CShaderPtr m_RaycastVSPtr;
        CShaderPtr m_RaycastFSPtr;

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
        
        CShaderPtr m_PointCloudVSPtr;
        CShaderPtr m_PointCloudFSPtr;

        bool m_UseTrackingCamera;

        bool m_RenderVertexMap;
        bool m_RaycastRootGrids;
        bool m_RaycastLevel1Grid;
        bool m_RenderRootQueue;
        bool m_RenderLevel1Queue;
        bool m_RenderLevel2Queue;
        bool m_RenderHONV;
    };
} // namespace

namespace
{
    using namespace Base;

    CGfxReconstructionRenderer::CGfxReconstructionRenderer()
        : m_UseTrackingCamera(true)
        , m_RenderVertexMap  (false)
        , m_RaycastRootGrids (false)
        , m_RaycastLevel1Grid(false)
        , m_RenderRootQueue  (false)
        , m_RenderLevel1Queue(false)
        , m_RenderLevel2Queue(false)
        , m_RenderHONV       (false)
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

        m_UseTrackingCamera = true;

        m_RenderVertexMap   = Base::CProgramParameters::GetInstance().GetBoolean("mr:slam:rendering:vertex_map"      , false);
        m_RaycastRootGrids  = Base::CProgramParameters::GetInstance().GetBoolean("mr:slam:rendering:root_grid"       , false);
        m_RaycastLevel1Grid = Base::CProgramParameters::GetInstance().GetBoolean("mr:slam:rendering:level1_grid"     , false);
        m_RenderRootQueue   = Base::CProgramParameters::GetInstance().GetBoolean("mr:slam:rendering:root_queue"      , false);
        m_RenderLevel1Queue = Base::CProgramParameters::GetInstance().GetBoolean("mr:slam:rendering:level1_queue"    , false);
        m_RenderLevel2Queue = Base::CProgramParameters::GetInstance().GetBoolean("mr:slam:rendering:level2_queue"    , false);
        m_RenderHONV        = Base::CProgramParameters::GetInstance().GetBoolean("mr:slam:rendering:normal_histogram", false);
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
        m_HistogramVSPtr = 0;
        m_HistogramFSPtr = 0;

        m_RaycastLevel1VSPtr = 0;
        m_RaycastLevel1FSPtr = 0;

        m_RaycastRootGridsVSPtr = 0;
        m_RaycastRootGridsFSPtr = 0;
        
        m_RaycastConstantBufferPtr = 0;
        m_DrawCallConstantBufferPtr = 0;
                
        m_CameraMeshPtr = 0;
        m_VolumeMeshPtr = 0;
        m_QuadMeshPtr = 0;
		m_CubeOutlineMeshPtr = 0;
        m_CameraInputLayoutPtr = 0;
        m_VolumeInputLayoutPtr = 0;
        m_QuadInputLayoutPtr = 0;
		m_CubeOutlineInputLayoutPtr = 0;

        m_OutlineRenderContextPtr = 0;

		m_pReconstructor = nullptr;
		m_pScalableReconstructor = nullptr;

        m_PointCloudVSPtr = 0;
        m_PointCloudFSPtr = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupShader()
    {
		MR::SReconstructionSettings Settings;

		if (m_pScalableReconstructor != nullptr)
		{
			m_pScalableReconstructor->GetReconstructionSettings(&Settings);

			std::stringstream DefineStream;

			DefineStream
			    << "#define VOLUME_RESOLUTION "      << Settings.m_VolumeResolution << " \n"
			    << "#define TRUNCATED_DISTANCE "     << Settings.m_TruncatedDistance << " \n"
			    << "#define VOLUME_SIZE "            << Settings.m_VolumeSize << " \n"
			    << "#define VOXEL_SIZE "             << Settings.m_VolumeSize / Settings.m_VolumeResolution << " \n"
		        << "#define MAX_INTEGRATION_WEIGHT " << Settings.m_MaxIntegrationWeight << '\n';

            if (Settings.m_CaptureColor)
            {
                DefineStream << "#define CAPTURE_COLOR\n";
            }

			std::string DefineString = DefineStream.str();

			m_OutlineVSPtr       = ShaderManager::CompileVS("scalable_kinect_fusion\\rendering\\vs_outline.glsl"       , "main", DefineString.c_str());
			m_OutlineFSPtr       = ShaderManager::CompilePS("scalable_kinect_fusion\\rendering\\fs_outline.glsl"       , "main", DefineString.c_str());
            m_OutlineLevel1VSPtr = ShaderManager::CompileVS("scalable_kinect_fusion\\rendering\\vs_outline_level1.glsl", "main", DefineString.c_str());
            m_OutlineLevel1FSPtr = ShaderManager::CompilePS("scalable_kinect_fusion\\rendering\\fs_outline_level1.glsl", "main", DefineString.c_str());
            m_OutlineLevel2VSPtr = ShaderManager::CompileVS("scalable_kinect_fusion\\rendering\\vs_outline_level2.glsl", "main", DefineString.c_str());
            m_OutlineLevel2FSPtr = ShaderManager::CompilePS("scalable_kinect_fusion\\rendering\\fs_outline_level2.glsl", "main", DefineString.c_str());
			
            m_PointCloudVSPtr = ShaderManager::CompileVS("scalable_kinect_fusion\\rendering\\vs_point_cloud.glsl", "main", DefineString.c_str());
            m_PointCloudFSPtr = ShaderManager::CompilePS("scalable_kinect_fusion\\rendering\\fs_point_cloud.glsl", "main", DefineString.c_str());

            m_RaycastRootGridsVSPtr = ShaderManager::CompileVS("scalable_kinect_fusion\\rendering\\vs_raycast_rootvolumes.glsl", "main", DefineString.c_str());
            m_RaycastRootGridsFSPtr = ShaderManager::CompilePS("scalable_kinect_fusion\\rendering\\fs_raycast_rootvolumes.glsl", "main", DefineString.c_str());

            m_RaycastLevel1VSPtr = ShaderManager::CompileVS("scalable_kinect_fusion\\rendering\\vs_raycast_level1.glsl", "main", DefineString.c_str());
            m_RaycastLevel1FSPtr = ShaderManager::CompilePS("scalable_kinect_fusion\\rendering\\fs_raycast_level1.glsl", "main", DefineString.c_str());

            m_RaycastVSPtr = ShaderManager::CompileVS("scalable_kinect_fusion\\rendering\\vs_raycast.glsl", "main", DefineString.c_str());
            m_RaycastFSPtr = ShaderManager::CompilePS("scalable_kinect_fusion\\rendering\\fs_raycast.glsl", "main", DefineString.c_str());

            m_HistogramVSPtr = ShaderManager::CompileVS("scalable_kinect_fusion\\rendering\\vs_histogram.glsl", "main", DefineString.c_str());
            m_HistogramFSPtr = ShaderManager::CompilePS("scalable_kinect_fusion\\rendering\\fs_histogram.glsl", "main", DefineString.c_str());
        }
		else
		{
			m_pReconstructor->GetReconstructionSettings(&Settings);

			std::stringstream DefineStream;

			DefineStream
				<< "#define VOLUME_RESOLUTION "  << Settings.m_VolumeResolution << " \n"
				<< "#define TRUNCATED_DISTANCE " << Settings.m_TruncatedDistance << " \n"
				<< "#define VOLUME_SIZE "        << Settings.m_VolumeSize << " \n"
				<< "#define VOXEL_SIZE "         << Settings.m_VolumeSize / Settings.m_VolumeResolution << " \n";

			if (Settings.m_CaptureColor)
			{
				DefineStream << "#define CAPTURE_COLOR\n";
			}

			std::string DefineString = DefineStream.str();

			m_OutlineVSPtr = ShaderManager::CompileVS("kinect_fusion\\vs_outline.glsl", "main", DefineString.c_str());
			m_OutlineFSPtr = ShaderManager::CompilePS("kinect_fusion\\fs_outline.glsl", "main", DefineString.c_str());
			m_RaycastVSPtr = ShaderManager::CompileVS("kinect_fusion\\vs_raycast.glsl", "main", DefineString.c_str());
			m_RaycastFSPtr = ShaderManager::CompilePS("kinect_fusion\\fs_raycast.glsl", "main", DefineString.c_str());
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

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupBuffers()
    {
        SBufferDescriptor ConstantBufferDesc = {};

        ConstantBufferDesc.m_Stride = 0;
        ConstantBufferDesc.m_Usage = CBuffer::EUsage::GPURead;
        ConstantBufferDesc.m_Binding = CBuffer::ConstantBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(Float4) * 2;
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
		//Todo: remove magic numbers (focal length/point, max/min depth)

		float x = (-0.50602675f) / 0.72113f;
		float y = (-0.499133f) / 0.870799f;
		
        Float3 CameraVertices[] =
        {
            Float3(-x * 8.0f, -y * 8.0f, 8.0f),
            Float3( x * 8.0f, -y * 8.0f, 8.0f),
            Float3( x * 8.0f,  y * 8.0f, 8.0f),
            Float3(-x * 8.0f,  y * 8.0f, 8.0f),
            Float3(     0.0f,      0.0f, 0.0f),
			Float3(-x * 0.5f, -y * 0.5f, 0.5f),
			Float3( x * 0.5f, -y * 0.5f, 0.5f),
			Float3( x * 0.5f,  y * 0.5f, 0.5f),
			Float3(-x * 0.5f,  y * 0.5f, 0.5f),
        };

		Float3 CameraLines[24] =
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

		Float3 CubeLines[24] =
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

        Float3 QuadLines[4] =
        {
            Float3(0.0f, 1.0f, 0.0f),
            Float3(0.0f, 0.0f, 0.0f),
            Float3(1.0f, 1.0f, 0.0f),
            Float3(1.0f, 0.0f, 0.0f),
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

        BufferManager::UploadBufferData(m_VolumeMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), &g_CubeVertices);
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
        if (m_UseTrackingCamera)
        {
            Cam::CControl& rControl = static_cast<Cam::CEditorControl&>(Cam::ControlManager::GetActiveControl());
            
            Float4x4 PoseMatrix = (m_pScalableReconstructor != nullptr) ? m_pScalableReconstructor->GetPoseMatrix() : m_pReconstructor->GetPoseMatrix();

            Base::Float3 Position;
            Base::Float3 Rotation;

            PoseMatrix.GetTranslation(Position);
            PoseMatrix.GetRotation(Rotation);

            Base::Float3x3 RotationMatrix;
            RotationMatrix.SetRotation(Rotation[0] + 3.14f, Rotation[1], Rotation[2]);            

            rControl.SetPosition(Position);
            rControl.SetRotation(RotationMatrix);
        }        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::RaycastVolume()
    {
        MR::SReconstructionSettings Settings;
        m_pReconstructor->GetReconstructionSettings(&Settings);

        Float4x4 PoseMatrix = m_pReconstructor->GetPoseMatrix();

        Float4 RaycastData[2];
        PoseMatrix.GetTranslation(RaycastData[0][0], RaycastData[0][1], RaycastData[0][2]);
        RaycastData[0][3] = 1.0f;
        if (Settings.m_CaptureColor)
        {
            RaycastData[1] = m_pReconstructor->IsTrackingLost() ? Float4(1.0f, 0.0f, 0.0f, 1.0f) : Float4(0.0f, 0.0f, 0.0f, 1.0f);
        }
        else
        {
            RaycastData[1] = m_pReconstructor->IsTrackingLost() ? Float4(1.0f, 0.0f, 0.0f, 1.0f) : Float4(0.0f, 1.0f, 0.0f, 1.0f);
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
        ContextManager::SetVertexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);
        ContextManager::SetInputLayout(m_VolumeInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::DrawIndexed(36, 0, 0);

		// Render volume box

		ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Wireframe));

		ContextManager::SetRenderContext(m_OutlineRenderContextPtr);
		ContextManager::SetShaderVS(m_OutlineVSPtr);
		ContextManager::SetShaderPS(m_OutlineFSPtr);

		SDrawCallConstantBuffer BufferData;

		BufferData.m_WorldMatrix.SetScale(Settings.m_VolumeSize);
		BufferData.m_Color = Float4(0.0f, 0.0f, 1.0f, 1.0f);

		BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

		ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
		ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);
		
		ContextManager::SetVertexBuffer(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
		ContextManager::SetInputLayout(m_CubeOutlineInputLayoutPtr);

		ContextManager::SetTopology(STopology::LineList);

		ContextManager::Draw(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfVertices(), 0);
    }

	// -----------------------------------------------------------------------------

	void CGfxReconstructionRenderer::RaycastScalableVolume()
	{
        MR::CScalableSLAMReconstructor::SScalableVolume& rVolume = m_pScalableReconstructor->GetVolume();

        MR::SReconstructionSettings Settings;
        m_pScalableReconstructor->GetReconstructionSettings(&Settings);

        Float4x4 PoseMatrix = m_pScalableReconstructor->GetPoseMatrix();

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

        const Float3 Min = Float3(
            rVolume.m_MinOffset[0] * Settings.m_VolumeSize,
            rVolume.m_MinOffset[1] * Settings.m_VolumeSize,
            rVolume.m_MinOffset[2] * Settings.m_VolumeSize
        );

        const Float3 Max = Float3(
            (rVolume.m_MaxOffset[0] + 1.0f) * Settings.m_VolumeSize, // Add 1.0f because MaxOffset stores the max volume offset
            (rVolume.m_MaxOffset[1] + 1.0f) * Settings.m_VolumeSize, // and we have to consider the volume size
            (rVolume.m_MaxOffset[2] + 1.0f) * Settings.m_VolumeSize
        );

        Float3 Vertices[8] =
        {
            Float3(Min[0], Min[1], Min[2]),
            Float3(Max[0], Min[1], Min[2]),
            Float3(Max[0], Max[1], Min[2]),
            Float3(Min[0], Max[1], Min[2]),
            Float3(Min[0], Min[1], Max[2]),
            Float3(Max[0], Min[1], Max[2]),
            Float3(Max[0], Max[1], Max[2]),
            Float3(Min[0], Max[1], Max[2]),
        };

        Float4 RaycastData[2];
        PoseMatrix.GetTranslation(RaycastData[0][0], RaycastData[0][1], RaycastData[0][2]);
        RaycastData[0][3] = 1.0f;
        if (Settings.m_CaptureColor)
        {
            RaycastData[1] = m_pScalableReconstructor->IsTrackingLost() ? Float4(1.0f, 0.0f, 0.0f, 1.0f) : Float4(0.0f, 0.0f, 0.0f, 1.0f);
        }
        else
        {
            RaycastData[1] = m_pScalableReconstructor->IsTrackingLost() ? Float4(1.0f, 0.0f, 0.0f, 1.0f) : Float4(0.0f, 1.0f, 0.0f, 1.0f);
        }
        BufferManager::UploadBufferData(m_RaycastConstantBufferPtr, RaycastData);

        BufferManager::UploadBufferData(m_VolumeMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), &Vertices);

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);
        ContextManager::SetInputLayout(m_VolumeInputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::DrawIndexed(36, 0, 0);
	}
    
    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RaycastRootGrids()
    {
        MR::CScalableSLAMReconstructor::SScalableVolume& rVolume = m_pScalableReconstructor->GetVolume();

        MR::SReconstructionSettings Settings;
        m_pScalableReconstructor->GetReconstructionSettings(&Settings);

        Float4x4 PoseMatrix = m_pScalableReconstructor->GetPoseMatrix();

        ContextManager::SetShaderVS(m_RaycastRootGridsVSPtr);
        ContextManager::SetShaderPS(m_RaycastRootGridsFSPtr);

        ContextManager::SetResourceBuffer(0, rVolume.m_RootVolumePoolPtr);
        ContextManager::SetResourceBuffer(1, rVolume.m_RootGridPoolPtr);
        ContextManager::SetResourceBuffer(6, rVolume.m_RootVolumePositionBufferPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(2, rVolume.m_AABBBufferPtr);

        ContextManager::Barrier();

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::Default));
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        const Float3 Min = Float3(
            rVolume.m_MinOffset[0] * Settings.m_VolumeSize,
            rVolume.m_MinOffset[1] * Settings.m_VolumeSize,
            rVolume.m_MinOffset[2] * Settings.m_VolumeSize
        );

        const Float3 Max = Float3(
            (rVolume.m_MaxOffset[0] + 1.0f) * Settings.m_VolumeSize, // Add 1.0f because MaxOffset stores the max volume offset
            (rVolume.m_MaxOffset[1] + 1.0f) * Settings.m_VolumeSize, // and we have to consider the volume size
            (rVolume.m_MaxOffset[2] + 1.0f) * Settings.m_VolumeSize
        );

        Float3 Vertices[8] =
        {
            Float3(Min[0], Min[1], Min[2]),
            Float3(Max[0], Min[1], Min[2]),
            Float3(Max[0], Max[1], Min[2]),
            Float3(Min[0], Max[1], Min[2]),
            Float3(Min[0], Min[1], Max[2]),
            Float3(Max[0], Min[1], Max[2]),
            Float3(Max[0], Max[1], Max[2]),
            Float3(Min[0], Max[1], Max[2]),
        };

        BufferManager::UploadBufferData(m_VolumeMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), &Vertices);

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);
        ContextManager::SetInputLayout(m_VolumeInputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::DrawIndexed(36, 0, 0);
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RaycastLevel1Grids()
    {
        MR::CScalableSLAMReconstructor::SScalableVolume& rVolume = m_pScalableReconstructor->GetVolume();

        MR::SReconstructionSettings Settings;
        m_pScalableReconstructor->GetReconstructionSettings(&Settings);

        Float4x4 PoseMatrix = m_pScalableReconstructor->GetPoseMatrix();

        ContextManager::SetShaderVS(m_RaycastLevel1VSPtr);
        ContextManager::SetShaderPS(m_RaycastLevel1FSPtr);

        ContextManager::SetResourceBuffer(0, rVolume.m_RootVolumePoolPtr);
        ContextManager::SetResourceBuffer(1, rVolume.m_RootGridPoolPtr);
        ContextManager::SetResourceBuffer(2, rVolume.m_Level1PoolPtr);
        ContextManager::SetResourceBuffer(6, rVolume.m_RootVolumePositionBufferPtr);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(2, rVolume.m_AABBBufferPtr);

        ContextManager::Barrier();

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::Default));
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        const Float3 Min = Float3(
            rVolume.m_MinOffset[0] * Settings.m_VolumeSize,
            rVolume.m_MinOffset[1] * Settings.m_VolumeSize,
            rVolume.m_MinOffset[2] * Settings.m_VolumeSize
        );

        const Float3 Max = Float3(
            (rVolume.m_MaxOffset[0] + 1.0f) * Settings.m_VolumeSize, // Add 1.0f because MaxOffset stores the max volume offset
            (rVolume.m_MaxOffset[1] + 1.0f) * Settings.m_VolumeSize, // and we have to consider the volume size
            (rVolume.m_MaxOffset[2] + 1.0f) * Settings.m_VolumeSize
        );

        Float3 Vertices[8] =
        {
            Float3(Min[0], Min[1], Min[2]),
            Float3(Max[0], Min[1], Min[2]),
            Float3(Max[0], Max[1], Min[2]),
            Float3(Min[0], Max[1], Min[2]),
            Float3(Min[0], Min[1], Max[2]),
            Float3(Max[0], Min[1], Max[2]),
            Float3(Max[0], Max[1], Max[2]),
            Float3(Min[0], Max[1], Max[2]),
        };

        BufferManager::UploadBufferData(m_VolumeMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), &Vertices);

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);
        ContextManager::SetInputLayout(m_VolumeInputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::DrawIndexed(36, 0, 0);
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

		ContextManager::SetVertexBuffer(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
		ContextManager::SetInputLayout(m_CubeOutlineInputLayoutPtr);

		ContextManager::SetTopology(STopology::LineList);

		Float3 Position;
		Float4x4 Scaling;
		Float4x4 Translation;

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

                Scaling.SetScale(GridSizes[0]);
                Translation.SetTranslation(Position);

                BufferData.m_WorldMatrix = Translation * Scaling;
                BufferData.m_Color = Float4(0.0f, 0.0f, 1.0f, 1.0f);

                BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

                ContextManager::Draw(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfVertices(), 0);
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

        ContextManager::SetVertexBuffer(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        ContextManager::SetInputLayout(m_CubeOutlineInputLayoutPtr);

        ContextManager::SetTopology(STopology::LineList);

        Float3 Position;
        Float4x4 Scaling;
        Float4x4 Translation;

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

                Scaling.SetScale(VolumeSizes[1]);
                Translation.SetTranslation(Position);

                BufferData.m_WorldMatrix = Translation * Scaling;
                BufferData.m_Color = Float4(0.0f, 0.0f, 1.0f, 1.0f);

                BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

                assert(rRootGrid.m_Level1QueuePtr != nullptr);
                ContextManager::SetResourceBuffer(2, rRootGrid.m_Level1QueuePtr);

                int VertexCount = m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfVertices();
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

        ContextManager::SetVertexBuffer(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        ContextManager::SetInputLayout(m_CubeOutlineInputLayoutPtr);

        ContextManager::SetTopology(STopology::LineList);

        Float3 Position;
        Float4x4 Translation;

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

                Translation.SetTranslation(Position);

                BufferData.m_WorldMatrix = Translation;
                BufferData.m_Color = Float4(0.0f, 0.0f, 1.0f, 1.0f);

                BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

                assert(rRootGrid.m_Level2QueuePtr != nullptr);
                ContextManager::SetResourceBuffer(2, rRootGrid.m_Level2QueuePtr);

                int VertexCount = m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfVertices();
                BufferManager::UploadBufferData(rRootGrid.m_IndirectLevel2Buffer, &VertexCount, 0, sizeof(uint32_t));

                ContextManager::DrawIndirect(rRootGrid.m_IndirectLevel2Buffer, MR::CScalableSLAMReconstructor::SIndirectBuffers::s_DrawOffset);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderHONV()
    {
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetRenderContext(m_OutlineRenderContextPtr);
        ContextManager::SetShaderVS(m_HistogramVSPtr);
        ContextManager::SetShaderPS(m_HistogramFSPtr);
        
        ContextManager::SetImageTexture(0, m_pScalableReconstructor->GetHONV());

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_QuadMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_QuadMeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);

        ContextManager::SetInputLayout(m_QuadInputLayoutPtr);
        ContextManager::SetTopology(STopology::TriangleStrip);

        ContextManager::Draw(4, 0);
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
		BufferData.m_Color = Float4(1.0f, 0.0f, 1.0f, 1.0f);

		BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_CameraMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_CameraMeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);

        ContextManager::SetInputLayout(m_CameraInputLayoutPtr);
        ContextManager::SetTopology(STopology::LineList);

        ContextManager::Draw(m_CameraMeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfVertices(), 0);
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderVertexMap()
    {
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));
        ContextManager::SetRenderContext(m_OutlineRenderContextPtr);

        ContextManager::SetShaderVS(m_PointCloudVSPtr);
        ContextManager::SetShaderPS(m_PointCloudFSPtr);

        SDrawCallConstantBuffer BufferData;

        BufferData.m_WorldMatrix = m_pScalableReconstructor->GetPoseMatrix();
        BufferData.m_Color = Float4(1.0f, 0.0f, 1.0f, 1.0f);

        BufferManager::UploadBufferData(m_DrawCallConstantBufferPtr, &BufferData);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);

        ContextManager::SetImageTexture(0, m_pScalableReconstructor->GetVertexMap());

        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_CameraMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_CameraMeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);

        ContextManager::SetInputLayout(m_CameraInputLayoutPtr);
        ContextManager::SetTopology(STopology::PointList);

        ContextManager::Draw(512 * 424, 0);
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::Render()
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
        //Base::Float4 ClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        //TargetSetManager::ClearTargetSet(TargetSetManager::GetDeferredTargetSet(), ClearColor);

        if (!m_UseTrackingCamera)
        {
            RenderCamera();
        }

		if (m_pScalableReconstructor != nullptr)
		{
            RaycastScalableVolume();

            if (m_RenderVertexMap)
            {
                RenderVertexMap();
            }

            if (m_RaycastRootGrids)
            {
                RaycastRootGrids();
            }

            if (m_RaycastLevel1Grid)
            {
                RaycastLevel1Grids();
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

            if (m_RenderHONV)
            {
                RenderHONV();
            }
		}
		else
		{
			RaycastVolume();
		}
        
        Performance::EndEvent();
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
        CGfxReconstructionRenderer::GetInstance().Render();
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

} // namespace Voxel
} // namespace Gfx

