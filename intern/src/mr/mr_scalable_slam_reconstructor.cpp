
#include "mr/mr_precompiled.h"

#include "base/base_vector3.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "graphic/gfx_buffer_manager.h"
#include "graphic/gfx_context_manager.h"
#include "graphic/gfx_main.h"
#include "graphic/gfx_mesh_manager.h"
#include "graphic/gfx_performance.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

#include "base/base_console.h"

#include "mr/mr_scalable_slam_reconstructor.h"
#include "mr/mr_rgbd_camera_control.h"
#include "mr/mr_kinect_control.h"
#include "mr/mr_realsense_control.h"

#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

#include <gl/glew.h>

using namespace MR;
using namespace Gfx;

//#define USE_PERISTENT_MAPPING

namespace
{
	//*
	const Base::Float3 g_InitialCameraPosition = Base::Float3(0.5f, 0.5f, 1.5f);
	const Base::Float3 g_InitialCameraRotation = Base::Float3(3.14f, 0.0f, 0.0f);
	/*/
	const Base::Float3 g_InitialCameraPosition = Base::Float3(0.5f, 0.5f, -0.5f);
	const Base::Float3 g_InitialCameraRotation = Base::Float3(0.0f, 0.0f, 0.0f);
	//*/
    
    const float g_EpsilonDistance = 0.1f;
    const float g_EpsilonAngle = 0.75f;
    
    const int g_ICPValueCount = 27;

    const unsigned int g_TileSize1D = 512;
    const unsigned int g_TileSize2D = 16;
    const unsigned int g_TileSize3D = 8;

    const bool g_UseHighPrecisionMaps = false;

    struct SIntrinsics
    {
        Base::Float4x4 m_KMatrix;
        Base::Float4x4 m_InvKMatrix;
        Base::Float2 m_FocalPoint;
        Base::Float2 m_FocalLength;
        Base::Float2 m_InvFocalLength;
        Base::Float2 Padding;
    };

    struct STrackingData
    {
        Base::Float4x4 m_PoseMatrix;
        Base::Float4x4 m_InvPoseMatrix;
    };

    struct SIncBuffer
    {
        Base::Float4x4 m_PoseMatrix;
        Base::Float4x4 m_InvPoseMatrix;
        int m_PyramidLevel;
        float Padding[3];
    };

	struct SPositionBuffer
	{
		Base::Float3 m_Position;
		int m_Index;
	};

    struct SDrawCallBufferData
    {
        Base::Float4x4 m_WorldMatrix;
    };

    struct SInstanceData
    {
        Base::Int3 m_Offset;
        int m_Index;
    };
    
    struct SIndexedIndirect 
    {
        uint32_t m_IndexCount;
        uint32_t m_InstanceCount;
        uint32_t m_FirstIndex;
        uint32_t m_BaseVertex;
        uint32_t m_BaseInstance;
    };

    struct SGridRasterization
    {
        int32_t m_Resolution;
        float m_CubeSize;
        float m_ParentSize;
        Base::Int3 m_Offset;
        float Padding[2];
    };

} // namespace

namespace MR
{
    using namespace Base;
    
    // -----------------------------------------------------------------------------

    CScalableSLAMReconstructor::CScalableSLAMReconstructor(const SReconstructionSettings* pReconstructionSettings)
    {
        if (pReconstructionSettings != nullptr)
        {
			assert(pReconstructionSettings->m_IsScalable);
            m_ReconstructionSettings = *pReconstructionSettings;
        }
        Start();
    }
    
    // -----------------------------------------------------------------------------
    
    CScalableSLAMReconstructor::~CScalableSLAMReconstructor()
    {
        Exit();
    }
    
    // -----------------------------------------------------------------------------
    
    bool CScalableSLAMReconstructor::IsTrackingLost() const
    {
        return !m_IsTrackingPaused && m_TrackingLost;
    }

    // -----------------------------------------------------------------------------

    Float4x4 CScalableSLAMReconstructor::GetPoseMatrix() const
    {
        return m_PoseMatrix;
    }

    // -----------------------------------------------------------------------------

	CScalableSLAMReconstructor::CRootGridMap& CScalableSLAMReconstructor::GetRootGridMap()
    {
        return m_RootGridMap;
    }

    // -----------------------------------------------------------------------------

    CScalableSLAMReconstructor::CRootGridVector& CScalableSLAMReconstructor::GetRootGridVector()
    {
        return m_RootGridVector;
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::Start()
    {
        m_pRGBDCameraControl.reset(new MR::CKinectControl);
        BASE_CONSOLE_INFO("Using Kinect for SLAM");

        m_DepthPixels = std::vector<unsigned short>(m_pRGBDCameraControl->GetDepthPixelCount());
        m_CameraPixels = std::vector<Base::Byte4>(m_pRGBDCameraControl->GetDepthPixelCount());

        SetupMeshes();
		SetupData();
        SetupRenderStates();
		SetupShaders();
		SetupTextures();
		SetupBuffers();

		m_IsIntegrationPaused = false;
		m_IsTrackingPaused = false;
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::SetupRenderStates()
    {
        STextureDescriptor RendertargetDescriptor = {};

        RendertargetDescriptor.m_NumberOfPixelsU = m_pRGBDCameraControl->GetDepthWidth();
        RendertargetDescriptor.m_NumberOfPixelsV = m_pRGBDCameraControl->GetDepthHeight();
        RendertargetDescriptor.m_NumberOfPixelsW = 1;
        RendertargetDescriptor.m_NumberOfMipMaps = 1;
        RendertargetDescriptor.m_NumberOfTextures = 1;
        RendertargetDescriptor.m_Binding = CTextureBase::RenderTarget | CTextureBase::ShaderResource;
        RendertargetDescriptor.m_Access = CTextureBase::CPUWrite;
        RendertargetDescriptor.m_Format = CTextureBase::Unknown;
        RendertargetDescriptor.m_Usage = CTextureBase::GPURead;
        RendertargetDescriptor.m_Semantic = CTextureBase::Diffuse;
        RendertargetDescriptor.m_Format = CTextureBase::R16G16B16A16_FLOAT;

        CTextureBasePtr RenderTarget = TextureManager::CreateTexture2D(RendertargetDescriptor);
        
        m_TargetSetPtr = TargetSetManager::CreateTargetSet(RenderTarget);

        SViewPortDescriptor ViewPortDescriptor = {};

        ViewPortDescriptor.m_MinDepth = 0.0f;
        ViewPortDescriptor.m_MaxDepth = 1.0f;
        ViewPortDescriptor.m_TopLeftX = 0.0f;
        ViewPortDescriptor.m_TopLeftY = 0.0f;
        ViewPortDescriptor.m_Width = static_cast<float>(m_pRGBDCameraControl->GetDepthWidth());
        ViewPortDescriptor.m_Height = static_cast<float>(m_pRGBDCameraControl->GetDepthHeight());

        Gfx::CViewPortPtr DepthViewPort = ViewManager::CreateViewPort(ViewPortDescriptor);

        m_DepthViewPortSetPtr = ViewManager::CreateViewPortSet(DepthViewPort);
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::SetupMeshes()
    {
        Float3 CubeVertices[] =
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

        Dt::CSurface* pSurface = new Dt::CSurface;
        Dt::CLOD* pLOD = new Dt::CLOD;
        Dt::CMesh* pMesh = new Dt::CMesh;

        pSurface->SetPositions(CubeVertices);
        pSurface->SetNumberOfVertices(sizeof(CubeVertices) / sizeof(CubeVertices[0]));
        pSurface->SetIndices(CubeIndices);
        pSurface->SetNumberOfIndices(sizeof(CubeIndices) / sizeof(CubeIndices[0]));
        pSurface->SetElements(0);

        pLOD->SetSurface(0, pSurface);
        pLOD->SetNumberOfSurfaces(1);

        pMesh->SetLOD(0, pLOD);
        pMesh->SetNumberOfLODs(1);

        SMeshDescriptor MeshDesc = {};
        MeshDesc.m_pMesh = pMesh;

        m_CubeMeshPtr = MeshManager::CreateMesh(MeshDesc);
    }

	// -----------------------------------------------------------------------------

	void CScalableSLAMReconstructor::SetupData()
	{
		const float VolumeSize = m_ReconstructionSettings.m_VolumeSize;
		Float4x4 PoseRotation, PoseTranslation;
		
		PoseRotation.SetRotation(g_InitialCameraRotation[0], g_InitialCameraRotation[1], g_InitialCameraRotation[2]);
		PoseTranslation.SetTranslation
		(
			g_InitialCameraPosition[0] * VolumeSize,
			g_InitialCameraPosition[1] * VolumeSize,
			g_InitialCameraPosition[2] * VolumeSize
		);
		m_PoseMatrix = PoseTranslation * PoseRotation;

		m_IntegratedFrameCount = 0;
		m_FrameCount = 0;
		m_TrackingLost = true;
		
		const int GridLevelCount = MR::SReconstructionSettings::GRID_LEVELS;

		m_GridSizes.resize(GridLevelCount);
		m_GridSizes[GridLevelCount - 1] = m_ReconstructionSettings.m_VoxelSize * m_ReconstructionSettings.m_GridResolutions[GridLevelCount - 1];
		for (int i = GridLevelCount - 2; i >= 0; -- i)
		{
			m_GridSizes[i] = m_GridSizes[i + 1] * m_ReconstructionSettings.m_GridResolutions[i];
		}

		UpdateFrustum();
	}

	// -----------------------------------------------------------------------------

	void CScalableSLAMReconstructor::UpdateFrustum()
	{
		//Todo: remove magic numbers (focal length/point)

		float x = (-0.50602675f) / 0.72113f;
		float y = (-0.499133f) / 0.870799f;

		const float Near = m_pRGBDCameraControl->GetMinDepth();
		const float Far = m_pRGBDCameraControl->GetMaxDepth();

		// near

		m_FrustumPoints[0] = Float3( x * Near,  y * Near, Near);
		m_FrustumPoints[1] = Float3(-x * Near,  y * Near, Near);
		m_FrustumPoints[2] = Float3(-x * Near, -y * Near, Near);
		m_FrustumPoints[3] = Float3( x * Near, -y * Near, Near);

		// far

		m_FrustumPoints[4] = Float3( x * Far,  y * Far, Far);
		m_FrustumPoints[5] = Float3(-x * Far,  y * Far, Far);
		m_FrustumPoints[6] = Float3(-x * Far, -y * Far, Far);
		m_FrustumPoints[7] = Float3( x * Far, -y * Far, Far);

		for (int i = 0; i < g_FrustumCorners; ++i)
		{
			Float4 Corner = Float4(m_FrustumPoints[i], 1.0f);
			Corner = m_PoseMatrix * Corner;
			m_FrustumPoints[i] = Float3(Corner[0], Corner[1], Corner[2]);
		}

		m_FrustumPlanes[0] = GetHessianNormalForm(m_FrustumPoints[0], m_FrustumPoints[2], m_FrustumPoints[1]); // near
		m_FrustumPlanes[1] = GetHessianNormalForm(m_FrustumPoints[6], m_FrustumPoints[7], m_FrustumPoints[4]); // far
		m_FrustumPlanes[2] = GetHessianNormalForm(m_FrustumPoints[4], m_FrustumPoints[3], m_FrustumPoints[0]); // right
		m_FrustumPlanes[3] = GetHessianNormalForm(m_FrustumPoints[1], m_FrustumPoints[6], m_FrustumPoints[5]); // left
		m_FrustumPlanes[4] = GetHessianNormalForm(m_FrustumPoints[4], m_FrustumPoints[1], m_FrustumPoints[5]); // top
		m_FrustumPlanes[5] = GetHessianNormalForm(m_FrustumPoints[7], m_FrustumPoints[6], m_FrustumPoints[2]); // bottom
	}

	// -----------------------------------------------------------------------------

	Base::Float4 CScalableSLAMReconstructor::GetHessianNormalForm(const Base::Float3& rA, const Base::Float3& rB, const Base::Float3& rC)
	{
		Float3 V1 = rB - rA;
		Float3 V2 = rC - rA;

		Float3 Normal = V1.CrossProduct(V2).Normalize();

		float D = rA.DotProduct(Normal);

		return Float4(Normal, D);
	}

	// -----------------------------------------------------------------------------

	float CScalableSLAMReconstructor::GetPointPlaneDistance(const Base::Float3& rPoint, const Base::Float4& rPlane)
	{
		const float Dot = rPoint.DotProduct(Float3(rPlane[0], rPlane[1], rPlane[2]));
		return Dot - rPlane[3];
	}

    // -----------------------------------------------------------------------------
    
    void CScalableSLAMReconstructor::Exit()
    {
        m_BilateralFilterCSPtr = 0;
        m_VertexMapCSPtr = 0;
        m_NormalMapCSPtr = 0;
        m_DownSampleDepthCSPtr = 0;
        m_IntegrationCSPtr = 0;
        m_RaycastCSPtr = 0;
        m_RaycastPyramidCSPtr = 0;
        m_DetermineSummandsCSPtr = 0;
        m_ReduceSumCSPtr = 0;
        m_ClearVolumeCSPtr = 0;
		m_RootgridDepthCSPtr = 0;
        m_VolumeCountersCSPtr = 0;

        m_RasterizeRootVolumeVSPtr = 0;
        m_RasterizeRootVolumeFSPtr = 0;
        m_ClearAtomicCountersCSPtr = 0;

        m_CubeMeshPtr = 0;
        m_CubeInputLayoutPtr = 0;

        m_RawVertexMapPtr = 0;
        m_RawDepthBufferPtr = 0;
        m_RawCameraFramePtr = 0;
        m_TargetSetPtr = 0;
        m_DepthViewPortSetPtr = 0;

        for (int i = 0; i < m_ReconstructionSettings.m_PyramidLevelCount; ++ i)
        {
            m_SmoothDepthBufferPtr[i] = 0;
            m_ReferenceVertexMapPtr[i] = 0;
            m_ReferenceNormalMapPtr[i] = 0;
            m_RaycastVertexMapPtr[i] = 0;
            m_RaycastNormalMapPtr[i] = 0;
        }

        m_RootGridMap.clear();

        m_IntrinsicsConstantBufferPtr = 0;
        m_TrackingDataConstantBufferPtr = 0;
        m_RaycastPyramidConstantBufferPtr = 0;
        m_ICPResourceBufferPtr = 0;
        m_ICPSummationConstantBufferPtr = 0;
        m_IncPoseMatrixConstantBufferPtr = 0;
        m_BilateralFilterConstantBufferPtr = 0;
		m_PositionConstantBufferPtr = 0;
		m_VolumeQueueBufferPtr = 0;
        m_HierarchyConstantBufferPtr = 0;
        m_AtomicCounterBufferPtr = 0;
        m_RootVolumeInstanceBufferPtr = 0;
        m_IndexedIndirectBufferPtr = 0;
        m_GridRasterizationBufferPtr = 0;
        m_GridQueueBufferPtr = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CScalableSLAMReconstructor::SetupShaders()
    {
        const int SummandsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth(), g_TileSize2D);
        const int SummandsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

        const int Summands = SummandsX * SummandsY;
        const float SummandsLog2 = Log2(static_cast<float>(Summands));
        const int SummandsPOT = 1 << (static_cast<int>(SummandsLog2) + 1);
        
        const float VoxelSize = m_ReconstructionSettings.m_VolumeSize / m_ReconstructionSettings.m_VolumeResolution;

        const std::string InternalFormatString = g_UseHighPrecisionMaps ? "rgba32f" : "rgba16f";

        std::stringstream DefineStream;

        DefineStream
            << "#define PYRAMID_LEVELS "         << m_ReconstructionSettings.m_PyramidLevelCount    << " \n"
            << "#define VOLUME_RESOLUTION "      << m_ReconstructionSettings.m_VolumeResolution     << " \n"
            << "#define VOXEL_SIZE "             << VoxelSize                                       << " \n"
            << "#define VOLUME_SIZE "            << m_GridSizes[0]                                  << " \n"
            << "#define DEPTH_IMAGE_WIDTH "      << m_pRGBDCameraControl->GetDepthWidth()           << " \n"
            << "#define DEPTH_IMAGE_HEIGHT "     << m_pRGBDCameraControl->GetDepthHeight()          << " \n"
            << "#define TILE_SIZE1D "            << g_TileSize1D                                    << " \n"
            << "#define TILE_SIZE2D "            << g_TileSize2D                                    << " \n"
            << "#define TILE_SIZE3D "            << g_TileSize3D                                    << " \n"
            << "#define TRUNCATED_DISTANCE "     << m_ReconstructionSettings.m_TruncatedDistance    << " \n"
            << "#define MAX_INTEGRATION_WEIGHT " << m_ReconstructionSettings.m_MaxIntegrationWeight << " \n"
            << "#define EPSILON_DISTANCE "       << g_EpsilonDistance                               << " \n"
            << "#define EPSILON_ANGLE "          << g_EpsilonAngle                                  << " \n"
            << "#define ICP_VALUE_COUNT "        << g_ICPValueCount                                 << " \n"
            << "#define REDUCTION_SHADER_COUNT " << SummandsPOT / 2                                 << " \n"
            << "#define ICP_SUMMAND_COUNT "      << Summands                                        << " \n"
            << "#define MAP_TEXTURE_FORMAT "     << InternalFormatString                            << " \n"
            << "#define HIERARCHY_LEVELS "       << MR::SReconstructionSettings::GRID_LEVELS        << " \n";

        if (m_ReconstructionSettings.m_CaptureColor)
        {
            DefineStream << "#define CAPTURE_COLOR\n";
        }

        std::string DefineString = DefineStream.str();
        
        m_BilateralFilterCSPtr     = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_bilateral_filter.glsl"      , "main", DefineString.c_str());
        m_VertexMapCSPtr           = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_vertex_map.glsl"            , "main", DefineString.c_str());
        m_NormalMapCSPtr           = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_normal_map.glsl"            , "main", DefineString.c_str());
        m_DownSampleDepthCSPtr     = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_downsample_depth.glsl"      , "main", DefineString.c_str());
        m_IntegrationCSPtr         = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_integrate.glsl"             , "main", DefineString.c_str());        
        m_RaycastCSPtr             = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_raycast.glsl"               , "main", DefineString.c_str());
        m_RaycastPyramidCSPtr      = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_raycast_pyramid.glsl"       , "main", DefineString.c_str());
        m_DetermineSummandsCSPtr   = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_determine_summands.glsl"    , "main", DefineString.c_str());
        m_ReduceSumCSPtr           = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_reduce_sum.glsl"            , "main", DefineString.c_str());
        m_ClearVolumeCSPtr         = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_clear_volume.glsl"          , "main", DefineString.c_str());
		m_RootgridDepthCSPtr       = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_rootgrid_depth.glsl"        , "main", DefineString.c_str());
        m_RasterizeRootVolumeVSPtr = ShaderManager::CompileVS("scalable_kinect_fusion\\vs_rasterize_rootvolume.glsl"  , "main", DefineString.c_str());
        m_RasterizeRootVolumeFSPtr = ShaderManager::CompilePS("scalable_kinect_fusion\\fs_rasterize_rootvolume.glsl"  , "main", DefineString.c_str());
        m_ClearAtomicCountersCSPtr = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_clear_atomic_buffer.glsl"   , "main", DefineString.c_str());
        m_VolumeCountersCSPtr      = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_volume_counters.glsl"       , "main", DefineString.c_str());
        m_RasterizeRootGridVSPtr   = ShaderManager::CompileVS("scalable_kinect_fusion\\vs_rasterize_grid.glsl"        , "main", DefineString.c_str());
        m_RasterizeRootGridFSPtr   = ShaderManager::CompilePS("scalable_kinect_fusion\\fs_rasterize_grid.glsl"        , "main", DefineString.c_str());
        m_GridCountersCSPtr        = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_grid_counters.glsl"         , "main", DefineString.c_str());

        SInputElementDescriptor InputLayoutDesc = {};

        InputLayoutDesc.m_pSemanticName = "POSITION";
        InputLayoutDesc.m_SemanticIndex = 0;
        InputLayoutDesc.m_Format = CInputLayout::Float3Format;
        InputLayoutDesc.m_InputSlot = 0;
        InputLayoutDesc.m_AlignedByteOffset = 0;
        InputLayoutDesc.m_Stride = 12;
        InputLayoutDesc.m_InputSlotClass = CInputLayout::PerVertex;
        InputLayoutDesc.m_InstanceDataStepRate = 0;

        m_CubeInputLayoutPtr = ShaderManager::CreateInputLayout(&InputLayoutDesc, 1, m_RasterizeRootVolumeVSPtr);
    }
    
    // -----------------------------------------------------------------------------
    
	bool CScalableSLAMReconstructor::RootGridInFrustum(const Int3& rKey)
	{
		float AABB[6];

		for (int PlaneIndex = 0; PlaneIndex < 3; ++PlaneIndex)
		{
			AABB[PlaneIndex * 2] = rKey[PlaneIndex] * m_ReconstructionSettings.m_VolumeSize;
			AABB[PlaneIndex * 2 + 1] = AABB[PlaneIndex * 2] + m_ReconstructionSettings.m_VolumeSize;
		}

		Float3 Cube[8] =
		{
			Float3(AABB[0], AABB[2], AABB[4]),
			Float3(AABB[0], AABB[3], AABB[4]),
			Float3(AABB[0], AABB[2], AABB[5]),
			Float3(AABB[0], AABB[3], AABB[5]),
			Float3(AABB[1], AABB[2], AABB[4]),
			Float3(AABB[1], AABB[3], AABB[4]),
			Float3(AABB[1], AABB[2], AABB[5]),
			Float3(AABB[1], AABB[3], AABB[5]),
		};

		for (int PlaneIndex = 0; PlaneIndex < 6; ++ PlaneIndex)
		{
			int Outside = 0;
			for (int CubeIndex = 0; CubeIndex < 8; ++ CubeIndex)
			{
				if (GetPointPlaneDistance(Cube[CubeIndex], m_FrustumPlanes[PlaneIndex]) > 0)
				{
					++ Outside;
				}
			}
			if (Outside == 8)
			{
				return false;
			}
		}

		return true;
	}

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::RasterizeRootVolumes()
    {
        TargetSetManager::ClearTargetSet(m_TargetSetPtr);

        ContextManager::SetViewPortSet(m_DepthViewPortSetPtr);
        ContextManager::SetTargetSet(m_TargetSetPtr);

        const unsigned int Offset = 0;
        ContextManager::SetVertexBufferSet(m_CubeMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), &Offset);
        ContextManager::SetIndexBuffer(m_CubeMeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);
        ContextManager::SetInputLayout(m_CubeInputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetConstantBuffer(0, m_IntrinsicsConstantBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);

        ContextManager::SetShaderVS(m_RasterizeRootVolumeVSPtr);
        ContextManager::SetShaderPS(m_RasterizeRootVolumeFSPtr);

        ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_RawVertexMapPtr));

        ContextManager::SetResourceBuffer(0, m_AtomicCounterBufferPtr);
        ContextManager::SetResourceBuffer(1, m_RootVolumeInstanceBufferPtr);

        ContextManager::Barrier();

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        const unsigned int IndexCount = m_CubeMeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices();
        const unsigned int InstanceCount = static_cast<unsigned int>(m_RootGridMap.size());
        ContextManager::DrawIndexedInstanced(IndexCount, InstanceCount, 0, 0, 0);

        ContextManager::ResetShaderVS();
        ContextManager::ResetShaderPS();
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::GatherCounters(unsigned int Size)
    {
        ContextManager::Barrier();

        ContextManager::SetShaderCS(m_VolumeCountersCSPtr);

        SIndexedIndirect IndirectBufferData = {};
        IndirectBufferData.m_IndexCount = 36;
        BufferManager::UploadConstantBufferData(m_IndexedIndirectBufferPtr, &IndirectBufferData);

        ContextManager::SetResourceBuffer(0, m_AtomicCounterBufferPtr);
        ContextManager::SetResourceBuffer(1, m_IndexedIndirectBufferPtr);
        ContextManager::SetResourceBuffer(2, m_VolumeQueueBufferPtr);
        
        ContextManager::Dispatch(Size, 1, 1);
    }

	// -----------------------------------------------------------------------------
    
    void CScalableSLAMReconstructor::IntegrateRootGrids(std::vector<uint32_t>& rVolumeQueue)
    {
        ////////////////////////////////////////////////////////////////////////////////
        // Prepare pipeline
        ////////////////////////////////////////////////////////////////////////////////

        ContextManager::SetViewPortSet(m_DepthViewPortSetPtr);
        ContextManager::SetTargetSet(m_TargetSetPtr);

        const unsigned int Offset = 0;
        ContextManager::SetVertexBufferSet(m_CubeMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), &Offset);
        ContextManager::SetIndexBuffer(m_CubeMeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);
        ContextManager::SetInputLayout(m_CubeInputLayoutPtr);

        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetConstantBuffer(0, m_IntrinsicsConstantBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);
        ContextManager::SetConstantBuffer(2, m_GridRasterizationBufferPtr);

        ContextManager::SetShaderVS(m_RasterizeRootGridVSPtr);
        ContextManager::SetShaderPS(m_RasterizeRootGridFSPtr);

        ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_RawVertexMapPtr));

        ContextManager::SetResourceBuffer(0, m_GridAtomicCounterBufferPtr);

        ContextManager::Barrier();

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        ////////////////////////////////////////////////////////////////////////////////
        // Integrate individual grids
        ////////////////////////////////////////////////////////////////////////////////
        
        for (uint32_t VolumeIndex : rVolumeQueue)

        {
            assert(m_RootGridVector[VolumeIndex] != nullptr);
            TargetSetManager::ClearTargetSet(m_TargetSetPtr);
            IntegrateSingleRootGrid(*m_RootGridVector[VolumeIndex]);
        }

        ContextManager::ResetShaderVS();
        ContextManager::ResetShaderPS();
    }
    
    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::IntegrateSingleRootGrid(SRootGrid& rRootGrid)
    {
        SGridRasterization GridData = {};
        GridData.m_Resolution = m_ReconstructionSettings.m_GridResolutions[0];
        GridData.m_CubeSize = m_GridSizes[1];
        GridData.m_ParentSize = m_GridSizes[0];
        GridData.m_Offset = rRootGrid.m_Offset;

        BufferManager::UploadConstantBufferData(m_GridRasterizationBufferPtr, &GridData);

        int InstanceCount = GridData.m_Resolution * GridData.m_Resolution * GridData.m_Resolution;

        ClearBuffer(m_GridAtomicCounterBufferPtr, InstanceCount);

        const unsigned int IndexCount = m_CubeMeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices();
        ContextManager::DrawIndexedInstanced(IndexCount, InstanceCount, 0, 0, 0);
    }

    // -----------------------------------------------------------------------------

	void CScalableSLAMReconstructor::UpdateRootrids()
	{
        ////////////////////////////////////////////////////////////////////////////////
        // Create all root grid volumes that are in the view frustum 
        ////////////////////////////////////////////////////////////////////////////////

		Float3 BBMax = m_FrustumPoints[0];
		Float3 BBMin = m_FrustumPoints[0];

		for (int i = 1; i < g_FrustumCorners; ++ i)
		{
			for (int j = 0; j < 3; ++ j)
			{
				BBMax[j] = Base::Max(m_FrustumPoints[i][j], BBMax[j]);
				BBMin[j] = Base::Min(m_FrustumPoints[i][j], BBMin[j]);
			}
		}
		
		Int3 MaxIndex;
		Int3 MinIndex;

		for (int i = 0; i < 3; ++ i)
		{
			MaxIndex[i] = static_cast<int>(BBMax[i] / m_ReconstructionSettings.m_VolumeSize);
			MinIndex[i] = static_cast<int>(BBMin[i] / m_ReconstructionSettings.m_VolumeSize);
		}

		SRootGrid RootGrid;

		for (int x = MinIndex[0] - 1; x <= MaxIndex[0]; ++ x)
		{
			for (int y = MinIndex[1] - 1; y <= MaxIndex[1]; ++ y)
			{
				for (int z = MinIndex[2] - 1; z <= MaxIndex[2]; ++ z)
				{
					Int3 Key = Int3(x, y, z);
					
					if (m_RootGridMap.count(Key) == 0 && RootGridInFrustum(Key))
					{
						RootGrid.m_Offset = Key;
						RootGrid.m_IsVisible = true;
                        RootGrid.m_TSDFVolumePtr = nullptr;
                        RootGrid.m_ColorVolumePtr = nullptr;

						m_RootGridMap[Key] = RootGrid;
					}
				}
			}
		}

        ////////////////////////////////////////////////////////////////////////////////
        // Prepare instance buffers
        ////////////////////////////////////////////////////////////////////////////////

        ResizeInstanceBuffers(m_RootGridMap.size());
        ClearBuffer(m_AtomicCounterBufferPtr, m_RootGridMap.size());
        
        ////////////////////////////////////////////////////////////////////////////////
        // Create vector and instance buffer for root grid volumes
        ////////////////////////////////////////////////////////////////////////////////

        m_RootGridVector.clear();
        int Index = 0;
        SInstanceData* pInstanceData = static_cast<SInstanceData*>(BufferManager::MapConstantBuffer(m_RootVolumeInstanceBufferPtr, CBuffer::Write));

		for (auto& rPair : m_RootGridMap)
		{
			auto& rRootGrid = rPair.second;

			rRootGrid.m_IsVisible = RootGridInFrustum(rRootGrid.m_Offset);
            
            m_RootGridVector.push_back(&rRootGrid);

            SInstanceData InstanceData;
            InstanceData.m_Index = Index++;
            InstanceData.m_Offset = rRootGrid.m_Offset;

            *pInstanceData = InstanceData;
            ++pInstanceData;
        }

        BufferManager::UnmapConstantBuffer(m_RootVolumeInstanceBufferPtr);

        ////////////////////////////////////////////////////////////////////////////////
        // Check all possible root grid volumes for depth data
        ////////////////////////////////////////////////////////////////////////////////

        RasterizeRootVolumes();
        GatherCounters(static_cast<unsigned int>(m_RootGridMap.size()));

        // todo: try to get rid of mapping
        SIndexedIndirect* pIndirectData = static_cast<SIndexedIndirect*>(BufferManager::MapConstantBuffer(m_IndexedIndirectBufferPtr, CBuffer::ReadWrite));
        int VolumeCount = pIndirectData->m_InstanceCount;
        BufferManager::UnmapConstantBuffer(m_IndexedIndirectBufferPtr);

        ////////////////////////////////////////////////////////////////////////////////
        // Integrate depth into individual root volume grids
        ////////////////////////////////////////////////////////////////////////////////

        if (VolumeCount > 0)
        {
            std::vector<uint32_t> VolumeQueue(VolumeCount);
            uint32_t* pVoxelQueue = static_cast<uint32_t*>(BufferManager::MapConstantBufferRange(m_VolumeQueueBufferPtr, CBuffer::Read, 0, VolumeCount * sizeof(uint32_t)));
            memcpy(VolumeQueue.data(), pVoxelQueue, sizeof(uint32_t) * VolumeCount);
            BufferManager::UnmapConstantBuffer(m_VolumeQueueBufferPtr);

            IntegrateRootGrids(VolumeQueue);
        }

        /*{
            GLint Memory;
            glGetIntegerv(0x9049, &Memory);

            if (Memory < 1000000)
            {
                BASE_CONSOLE_ERROR("Out of GPU memory");
                throw std::exception("Out of GPU memory");
            }

            TextureDescriptor.m_NumberOfPixelsU = m_ReconstructionSettings.m_VolumeResolution;
            TextureDescriptor.m_NumberOfPixelsV = m_ReconstructionSettings.m_VolumeResolution;
            TextureDescriptor.m_NumberOfPixelsW = m_ReconstructionSettings.m_VolumeResolution;
            TextureDescriptor.m_Format = CTextureBase::R16G16_FLOAT;

            RootGrid.m_TSDFVolumePtr = TextureManager::CreateTexture3D(TextureDescriptor);

            if (m_ReconstructionSettings.m_CaptureColor)
            {
                TextureDescriptor.m_Format = CTextureBase::R8G8B8A8_UBYTE;

                RootGrid.m_ColorVolumePtr = TextureManager::CreateTexture3D(TextureDescriptor);
            }
        }*/
	}

	// -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::SetupTextures()
    {
        m_SmoothDepthBufferPtr.resize(m_ReconstructionSettings.m_PyramidLevelCount);
        m_ReferenceVertexMapPtr.resize(m_ReconstructionSettings.m_PyramidLevelCount);
        m_ReferenceNormalMapPtr.resize(m_ReconstructionSettings.m_PyramidLevelCount);
        m_RaycastVertexMapPtr.resize(m_ReconstructionSettings.m_PyramidLevelCount);
        m_RaycastNormalMapPtr.resize(m_ReconstructionSettings.m_PyramidLevelCount);

        STextureDescriptor TextureDescriptor = {};
        
        for (int i = 0; i < m_ReconstructionSettings.m_PyramidLevelCount; ++i)
        {
            TextureDescriptor.m_NumberOfPixelsU = m_pRGBDCameraControl->GetDepthWidth() >> i;
            TextureDescriptor.m_NumberOfPixelsV = m_pRGBDCameraControl->GetDepthHeight() >> i;
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_NumberOfMipMaps = 1;
            TextureDescriptor.m_NumberOfTextures = 1;
            TextureDescriptor.m_Binding = CTextureBase::ShaderResource;
            TextureDescriptor.m_Access = CTextureBase::CPUWrite;
            TextureDescriptor.m_Usage = CTextureBase::GPUReadWrite;
            TextureDescriptor.m_Semantic = CTextureBase::UndefinedSemantic;
            TextureDescriptor.m_pFileName = 0;
            TextureDescriptor.m_pPixels = 0;
            TextureDescriptor.m_Format = CTextureBase::R16_UINT;

            m_SmoothDepthBufferPtr[i] = TextureManager::CreateTexture2D(TextureDescriptor);

            TextureDescriptor.m_Format = g_UseHighPrecisionMaps ? CTextureBase::R32G32B32A32_FLOAT : CTextureBase::R16G16B16A16_FLOAT;

            m_ReferenceVertexMapPtr[i] = TextureManager::CreateTexture2D(TextureDescriptor);
            m_ReferenceNormalMapPtr[i] = TextureManager::CreateTexture2D(TextureDescriptor);
            m_RaycastVertexMapPtr[i] = TextureManager::CreateTexture2D(TextureDescriptor);
            m_RaycastNormalMapPtr[i] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        TextureDescriptor.m_NumberOfPixelsU = m_pRGBDCameraControl->GetDepthWidth();
        TextureDescriptor.m_NumberOfPixelsV = m_pRGBDCameraControl->GetDepthHeight();
        TextureDescriptor.m_NumberOfPixelsW = 1;
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding = CTextureBase::ShaderResource;
        TextureDescriptor.m_Access = CTextureBase::CPUWrite;
        TextureDescriptor.m_Usage = CTextureBase::GPUReadWrite;
        TextureDescriptor.m_Semantic = CTextureBase::UndefinedSemantic;
        TextureDescriptor.m_pFileName = nullptr;
        TextureDescriptor.m_pPixels = 0;
        TextureDescriptor.m_Format = CTextureBase::R16_UINT;

        m_RawDepthBufferPtr = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureDescriptor.m_Format = g_UseHighPrecisionMaps ? CTextureBase::R32G32B32A32_FLOAT : CTextureBase::R16G16B16A16_FLOAT;

        m_RawVertexMapPtr = TextureManager::CreateTexture2D(TextureDescriptor);

		if (m_ReconstructionSettings.m_CaptureColor)
		{
			TextureDescriptor.m_NumberOfPixelsU = m_pRGBDCameraControl->GetDepthWidth();
			TextureDescriptor.m_NumberOfPixelsV = m_pRGBDCameraControl->GetDepthHeight();
			TextureDescriptor.m_Format = CTextureBase::R8G8B8A8_UBYTE;

			m_RawCameraFramePtr = TextureManager::CreateTexture2D(TextureDescriptor);
		}        
    }
    
    // -----------------------------------------------------------------------------
    
    void CScalableSLAMReconstructor::SetupBuffers()
    {
        const float FocalLengthX0 = m_pRGBDCameraControl->GetDepthFocalLengthX();
        const float FocalLengthY0 = m_pRGBDCameraControl->GetDepthFocalLengthY();
        const float FocalPointX0 = m_pRGBDCameraControl->GetDepthFocalPointX();
        const float FocalPointY0 = m_pRGBDCameraControl->GetDepthFocalPointY();
        
        std::vector<SIntrinsics> Intrinsics(m_ReconstructionSettings.m_PyramidLevelCount);

        for (int i = 0; i < m_ReconstructionSettings.m_PyramidLevelCount; ++ i)
        {
            const int PyramidFactor = 1 << i;

            const float FocalLengthX = FocalLengthX0 / PyramidFactor;
            const float FocalLengthY = FocalLengthY0 / PyramidFactor;
            const float FocalPointX = FocalPointX0 / PyramidFactor;
            const float FocalPointY = FocalPointY0 / PyramidFactor;

            Float4x4 KMatrix(
                FocalLengthX, 0.0f, FocalPointX, 0.0f,
                0.0f, FocalLengthY, FocalPointY, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );

            Intrinsics[i].m_FocalPoint = Float2(FocalPointX, FocalPointY);
            Intrinsics[i].m_FocalLength = Float2(FocalLengthX, FocalLengthY);
            Intrinsics[i].m_InvFocalLength = Float2(1.0f / FocalLengthX, 1.0f / FocalLengthY);
            Intrinsics[i].m_KMatrix = Intrinsics[i].m_InvKMatrix = KMatrix;
            Intrinsics[i].m_InvKMatrix.Invert();
        }

        SBufferDescriptor ConstantBufferDesc = {};

        ConstantBufferDesc.m_Stride = 0;
        ConstantBufferDesc.m_Usage = CBuffer::EUsage::GPURead;
        ConstantBufferDesc.m_Binding = CBuffer::ConstantBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SIntrinsics) * m_ReconstructionSettings.m_PyramidLevelCount;
        ConstantBufferDesc.m_pBytes = Intrinsics.data();
        ConstantBufferDesc.m_pClassKey = 0;

        m_IntrinsicsConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        
        STrackingData TrackingData;
        TrackingData.m_PoseMatrix = m_PoseMatrix;
        TrackingData.m_InvPoseMatrix = m_PoseMatrix.GetInverted();

        ConstantBufferDesc.m_NumberOfBytes = sizeof(STrackingData);
        ConstantBufferDesc.m_pBytes        = &TrackingData;
        m_TrackingDataConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_pBytes = 0;
        ConstantBufferDesc.m_NumberOfBytes = 16;
        ConstantBufferDesc.m_Usage = CBuffer::GPUReadWrite;
        m_RaycastPyramidConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        
        ConstantBufferDesc.m_Usage = CBuffer::GPURead;
        ConstantBufferDesc.m_NumberOfBytes = 16;
        m_ICPSummationConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_NumberOfBytes = sizeof(SIncBuffer);
        m_IncPoseMatrixConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_NumberOfBytes = 16;
        ConstantBufferDesc.m_pBytes = &m_ReconstructionSettings.m_DepthThreshold;
        m_BilateralFilterConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

		ConstantBufferDesc.m_NumberOfBytes = sizeof(SPositionBuffer);
		ConstantBufferDesc.m_pBytes = nullptr;
		m_PositionConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        const int ICPRowCount = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() , g_TileSize2D) *
                                GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

        ConstantBufferDesc.m_Usage = CBuffer::GPUToCPU;
        ConstantBufferDesc.m_Binding = CBuffer::ResourceBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPURead;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(float) * ICPRowCount * g_ICPValueCount;
        ConstantBufferDesc.m_pBytes = nullptr;
        m_ICPResourceBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_Usage = CBuffer::GPURead;
        ConstantBufferDesc.m_Binding = CBuffer::ConstantBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(float) * MR::SReconstructionSettings::GRID_LEVELS;
        ConstantBufferDesc.m_pBytes = m_ReconstructionSettings.m_GridResolutions;
        m_HierarchyConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_Usage = CBuffer::GPUToCPU;
        ConstantBufferDesc.m_Binding = CBuffer::ResourceBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPURead;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(uint32_t);
        ConstantBufferDesc.m_pBytes = nullptr;
        m_AtomicCounterBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        m_RootVolumeInstanceBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_Usage = CBuffer::GPURead;
        ConstantBufferDesc.m_Binding = CBuffer::ResourceBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SIndexedIndirect);
        m_IndexedIndirectBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        
        ConstantBufferDesc.m_Binding = CBuffer::ResourceBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = 16;
        ConstantBufferDesc.m_pBytes = nullptr;
        ConstantBufferDesc.m_Usage = CBuffer::GPURead;
        m_VolumeQueueBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_Binding = CBuffer::ConstantBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SGridRasterization);
        ConstantBufferDesc.m_pBytes = nullptr;
        ConstantBufferDesc.m_Usage = CBuffer::GPURead;
        m_GridRasterizationBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_Binding = CBuffer::ResourceBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = 4096 * sizeof(uint32_t); // todo: remove magic number
        ConstantBufferDesc.m_pBytes = nullptr;
        ConstantBufferDesc.m_Usage = CBuffer::GPURead;
        m_GridAtomicCounterBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        m_GridQueueBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::Update()
    {
        const bool CaptureColor = m_ReconstructionSettings.m_CaptureColor;

        unsigned short* pDepth = m_DepthPixels.data();
        Base::Byte4* pColor = m_CameraPixels.data();

        if (m_IsTrackingPaused)
        {
            return;
        }

        if (!m_pRGBDCameraControl->GetDepthBuffer(pDepth))
        {
            return;
        }

        if (CaptureColor && !m_pRGBDCameraControl->GetCameraFrame(pColor))
        {
            return;
        }

        Performance::BeginEvent("Kinect Fusion");

        Performance::BeginEvent("Data Input");

        Base::AABB2UInt TargetRect;
        TargetRect = Base::AABB2UInt(Base::UInt2(0, 0), Base::UInt2(m_pRGBDCameraControl->GetDepthWidth(), m_pRGBDCameraControl->GetDepthHeight()));
        TextureManager::CopyToTexture2D(m_RawDepthBufferPtr, TargetRect, m_pRGBDCameraControl->GetDepthWidth(), pDepth);

        if (CaptureColor)
        {
            TargetRect = Base::AABB2UInt(Base::UInt2(0, 0), Base::UInt2(m_pRGBDCameraControl->GetDepthWidth(), m_pRGBDCameraControl->GetDepthHeight()));
            TextureManager::CopyToTexture2D(m_RawCameraFramePtr, TargetRect, m_pRGBDCameraControl->GetDepthWidth(), pColor);
        }

        //////////////////////////////////////////////////////////////////////////////////////
        // Create reference data
        //////////////////////////////////////////////////////////////////////////////////////

        CreateReferencePyramid();

        Performance::EndEvent();

        //////////////////////////////////////////////////////////////////////////////////////
        // Tracking
        //////////////////////////////////////////////////////////////////////////////////////

        if (m_IntegratedFrameCount > 0)
        {
            Performance::BeginEvent("Tracking");

            PerformTracking();

            STrackingData TrackingData;
            TrackingData.m_PoseMatrix = m_PoseMatrix;
            TrackingData.m_InvPoseMatrix = m_PoseMatrix.GetInverted();

            BufferManager::UploadConstantBufferData(m_TrackingDataConstantBufferPtr, &TrackingData);

            Performance::EndEvent();
        }

        //////////////////////////////////////////////////////////////////////////////////////
        // Integrate and raycast pyramid
        //////////////////////////////////////////////////////////////////////////////////////

		Performance::BeginEvent("Updating root grid");

		UpdateFrustum();
		UpdateRootrids();

		Performance::EndEvent();

        Performance::BeginEvent("TSDF Integration and Raycasting");

        if (!m_IsIntegrationPaused)
        {
            Performance::BeginEvent("Old integration");
            IntegrateOld();
            Performance::EndEvent();
        }

        Raycast();
        CreateRaycastPyramid();

        Performance::EndEvent();

        ++m_IntegratedFrameCount;
        ++m_FrameCount;

        ContextManager::ResetShaderCS();
        ContextManager::ResetImageTexture(0);
        ContextManager::ResetImageTexture(1);
        ContextManager::ResetImageTexture(2);
        ContextManager::ResetImageTexture(3);
        ContextManager::ResetTexture(0);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::CreateReferencePyramid()
    {
        const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth(), g_TileSize2D);
        const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

        //////////////////////////////////////////////////////////////////////////////////////
        // Bilateral Filter
        //////////////////////////////////////////////////////////////////////////////////////

        ContextManager::Barrier();

        ContextManager::SetShaderCS(m_BilateralFilterCSPtr);
        ContextManager::SetConstantBuffer(0, m_BilateralFilterConstantBufferPtr);
        ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_RawDepthBufferPtr));
        ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_SmoothDepthBufferPtr[0]));
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        //////////////////////////////////////////////////////////////////////////////////////
        // Downsample depth buffer
        //////////////////////////////////////////////////////////////////////////////////////

        for (int PyramidLevel = 1; PyramidLevel < m_ReconstructionSettings.m_PyramidLevelCount; ++ PyramidLevel)
        {
            const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);
            
            ContextManager::SetShaderCS(m_DownSampleDepthCSPtr);

            ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_SmoothDepthBufferPtr[PyramidLevel - 1]));
            ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_SmoothDepthBufferPtr[PyramidLevel]));
            ContextManager::Barrier();

            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
        }

        /////////////////////////////////////////////////////////////////////////////////////
        // Generate vertex map pyramid
        /////////////////////////////////////////////////////////////////////////////////////

        ContextManager::SetConstantBuffer(0, m_IntrinsicsConstantBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);

        ContextManager::SetShaderCS(m_VertexMapCSPtr);
        for (int PyramidLevel = 0; PyramidLevel < m_ReconstructionSettings.m_PyramidLevelCount; ++ PyramidLevel)
        {
            const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);

            ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_SmoothDepthBufferPtr[PyramidLevel]));
            ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_ReferenceVertexMapPtr[PyramidLevel]));
            ContextManager::Barrier();
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
        }

        /////////////////////////////////////////////////////////////////////////////////////
        // Generate raw vertex map
        /////////////////////////////////////////////////////////////////////////////////////
        
        ContextManager::SetShaderCS(m_VertexMapCSPtr);

        ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_RawDepthBufferPtr));
        ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_RawVertexMapPtr));
        ContextManager::Barrier();
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        /////////////////////////////////////////////////////////////////////////////////////
        // Generate normal map pyramid
        /////////////////////////////////////////////////////////////////////////////////////

        ContextManager::SetShaderCS(m_NormalMapCSPtr);
        for (int PyramidLevel = 0; PyramidLevel < m_ReconstructionSettings.m_PyramidLevelCount; ++ PyramidLevel)
        {
            const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);
                        
            ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_ReferenceVertexMapPtr[PyramidLevel]));
            ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_ReferenceNormalMapPtr[PyramidLevel]));
            ContextManager::Barrier();
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
        }
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::PerformTracking()
    {
        Float4x4 IncPoseMatrix = m_PoseMatrix;

        for (int PyramidLevel = m_ReconstructionSettings.m_PyramidLevelCount - 1; PyramidLevel >= 0; -- PyramidLevel)
        {
            for (int Iteration = 0; Iteration < m_ReconstructionSettings.m_PyramidLevelIterations[PyramidLevel]; ++ Iteration)
            {
                DetermineSummands(PyramidLevel, IncPoseMatrix);
                ReduceSum(PyramidLevel);

                m_TrackingLost = !CalculatePoseMatrix(IncPoseMatrix);
                if (m_TrackingLost)
                {
                    return;
                }
            }
        }
        m_PoseMatrix = IncPoseMatrix;
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::DetermineSummands(int PyramidLevel, const Float4x4& rIncPoseMatrix)
    {
        const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
        const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);
        
        SIncBuffer TrackingData;
        TrackingData.m_PoseMatrix = rIncPoseMatrix;
        TrackingData.m_InvPoseMatrix = rIncPoseMatrix.GetInverted();
        TrackingData.m_PyramidLevel = PyramidLevel;
        
        BufferManager::UploadConstantBufferData(m_IncPoseMatrixConstantBufferPtr, &TrackingData);

        ContextManager::SetShaderCS(m_DetermineSummandsCSPtr);
        ContextManager::SetResourceBuffer(0, m_ICPResourceBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);
        ContextManager::SetConstantBuffer(2, m_IncPoseMatrixConstantBufferPtr);
        
        ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_ReferenceVertexMapPtr[PyramidLevel]));
        ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_ReferenceNormalMapPtr[PyramidLevel]));
        ContextManager::SetImageTexture(2, static_cast<CTextureBasePtr>(m_RaycastVertexMapPtr[PyramidLevel]));
        ContextManager::SetImageTexture(3, static_cast<CTextureBasePtr>(m_RaycastNormalMapPtr[PyramidLevel]));

        ContextManager::Barrier();

        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::ReduceSum(int PyramidLevel)
    {
        const int SummandsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
        const int SummandsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);

        const int Summands = SummandsX * SummandsY;
        const float SummandsLog2 = Log2(static_cast<float>(Summands));
        const int SummandsPOT = 1 << (static_cast<int>(SummandsLog2) + 1);
        
        Base::Int2 BufferData;
        BufferData[0] = Summands / 2;
        BufferData[1] = SummandsPOT / 2;

        BufferManager::UploadConstantBufferData(m_ICPSummationConstantBufferPtr, &BufferData);

        ContextManager::SetShaderCS(m_ReduceSumCSPtr);
        ContextManager::SetResourceBuffer(0, m_ICPResourceBufferPtr);
        ContextManager::SetConstantBuffer(2, m_ICPSummationConstantBufferPtr);
        
        ContextManager::Barrier();

        ContextManager::Dispatch(1, g_ICPValueCount, 1);
    }

    // -----------------------------------------------------------------------------

    bool CScalableSLAMReconstructor::CalculatePoseMatrix(Float4x4& rIncPoseMatrix)
    {
        typedef double Scalar;

        Scalar A[36];
        Scalar b[6];
        
        float ICPValues[g_ICPValueCount];
        void* pICPBuffer = BufferManager::MapConstantBufferRange(m_ICPResourceBufferPtr, CBuffer::EMap::Read, 0, sizeof(float) * g_ICPValueCount);
        memcpy(ICPValues, pICPBuffer, sizeof(ICPValues[0]) * g_ICPValueCount);
        BufferManager::UnmapConstantBuffer(m_ICPResourceBufferPtr);

        int ValueIndex = 0;
        for (int i = 0; i < 6; ++ i)
        {
            for (int j = i; j < 7; ++ j)
            {
                float Value = ICPValues[ValueIndex++];
                
                if (j == 6)
                {
                    b[i] = static_cast<Scalar>(Value);
                }
                else
                {
                    A[j * 6 + i] = A[i * 6 + j] = static_cast<Scalar>(Value);
                }
            }
        }

        Scalar L[36];

        for (int i = 0; i < 6; ++ i)
        {
            for (int j = 0; j <= i; ++ j)
            {
                Scalar Sum = 0.0;
                for (int k = 0; k < j; ++ k)
                {
                    Sum += L[k * 6 + i] * L[k * 6 + j];
                }
                L[j * 6 + i] = i == j ? sqrt(A[i * 6 + i] - Sum) : ((1.0f / L[j * 6 + j]) * (A[j * 6 + i] - Sum));
            }
        }

        const Scalar Det = L[0] * L[0] * L[7] * L[7] * L[14] * L[14] * L[21] * L[21] * L[28] * L[28] * L[35] * L[35];
        
        if (std::isnan(Det) || abs(Det) < 1e-5)
        {
            return false;
        }

        Scalar y[6];
        
        y[0] = b[0] / L[0];
        y[1] = (b[1] - L[1] * y[0]) / L[7];
        y[2] = (b[2] - L[2] * y[0] - L[8] * y[1]) / L[14];
        y[3] = (b[3] - L[3] * y[0] - L[9] * y[1] - L[15] * y[2]) / L[21];
        y[4] = (b[4] - L[4] * y[0] - L[10] * y[1] - L[16] * y[2] - L[22] * y[3]) / L[28];
        y[5] = (b[5] - L[5] * y[0] - L[11] * y[1] - L[17] * y[2] - L[23] * y[3] - L[29] * y[4]) / L[35];

        Scalar x[6];

        x[5] = y[5] / L[35];
        x[4] = (y[4] - L[29] * x[5]) / L[28];
        x[3] = (y[3] - L[23] * x[5] - L[22] * x[4]) / L[21];
        x[2] = (y[2] - L[17] * x[5] - L[16] * x[4] - L[15] * x[3]) / L[14];
        x[1] = (y[1] - L[11] * x[5] - L[10] * x[4] - L[9] * x[3] - L[8] * x[2]) / L[7];
        x[0] = (y[0] - L[5] * x[5] - L[4] * x[4] - L[3] * x[3] - L[2] * x[2] - L[1] * x[1]) / L[0];
        
        Float4x4 RotationX, RotationY, RotationZ, Rotation, Translation;
        RotationX.SetRotationX(static_cast<float>(x[0]));
        RotationY.SetRotationY(static_cast<float>(x[1]));
        RotationZ.SetRotationZ(static_cast<float>(x[2]));
        Rotation = RotationZ * RotationY * RotationX;
        Translation.SetTranslation(static_cast<float>(x[3]), static_cast<float>(x[4]), static_cast<float>(x[5]));
        
        rIncPoseMatrix = Translation * Rotation * rIncPoseMatrix;

        return true;
    }    

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::IntegrateOld()
    {
        const int WorkGroups = GetWorkGroupCount(m_ReconstructionSettings.m_VolumeResolution, g_TileSize2D);

        ContextManager::SetShaderCS(m_IntegrationCSPtr);

        ContextManager::SetConstantBuffer(0, m_IntrinsicsConstantBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);        
		ContextManager::SetConstantBuffer(2, m_PositionConstantBufferPtr);

        ContextManager::Barrier();

		ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_RawDepthBufferPtr));

		if (m_ReconstructionSettings.m_CaptureColor)
		{
			ContextManager::SetImageTexture(3, static_cast<CTextureBasePtr>(m_RawCameraFramePtr));
		}
		else
		{
			ContextManager::ResetImageTexture(3);
		}

		for (auto& rEntry : m_RootGridMap)
		{
			SRootGrid& rRootGrid = rEntry.second;

			if (rRootGrid.m_IsVisible)
			{
				Float3 Position;

				Position[0] = rRootGrid.m_Offset[0] * m_ReconstructionSettings.m_VolumeSize;
				Position[1] = rRootGrid.m_Offset[1] * m_ReconstructionSettings.m_VolumeSize;
				Position[2] = rRootGrid.m_Offset[2] * m_ReconstructionSettings.m_VolumeSize;

				BufferManager::UploadConstantBufferData(m_PositionConstantBufferPtr, &Position);

				ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(rRootGrid.m_TSDFVolumePtr));

				if (m_ReconstructionSettings.m_CaptureColor)
				{
					ContextManager::SetImageTexture(2, static_cast<CTextureBasePtr>(rRootGrid.m_ColorVolumePtr));
				}

				ContextManager::Dispatch(WorkGroups, WorkGroups, 1);
			}
		}
		ContextManager::ResetImageTexture(0);
		ContextManager::ResetImageTexture(1);
		ContextManager::ResetImageTexture(2);
		ContextManager::ResetImageTexture(3);
    }
    
    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::CreateRaycastPyramid()
    {
        ContextManager::SetShaderCS(m_RaycastPyramidCSPtr);

        ContextManager::SetConstantBuffer(0, m_RaycastPyramidConstantBufferPtr);
        
        for (int PyramidLevel = 1; PyramidLevel < m_ReconstructionSettings.m_PyramidLevelCount; ++PyramidLevel)
        {
            const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);

            float Normalized = 0.0f;
            BufferManager::UploadConstantBufferData(m_RaycastPyramidConstantBufferPtr, &Normalized);
            
            ContextManager::Barrier();
            ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_RaycastVertexMapPtr[PyramidLevel - 1]));
            ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_RaycastVertexMapPtr[PyramidLevel]));
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

            Normalized = 1.0f;
            BufferManager::UploadConstantBufferData(m_RaycastPyramidConstantBufferPtr, &Normalized);

            ContextManager::Barrier();
            ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_RaycastNormalMapPtr[PyramidLevel - 1]));
            ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_RaycastNormalMapPtr[PyramidLevel]));
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
        }
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::Raycast()
    {
        const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth(), g_TileSize2D);
        const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

        ContextManager::SetShaderCS(m_RaycastCSPtr);

		ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(m_RootGridMap[Int3(0, 0, 0)].m_TSDFVolumePtr));
        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::ESampler::MinMagMipLinearClamp));

        ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_RaycastVertexMapPtr[0]));
        ContextManager::SetImageTexture(2, static_cast<CTextureBasePtr>(m_RaycastNormalMapPtr[0]));

        ContextManager::SetConstantBuffer(0, m_IntrinsicsConstantBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);
        
        ContextManager::Barrier();

        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::ResizeInstanceBuffers(size_t Size)
    {
        if (Size > m_AtomicCounterBufferPtr->GetNumberOfBytes() / sizeof(uint32_t))
        {
            SBufferDescriptor BufferDesc = {};

            BufferDesc.m_Usage = CBuffer::GPUToCPU;
            BufferDesc.m_Binding = CBuffer::ResourceBuffer;
            BufferDesc.m_Access = CBuffer::CPURead;
            BufferDesc.m_NumberOfBytes = static_cast<unsigned int>(sizeof(uint32_t) * Size);
            BufferDesc.m_pBytes = nullptr;
            m_AtomicCounterBufferPtr = BufferManager::CreateBuffer(BufferDesc);
            
            BufferDesc.m_Usage = CBuffer::GPURead;
            BufferDesc.m_Binding = CBuffer::ResourceBuffer;
            BufferDesc.m_Access = CBuffer::CPUWrite;
            BufferDesc.m_NumberOfBytes = static_cast<unsigned int>(sizeof(SInstanceData) * Size);
            m_RootVolumeInstanceBufferPtr = BufferManager::CreateBuffer(BufferDesc);

            BufferDesc.m_NumberOfBytes = static_cast<unsigned int>(sizeof(uint32_t) * Size);
            m_VolumeQueueBufferPtr = BufferManager::CreateBuffer(BufferDesc);
        }
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::ClearBuffer(CBufferPtr BufferPtr, size_t Size)
    {
        const int WorkGroups = static_cast<int>(Size);

        ContextManager::SetShaderCS(m_ClearAtomicCountersCSPtr);
        
        ContextManager::SetResourceBuffer(0, BufferPtr);
        
        ContextManager::Barrier();

        ContextManager::Dispatch(WorkGroups, 1, 1);
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::ResetReconstruction(const SReconstructionSettings* pReconstructionSettings)
    {
		m_RootGridMap.clear();

        if (pReconstructionSettings != nullptr)
        {
			assert(pReconstructionSettings->m_IsScalable);
            m_ReconstructionSettings = *pReconstructionSettings;
        }

		SetupData();

		SetupTextures();
		SetupBuffers();
		SetupShaders();
    }

    // -----------------------------------------------------------------------------
    
    void CScalableSLAMReconstructor::PauseIntegration(bool _Paused)
    {
        m_IsIntegrationPaused = _Paused;
    }
    
    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::PauseTracking(bool _Paused)
    {
        m_IsTrackingPaused = _Paused;
    }
        
    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::GetReconstructionSettings(SReconstructionSettings* pReconstructionSettings)
    {
        assert(pReconstructionSettings != nullptr);
        *pReconstructionSettings = m_ReconstructionSettings;
    }

    // -----------------------------------------------------------------------------

    int CScalableSLAMReconstructor::GetWorkGroupCount(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }
} // namespace MR
