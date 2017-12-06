
#include "mr/mr_precompiled.h"

#include "base/base_program_parameters.h"
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

#include "graphic/gfx_native_buffer.h"

using namespace MR;
using namespace Gfx;

namespace
{
	//*
	const Base::Float3 g_InitialCameraPosition = Base::Float3(0.0f, 0.0f, 0.0f);
	const Base::Float3 g_InitialCameraRotation = Base::Float3(3.14f, 0.0f, 0.0f);
	/*/
	const Base::Float3 g_InitialCameraPosition = Base::Float3(0.5f, 0.5f, -0.5f);
	const Base::Float3 g_InitialCameraRotation = Base::Float3(0.0f, 0.0f, 0.0f);
	//*/

    const unsigned int g_MegabyteSize = 1024u * 1024u;

    const unsigned int g_AABB = 8;

    //*
    const unsigned int g_RootVolumePoolSize =        g_MegabyteSize;
    const unsigned int g_RootGridPoolSize   =  16u * g_MegabyteSize;
    const unsigned int g_Level1GridPoolSize =  64u * g_MegabyteSize;
    const unsigned int g_TSDFPoolSize       = 512u * g_MegabyteSize;
    /*/
    const unsigned int g_RootVolumePoolSize =              g_MegabyteSize;
    const unsigned int g_RootGridPoolSize   =       128u * g_MegabyteSize;
    const unsigned int g_Level1GridPoolSize =       128u * g_MegabyteSize;
    const unsigned int g_TSDFPoolSize       = 16u * 128u * g_MegabyteSize;
    //*/

    const bool g_UseFullVolumeIntegration = true;
    const bool g_UseReverseIntegration = true;

    const bool g_UseHighPrecisionMaps = false;
    
    const float g_EpsilonDistance = 0.1f;
    const float g_EpsilonAngle = 0.75f;
    
    const int g_ICPValueCount = 27;

    const unsigned int g_TileSize1D = 64;
    const unsigned int g_TileSize2D = 16;
    const unsigned int g_TileSize3D = 8;

    Base::Float3 CubeVertices[] =
    {
        Base::Float3(0.0f, 0.0f, 0.0f),
        Base::Float3(1.0f, 0.0f, 0.0f),
        Base::Float3(1.0f, 1.0f, 0.0f),
        Base::Float3(0.0f, 1.0f, 0.0f),
        Base::Float3(0.0f, 0.0f, 1.0f),
        Base::Float3(1.0f, 0.0f, 1.0f),
        Base::Float3(1.0f, 1.0f, 1.0f),
        Base::Float3(0.0f, 1.0f, 1.0f),
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

    struct SGridRasterization
    {
        Base::Int3 m_Offset;
        int32_t m_Resolution;
        float m_CubeSize;
        float m_ParentSize;
        float Padding[2];
    };

    struct SPointRasterization
    {
        Base::Int3 m_Offset;
        int32_t m_Resolution;
    };

    struct SVolumePoolItem
    {
        Base::Int3 m_Offset;
        bool m_NearSurface;
    };

    struct SGridPoolItem
    {
        int m_PoolIndex;
        bool m_NearSurface;
    };

    struct STSDFPoolItem
    {
        uint16_t m_TSDF;
        uint16_t m_Weight;
    };

} // namespace

namespace MR
{
    using namespace Base;
    
    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::Start()
    {
        ////////////////////////////////////////////////////////////////////////////////
        // Check if conservative rasterization is available
        ////////////////////////////////////////////////////////////////////////////////

        const bool EnableConservativeRaster = Base::CProgramParameters::GetInstance().GetBoolean("conservative_raster_enable");

        m_UseConservativeRasterization = false;

        if (EnableConservativeRaster)
        {
            m_UseConservativeRasterization = Main::IsExtensionAvailable("GL_NV_conservative_raster");

            if (!m_UseConservativeRasterization)
            {
                BASE_CONSOLE_INFO("Conservative rasterization is not available. Will use fallback method");
            }
        }

        m_UseShuffleIntrinsics = false;

        const bool EnableShuffleIntrinsics = Base::CProgramParameters::GetInstance().GetBoolean("tracking_with_shuffle_intrinsics");

        if (EnableShuffleIntrinsics)
        {
            m_UseShuffleIntrinsics = Main::IsExtensionAvailable("GL_NV_shader_thread_shuffle");

            if (!m_UseShuffleIntrinsics)
            {
                BASE_CONSOLE_INFO("Shuffle intrinsics are not available. Will use fallback method");
            }
        }

        ////////////////////////////////////////////////////////////////////////////////
        // Setup data, buffer etc.
        ////////////////////////////////////////////////////////////////////////////////

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

        ClearPool();

        m_IsIntegrationPaused = false;
        m_IsTrackingPaused = false;
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::SetupRenderStates()
    {
        SViewPortDescriptor ViewPortDescriptor = {};

        ViewPortDescriptor.m_MinDepth = 0.0f;
        ViewPortDescriptor.m_MaxDepth = 1.0f;
        ViewPortDescriptor.m_TopLeftX = 0.0f;
        ViewPortDescriptor.m_TopLeftY = 0.0f;
        ViewPortDescriptor.m_Width = static_cast<float>(m_pRGBDCameraControl->GetDepthWidth());
        ViewPortDescriptor.m_Height = static_cast<float>(m_pRGBDCameraControl->GetDepthHeight());

        Gfx::CViewPortPtr DepthViewPort = ViewManager::CreateViewPort(ViewPortDescriptor);

        m_DepthViewPortSetPtr = ViewManager::CreateViewPortSet(DepthViewPort);

        ViewPortDescriptor.m_Width = static_cast<float>(m_ReconstructionSettings.m_GridResolutions[0]);
        ViewPortDescriptor.m_Height = static_cast<float>(m_ReconstructionSettings.m_GridResolutions[0]);

        Gfx::CViewPortPtr RootGridViewPort = ViewManager::CreateViewPort(ViewPortDescriptor);

        m_RootGridViewPort = ViewManager::CreateViewPortSet(RootGridViewPort);

        ViewPortDescriptor.m_Width = 128.0f;
        ViewPortDescriptor.m_Height = 128.0f;

        Gfx::CViewPortPtr FullVolumeViewPort = ViewManager::CreateViewPort(ViewPortDescriptor);

        m_FullVolumeViewPort = ViewManager::CreateViewPortSet(FullVolumeViewPort);
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::SetupMeshes()
    {
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
        
        m_Grid8MeshPtr = CreateGridMesh(8);
        m_Grid16MeshPtr = CreateGridMesh(16);
    }

    // -----------------------------------------------------------------------------

    Gfx::CMeshPtr CScalableSLAMReconstructor::CreateGridMesh(int Width)
    {
        std::vector<Float3> Vertices;
        std::vector<unsigned int> Indices;

        for (int z = 0; z < Width; ++ z)
        {
            for (int y = 0; y < Width; ++ y)
            {
                for (int x = 0; x < Width; ++ x)
                {
                    for (int i = 0; i < 8; ++ i)
                    {
                        Float3 Vertex = CubeVertices[i];

                        Vertex[0] += x;
                        Vertex[1] += y;
                        Vertex[2] += z;

                        Vertices.push_back(Vertex);
                    }
                }
            }
        }

        for (int i = 0; i < Width * Width * Width; ++ i)
        {
            for (int j = 0; j < 36; ++ j)
            {
                Indices.push_back(CubeIndices[j] + i * 8);
            }
        }

        Dt::CSurface* pSurface = new Dt::CSurface;
        Dt::CLOD* pLOD = new Dt::CLOD;
        Dt::CMesh* pMesh = new Dt::CMesh;

        pSurface->SetPositions(Vertices.data());
        pSurface->SetNumberOfVertices(static_cast<unsigned int>(Vertices.size()));
        pSurface->SetIndices(Indices.data());
        pSurface->SetNumberOfIndices(static_cast<unsigned int>(Indices.size()));
        pSurface->SetElements(0);

        pLOD->SetSurface(0, pSurface);
        pLOD->SetNumberOfSurfaces(1);

        pMesh->SetLOD(0, pLOD);
        pMesh->SetNumberOfLODs(1);

        SMeshDescriptor MeshDesc = {};
        MeshDesc.m_pMesh = pMesh;

        return MeshManager::CreateMesh(MeshDesc);
    }

	// -----------------------------------------------------------------------------

	void CScalableSLAMReconstructor::SetupData()
	{
        m_ReconstructionSettings.m_UseFullVolumeIntegration = g_UseFullVolumeIntegration;
        m_ReconstructionSettings.m_UseReverseIntegration = g_UseReverseIntegration;

        const int GridLevelCount = MR::SReconstructionSettings::GRID_LEVELS;

        m_VolumeSizes.resize(GridLevelCount);
        m_VolumeSizes[GridLevelCount - 1] = m_ReconstructionSettings.m_VoxelSize * m_ReconstructionSettings.m_GridResolutions[GridLevelCount - 1];
        for (int i = GridLevelCount - 2; i >= 0; --i)
        {
            m_VolumeSizes[i] = m_VolumeSizes[i + 1] * m_ReconstructionSettings.m_GridResolutions[i];
        }

		const float VolumeSize = m_VolumeSizes[0];
		Float4x4 PoseRotation, PoseTranslation;
		
		PoseRotation.SetRotation(g_InitialCameraRotation[0], g_InitialCameraRotation[1], g_InitialCameraRotation[2]);
		PoseTranslation.SetTranslation
		(
			g_InitialCameraPosition[0] * VolumeSize,
			g_InitialCameraPosition[1] * VolumeSize,
			g_InitialCameraPosition[2] * VolumeSize
		);
		m_PoseMatrix = PoseTranslation * PoseRotation;
        
        m_RootVolumePoolItemCount = 0;
		m_IntegratedFrameCount = 0;
		m_FrameCount = 0;
		m_TrackingLost = true;

        m_VolumeBuffers.m_RootVolumeTotalWidth = g_AABB;

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
        m_IntegrateTSDFCSPtr = 0;
        m_RaycastCSPtr = 0;
        m_RaycastPyramidCSPtr = 0;
        m_DetermineSummandsCSPtr = 0;
        m_ReduceSumCSPtr = 0;
        m_ClearVolumeCSPtr = 0;
		m_RootgridDepthCSPtr = 0;
        m_VolumeCountersCSPtr = 0;

        m_RasterizeRootGridVSPtr = 0;
        m_RasterizeRootGridFSPtr = 0;
        m_RasterizeLevel1GridVSPtr = 0;
        m_RasterizeLevel1GridFSPtr = 0;

        m_RasterizeRootVolumeVSPtr = 0;
        m_RasterizeRootVolumeFSPtr = 0;

        m_RasterizeRootGridVSPtr = 0;
        m_RasterizeRootGridFSPtr = 0;

        m_RasterizeLevel1GridVSPtr = 0;
        m_RasterizeLevel1GridFSPtr = 0;

        m_ClearAtomicCountersCSPtr = 0;

        m_IntegrateRootGridCSPtr = 0;
        m_IntegrateLevel1GridCSPtr = 0;

        m_CubeMeshPtr = 0;
        m_Grid8MeshPtr = 0;
        m_Grid16MeshPtr = 0;
        m_CubeInputLayoutPtr = 0;

        m_RawVertexMapPtr = 0;
        m_RawDepthBufferPtr = 0;
        m_RawCameraFramePtr = 0;
        m_EmptyTargetSetPtr = 0;
        m_DepthViewPortSetPtr = 0;

        for (int i = 0; i < m_ReconstructionSettings.m_PyramidLevelCount; ++ i)
        {
            m_SmoothDepthBufferPtr[i] = 0;
            m_ReferenceVertexMapPtr[i] = 0;
            m_ReferenceNormalMapPtr[i] = 0;
            m_RaycastVertexMapPtr[i] = 0;
            m_RaycastNormalMapPtr[i] = 0;
        }

        m_RootVolumeMap.clear();

        m_RootGridVolumePtr = 0;

        m_IntrinsicsConstantBufferPtr = 0;
        m_TrackingDataConstantBufferPtr = 0;
        m_RaycastPyramidConstantBufferPtr = 0;
        m_ICPResourceBufferPtr = 0;
        m_ICPSummationConstantBufferPtr = 0;
        m_IncPoseMatrixConstantBufferPtr = 0;
        m_BilateralFilterConstantBufferPtr = 0;
		m_PositionConstantBufferPtr = 0;
        m_HierarchyConstantBufferPtr = 0;
        m_AtomicCounterBufferPtr = 0;
        m_IndexedIndirectBufferPtr = 0;
        m_GridRasterizationBufferPtr = 0;
        m_VolumeQueueBufferPtr = 0;
        m_RootVolumeInstanceBufferPtr = 0;
        m_VolumeBuffers.m_RootVolumePoolPtr = 0;
        m_VolumeBuffers.m_RootGridPoolPtr = 0;
        m_VolumeBuffers.m_Level1PoolPtr = 0;
        m_VolumeBuffers.m_TSDFPoolPtr = 0;
        m_VolumeBuffers.m_PoolItemCountBufferPtr = 0;
        m_VolumeIndexBufferPtr = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CScalableSLAMReconstructor::SetupShaders()
    {
        const int SummandsX = DivUp(m_pRGBDCameraControl->GetDepthWidth(), g_TileSize2D);
        const int SummandsY = DivUp(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

        const int Summands = SummandsX * SummandsY;
        const float SummandsLog2 = Log2(static_cast<float>(Summands));
        const int SummandsPOT = 1 << (static_cast<int>(SummandsLog2) + 1);
        
        const float VoxelSize = m_ReconstructionSettings.m_VoxelSize;

        const std::string InternalFormatString = g_UseHighPrecisionMaps ? "rgba32f" : "rgba16f";

        std::stringstream DefineStream;

        DefineStream
            << "#define PYRAMID_LEVELS "         << m_ReconstructionSettings.m_PyramidLevelCount    << " \n"
            << "#define VOLUME_RESOLUTION "      << m_ReconstructionSettings.m_GridResolutions[0]   << " \n"
            << "#define VOXEL_SIZE "             << VoxelSize                                       << " \n"
            << "#define VOLUME_SIZE "            << m_VolumeSizes[0]                                << " \n"
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
            << "#define HIERARCHY_LEVELS "       << MR::SReconstructionSettings::GRID_LEVELS        << " \n"
            << "#define VOXELS_PER_ROOTGRID "    << m_ReconstructionSettings.m_VoxelsPerGrid[0]     << " \n"
            << "#define VOXELS_PER_LEVEL1GRID "  << m_ReconstructionSettings.m_VoxelsPerGrid[1]     << " \n"
            << "#define VOXELS_PER_LEVEL2GRID "  << m_ReconstructionSettings.m_VoxelsPerGrid[2]     << " \n";

        if (m_ReconstructionSettings.m_CaptureColor)
        {
            DefineStream << "#define CAPTURE_COLOR\n";
        }
        if (m_UseConservativeRasterization)
        {
            DefineStream << "#define CONSERVATIVE_RASTERIZATION_AVAILABLE\n";
        }
        if (m_UseShuffleIntrinsics)
        {
            DefineStream << "#define USE_SHUFFLE_INTRINSICS\n";
        }

        std::string DefineString = DefineStream.str();
        
        m_BilateralFilterCSPtr     = ShaderManager::CompileCS("scalable_kinect_fusion\\pyramid_creation\\cs_bilateral_filter.glsl"  , "main", DefineString.c_str());
        m_VertexMapCSPtr           = ShaderManager::CompileCS("scalable_kinect_fusion\\pyramid_creation\\cs_vertex_map.glsl"        , "main", DefineString.c_str());
        m_NormalMapCSPtr           = ShaderManager::CompileCS("scalable_kinect_fusion\\pyramid_creation\\cs_normal_map.glsl"        , "main", DefineString.c_str());
        m_DownSampleDepthCSPtr     = ShaderManager::CompileCS("scalable_kinect_fusion\\pyramid_creation\\cs_downsample_depth.glsl"  , "main", DefineString.c_str());
        m_RaycastPyramidCSPtr      = ShaderManager::CompileCS("scalable_kinect_fusion\\pyramid_creation\\cs_raycast_pyramid.glsl"   , "main", DefineString.c_str());
        m_RaycastCSPtr             = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_raycast.glsl"                             , "main", DefineString.c_str());
        m_DetermineSummandsCSPtr   = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_determine_summands.glsl"                  , "main", DefineString.c_str());
        m_ReduceSumCSPtr           = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_reduce_sum.glsl"                          , "main", DefineString.c_str());
        m_ClearVolumeCSPtr         = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_clear_volume.glsl"                        , "main", DefineString.c_str());
		m_RootgridDepthCSPtr       = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_rootgrid_depth.glsl"                      , "main", DefineString.c_str());
        m_ClearAtomicCountersCSPtr = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_clear_atomic_buffer.glsl"                 , "main", DefineString.c_str());
        m_VolumeCountersCSPtr      = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_volume_counters.glsl"                     , "main", DefineString.c_str());
        m_GridCountersCSPtr        = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_grid_counters.glsl"                       , "main", DefineString.c_str());
        m_RasterizeRootVolumeVSPtr = ShaderManager::CompileVS("scalable_kinect_fusion\\rasterization\\vs_rasterize_rootvolume.glsl" , "main", DefineString.c_str());
        m_RasterizeRootVolumeFSPtr = ShaderManager::CompilePS("scalable_kinect_fusion\\rasterization\\fs_rasterize_rootvolume.glsl" , "main", DefineString.c_str());
        m_RasterizeRootGridVSPtr   = ShaderManager::CompileVS("scalable_kinect_fusion\\rasterization\\vs_rasterize_grid.glsl"       , "main", DefineString.c_str());
        m_RasterizeRootGridFSPtr   = ShaderManager::CompilePS("scalable_kinect_fusion\\rasterization\\fs_rasterize_grid.glsl"       , "main", DefineString.c_str());
        m_RasterizeLevel1GridVSPtr = ShaderManager::CompileVS("scalable_kinect_fusion\\rasterization\\vs_rasterize_level1grid.glsl" , "main", DefineString.c_str());
        m_RasterizeLevel1GridFSPtr = ShaderManager::CompilePS("scalable_kinect_fusion\\rasterization\\fs_rasterize_level1grid.glsl" , "main", DefineString.c_str());
        m_IntegrateRootGridCSPtr   = ShaderManager::CompileCS("scalable_kinect_fusion\\integration\\cs_integrate_rootgrid.glsl"     , "main", DefineString.c_str());
        m_IntegrateLevel1GridCSPtr = ShaderManager::CompileCS("scalable_kinect_fusion\\integration\\cs_integrate_level1grid.glsl"   , "main", DefineString.c_str());
        m_IntegrateTSDFCSPtr       = ShaderManager::CompileCS("scalable_kinect_fusion\\integration\\cs_integrate_tsdf.glsl"         , "main", DefineString.c_str());
        m_PointsRootGridVSPtr      = ShaderManager::CompileVS("scalable_kinect_fusion\\rasterization_reverse\\vs_rootgrid.glsl"     , "main", DefineString.c_str());
        m_PointsRootGridGSPtr      = ShaderManager::CompileGS("scalable_kinect_fusion\\rasterization_reverse\\gs_rootgrid.glsl"     , "main", DefineString.c_str());
        m_PointsRootGridFSPtr      = ShaderManager::CompilePS("scalable_kinect_fusion\\rasterization_reverse\\fs_rootgrid.glsl"     , "main", DefineString.c_str());
        m_PointsRootGridCSPtr      = ShaderManager::CompileCS("scalable_kinect_fusion\\rasterization_reverse\\cs_gather.glsl"       , "main", DefineString.c_str());
        m_PointsFullCSPtr          = ShaderManager::CompileCS("scalable_kinect_fusion\\rasterization_reverse\\cs_gather_full.glsl"  , "main", DefineString.c_str());
        m_FillIndirectBufferCSPtr  = ShaderManager::CompileCS("scalable_kinect_fusion\\cs_fill_indirect.glsl"                       , "main", DefineString.c_str());

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
			AABB[PlaneIndex * 2] = rKey[PlaneIndex] * m_VolumeSizes[0];
			AABB[PlaneIndex * 2 + 1] = AABB[PlaneIndex * 2] + m_VolumeSizes[0];
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
        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(m_CubeMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
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
        
        const unsigned int IndexCount = m_CubeMeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices();
        const unsigned int InstanceCount = static_cast<unsigned int>(m_RootVolumeMap.size());
        ClearBuffer(m_AtomicCounterBufferPtr, InstanceCount);
        ContextManager::DrawIndexedInstanced(IndexCount, InstanceCount, 0, 0, 0);

        ContextManager::ResetShaderVS();
        ContextManager::ResetShaderPS();
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::GatherVolumeCounters(unsigned int Count, CBufferPtr CounterBuffer, CBufferPtr QueueBuffer, CBufferPtr IndirectBuffer)
    {
        ContextManager::Barrier();

        ContextManager::SetShaderCS(m_VolumeCountersCSPtr);

        SIndirectBuffers IndirectBufferData = {};
        IndirectBufferData.m_Indexed.m_IndexCount = 36;
        BufferManager::UploadBufferData(IndirectBuffer, &IndirectBufferData);

        ContextManager::SetResourceBuffer(0, CounterBuffer);
        ContextManager::SetResourceBuffer(1, IndirectBuffer);
        ContextManager::SetResourceBuffer(2, QueueBuffer);
        
        ContextManager::Dispatch(Count, 1, 1);
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::GatherGridCounters(unsigned int Count, CBufferPtr CounterBuffer, CBufferPtr QueueBuffer, CBufferPtr IndirectBuffer)
    {
        ContextManager::Barrier();

        ContextManager::SetShaderCS(m_GridCountersCSPtr);

        SIndirectBuffers IndirectBufferData = {};
        IndirectBufferData.m_Indexed.m_IndexCount = 36;
        BufferManager::UploadBufferData(IndirectBuffer, &IndirectBufferData);

        ContextManager::SetResourceBuffer(0, CounterBuffer);
        ContextManager::SetResourceBuffer(1, IndirectBuffer);
        ContextManager::SetResourceBuffer(2, QueueBuffer);

        ContextManager::Dispatch(Count, 1, 1);
    }

	// -----------------------------------------------------------------------------
    
    void CScalableSLAMReconstructor::CreateIntegrationQueues(std::vector<uint32_t>& rVolumeQueue)
    {
        ////////////////////////////////////////////////////////////////////////////////
        // Create buffers for new volumes
        ////////////////////////////////////////////////////////////////////////////////

        ContextManager::SetConstantBuffer(0, m_IntrinsicsConstantBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);
        ContextManager::SetConstantBuffer(2, m_GridRasterizationBufferPtr);

        ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_RawVertexMapPtr));

        const unsigned int Offset = 0;

        for (uint32_t VolumeIndex : rVolumeQueue)
        {
            assert(m_RootVolumeVector[VolumeIndex] != nullptr);

            auto& rRootVolume = *m_RootVolumeVector[VolumeIndex];

            rRootVolume.m_IsVisible = true;

            if (rRootVolume.m_Level1QueuePtr == nullptr)
            {
                SBufferDescriptor ConstantBufferDesc = {};
                ConstantBufferDesc.m_Binding = CBuffer::ResourceBuffer;
                ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
                ConstantBufferDesc.m_NumberOfBytes = sizeof(uint32_t) * m_ReconstructionSettings.m_VoxelsPerGrid[0];
                ConstantBufferDesc.m_pBytes = nullptr;
                ConstantBufferDesc.m_Usage = CBuffer::GPURead;
                rRootVolume.m_Level1QueuePtr = BufferManager::CreateBuffer(ConstantBufferDesc);
                ConstantBufferDesc.m_NumberOfBytes *= m_ReconstructionSettings.m_VoxelsPerGrid[1];
                rRootVolume.m_Level2QueuePtr = BufferManager::CreateBuffer(ConstantBufferDesc);

                SIndirectBuffers InitialData = {};
                ConstantBufferDesc.m_NumberOfBytes = sizeof(SIndirectBuffers);
                ConstantBufferDesc.m_pBytes = &InitialData;
                rRootVolume.m_IndirectLevel1Buffer = BufferManager::CreateBuffer(ConstantBufferDesc);
                rRootVolume.m_IndirectLevel2Buffer = BufferManager::CreateBuffer(ConstantBufferDesc);
            }
        }

        if (m_ReconstructionSettings.m_UseFullVolumeIntegration)
        {
            ////////////////////////////////////////////////////////////////////////////////////////////////
            // Render point cloud to full volume
            ////////////////////////////////////////////////////////////////////////////////////////////////

            Performance::BeginEvent("Rasterize point cloud to full volume");

            ContextManager::SetTargetSet(m_FullVolumeTargetSetPtr);
            ContextManager::SetViewPortSet(m_FullVolumeViewPort);

            const unsigned int Offset = 0;
            ContextManager::SetShaderVS(m_PointsRootGridVSPtr);
            ContextManager::SetShaderPS(m_PointsRootGridGSPtr);
            ContextManager::SetShaderPS(m_PointsRootGridFSPtr);
            ContextManager::Barrier();
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_MULTISAMPLE);
            ContextManager::SetVertexBuffer(m_Grid8MeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
            ContextManager::SetIndexBuffer(m_Grid8MeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);
            ContextManager::SetInputLayout(m_CubeInputLayoutPtr);
            ContextManager::SetTopology(STopology::PointList);


            if (m_UseConservativeRasterization)
            {
                glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
            }
            for (uint32_t VolumeIndex : rVolumeQueue)
            {
                auto& rRootVolume = *m_RootVolumeVector[VolumeIndex];

                RasterizeFullVolumeReverse(rRootVolume);

                ContextManager::SetShaderCS(m_PointsFullCSPtr);
                
                ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_FullVolumePtr));

                SIndirectBuffers IndirectBufferData = {};
                IndirectBufferData.m_Indexed.m_IndexCount = m_Grid8MeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices();
                BufferManager::UploadBufferData(rRootVolume.m_IndirectLevel1Buffer, &IndirectBufferData);
                BufferManager::UploadBufferData(rRootVolume.m_IndirectLevel2Buffer, &IndirectBufferData);

                ContextManager::SetResourceBuffer(0, rRootVolume.m_IndirectLevel1Buffer);
                ContextManager::SetResourceBuffer(1, rRootVolume.m_IndirectLevel2Buffer);
                ContextManager::SetResourceBuffer(2, rRootVolume.m_Level1QueuePtr);
                ContextManager::SetResourceBuffer(3, rRootVolume.m_Level2QueuePtr);

                ContextManager::Dispatch(16, 16, 16);

                ContextManager::Barrier();
            }
            if (m_UseConservativeRasterization)
            {
                glDisable(GL_CONSERVATIVE_RASTERIZATION_NV);
            }
            
            Performance::EndEvent();
        }
        else
        {
            if (m_ReconstructionSettings.m_UseReverseIntegration)
            {
                ////////////////////////////////////////////////////////////////////////////////////////////////
                // Render point cloud to root grids
                ////////////////////////////////////////////////////////////////////////////////////////////////

                Performance::BeginEvent("Rasterize point cloud to root grids");

                ContextManager::SetTargetSet(m_RootGridVolumeTargetSetPtr);
                ContextManager::SetViewPortSet(m_RootGridViewPort);

                const unsigned int Offset = 0;
                ContextManager::SetShaderVS(m_PointsRootGridVSPtr);
                ContextManager::SetShaderPS(m_PointsRootGridGSPtr);
                ContextManager::SetShaderPS(m_PointsRootGridFSPtr);
                ContextManager::Barrier();
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_MULTISAMPLE);
                ContextManager::SetVertexBuffer(m_Grid8MeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
                ContextManager::SetIndexBuffer(m_Grid8MeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);
                ContextManager::SetInputLayout(m_CubeInputLayoutPtr);
                ContextManager::SetTopology(STopology::PointList);

                if (m_UseConservativeRasterization)
                {
                    glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
                }
                for (uint32_t VolumeIndex : rVolumeQueue)
                {
                    auto& rRootVolume = *m_RootVolumeVector[VolumeIndex];

                    RasterizeRootGridReverse(rRootVolume);
                }
                if (m_UseConservativeRasterization)
                {
                    glDisable(GL_CONSERVATIVE_RASTERIZATION_NV);
                }

                Performance::EndEvent();
            }
            else
            {
                ////////////////////////////////////////////////////////////////////////////////////////////////
                // Rasterize root grids
                ////////////////////////////////////////////////////////////////////////////////////////////////

                Performance::BeginEvent("Rasterize Root Grids");

                ContextManager::SetShaderVS(m_RasterizeRootGridVSPtr);
                ContextManager::SetShaderPS(m_RasterizeRootGridFSPtr);
                ContextManager::Barrier();
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_MULTISAMPLE);
                ContextManager::SetVertexBuffer(m_Grid16MeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
                ContextManager::SetIndexBuffer(m_Grid16MeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);
                ContextManager::SetInputLayout(m_CubeInputLayoutPtr);
                ContextManager::SetTopology(STopology::TriangleList);

                for (uint32_t VolumeIndex : rVolumeQueue)
                {
                    auto& rRootVolume = *m_RootVolumeVector[VolumeIndex];

                    RasterizeRootGrid(rRootVolume);
                }

                Performance::EndEvent();
            }

            ContextManager::ResetShaderVS();
            ContextManager::ResetShaderGS();
            ContextManager::ResetShaderPS();
            ContextManager::ResetShaderCS();

            ////////////////////////////////////////////////////////////////////////////////////////////////
            // Rasterize level1 grids
            ////////////////////////////////////////////////////////////////////////////////////////////////

            Performance::BeginEvent("Rasterize Level 1 Grids");

            ContextManager::SetTargetSet(m_EmptyTargetSetPtr);
            ContextManager::SetViewPortSet(m_DepthViewPortSetPtr);

            ContextManager::SetShaderVS(m_RasterizeLevel1GridVSPtr);
            ContextManager::SetShaderPS(m_RasterizeLevel1GridFSPtr);
            ContextManager::SetVertexBuffer(m_Grid8MeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer());
            ContextManager::SetIndexBuffer(m_Grid8MeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);
            ContextManager::SetInputLayout(m_CubeInputLayoutPtr);
            ContextManager::SetTopology(STopology::TriangleList);

            for (uint32_t VolumeIndex : rVolumeQueue)
            {
                auto& rRootVolume = *m_RootVolumeVector[VolumeIndex];

                RasterizeLevel1Grid(rRootVolume);
            }

            Performance::EndEvent();
        }

        ContextManager::SetShaderCS(m_FillIndirectBufferCSPtr);

        Performance::BeginEvent("Fill indirect buffers");

        for (uint32_t VolumeIndex : rVolumeQueue)
        {
            auto& rRootVolume = *m_RootVolumeVector[VolumeIndex];

            ContextManager::SetResourceBuffer(0, rRootVolume.m_IndirectLevel1Buffer);
            ContextManager::SetResourceBuffer(1, rRootVolume.m_IndirectLevel2Buffer);

            ContextManager::Dispatch(1, 1, 1);
        }

        Performance::EndEvent();

        ContextManager::ResetShaderVS();
        ContextManager::ResetShaderGS();
        ContextManager::ResetShaderPS();
    }
    
    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::IntegrateHierarchies(std::vector<uint32_t>& rVolumeQueue)
    {
        for (uint32_t VolumeIndex : rVolumeQueue)
        {
            assert(m_RootVolumeVector[VolumeIndex] != nullptr);

            auto& rRootVolume = *m_RootVolumeVector[VolumeIndex];

            ////////////////////////////////////////////////////////////////////////////////////////////////
            // Assign GPU memory to volume when necessary
            ////////////////////////////////////////////////////////////////////////////////////////////////

            if (rRootVolume.m_PoolIndex == -1) // Allocate pool memory for volume
            {
                rRootVolume.m_PoolIndex = m_RootVolumePoolItemCount++;

                int Range = sizeof(SVolumePoolItem);
                int Offset = rRootVolume.m_PoolIndex * sizeof(SVolumePoolItem);

                SVolumePoolItem Item;
                Item.m_NearSurface = false;
                Item.m_Offset = rRootVolume.m_Offset;

                BufferManager::UploadBufferData(m_VolumeBuffers.m_RootVolumePoolPtr, &Item, Offset, Range);
            }
        }

        BufferManager::UploadBufferData(m_VolumeBuffers.m_PoolItemCountBufferPtr, &m_RootVolumePoolItemCount, 0, sizeof(uint32_t));

        ContextManager::SetResourceBuffer(0, m_VolumeBuffers.m_RootVolumePoolPtr);
        ContextManager::SetResourceBuffer(1, m_VolumeBuffers.m_RootGridPoolPtr);
        ContextManager::SetResourceBuffer(2, m_VolumeBuffers.m_Level1PoolPtr);
        ContextManager::SetResourceBuffer(3, m_VolumeBuffers.m_TSDFPoolPtr);
        ContextManager::SetResourceBuffer(4, m_VolumeBuffers.m_PoolItemCountBufferPtr);
        ContextManager::SetResourceBuffer(5, m_VolumeIndexBufferPtr);

        ////////////////////////////////////////////////////////////////////////////////////////////////
        // Fill root grids
        ////////////////////////////////////////////////////////////////////////////////////////////////

        Performance::BeginEvent("Fill root grids");

        ContextManager::SetShaderCS(m_IntegrateRootGridCSPtr);

        for (uint32_t VolumeIndex : rVolumeQueue)
        {
            auto& rRootVolume = *m_RootVolumeVector[VolumeIndex];

            ////////////////////////////////////////////////////////////////////////////////////////////////
            // Set current volume
            ////////////////////////////////////////////////////////////////////////////////////////////////
            
            BufferManager::UploadBufferData(m_VolumeIndexBufferPtr, &rRootVolume.m_PoolIndex);
            
            ////////////////////////////////////////////////////////////////////////////////////////////////
            // Integrate into root grid
            ////////////////////////////////////////////////////////////////////////////////////////////////

            ContextManager::SetResourceBuffer(6, rRootVolume.m_Level1QueuePtr);
            ContextManager::SetResourceBuffer(7, rRootVolume.m_IndirectLevel1Buffer);

            ContextManager::Barrier();

            ContextManager::DispatchIndirect(rRootVolume.m_IndirectLevel1Buffer, SIndirectBuffers::s_ComputeDivOffset);
        }

        Performance::EndEvent();

        ////////////////////////////////////////////////////////////////////////////////////////////////
        // Fill intenal grids
        ////////////////////////////////////////////////////////////////////////////////////////////////

        Performance::BeginEvent("Fill internal grids");

        ContextManager::SetShaderCS(m_IntegrateLevel1GridCSPtr);

        for (uint32_t VolumeIndex : rVolumeQueue)
        {
            auto& rRootVolume = *m_RootVolumeVector[VolumeIndex];

            ////////////////////////////////////////////////////////////////////////////////////////////////
            // Set current volume
            ////////////////////////////////////////////////////////////////////////////////////////////////

            BufferManager::UploadBufferData(m_VolumeIndexBufferPtr, &rRootVolume.m_PoolIndex);

            ////////////////////////////////////////////////////////////////////////////////////////////////
            // Integrate into root grid
            ////////////////////////////////////////////////////////////////////////////////////////////////

            ContextManager::SetResourceBuffer(6, rRootVolume.m_Level2QueuePtr);
            ContextManager::SetResourceBuffer(7, rRootVolume.m_IndirectLevel2Buffer);

            ContextManager::Barrier();

            ContextManager::DispatchIndirect(rRootVolume.m_IndirectLevel2Buffer, SIndirectBuffers::s_ComputeDivOffset);
        }

        Performance::EndEvent();

        ////////////////////////////////////////////////////////////////////////////////////////////////
        // Compute new TSDF values
        ////////////////////////////////////////////////////////////////////////////////////////////////

        Performance::BeginEvent("Compute new TSDF");

        ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_RawDepthBufferPtr));
        ContextManager::SetShaderCS(m_IntegrateTSDFCSPtr);

        for (uint32_t VolumeIndex : rVolumeQueue)
        {
            auto& rRootVolume = *m_RootVolumeVector[VolumeIndex];

            ////////////////////////////////////////////////////////////////////////////////////////////////
            // Set current volume
            ////////////////////////////////////////////////////////////////////////////////////////////////

            BufferManager::UploadBufferData(m_VolumeIndexBufferPtr, &rRootVolume.m_PoolIndex);

            ////////////////////////////////////////////////////////////////////////////////////////////////
            // Integrate into TSDF grids
            ////////////////////////////////////////////////////////////////////////////////////////////////

            ContextManager::SetResourceBuffer(6, rRootVolume.m_Level2QueuePtr);
            ContextManager::SetResourceBuffer(7, rRootVolume.m_IndirectLevel2Buffer);

            ContextManager::Barrier();

            ContextManager::DispatchIndirect(rRootVolume.m_IndirectLevel2Buffer, SIndirectBuffers::s_ComputeOffset);
        }

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::RasterizeRootGrid(SRootVolume& rRootGrid)
    {
        SGridRasterization GridData = {};
        GridData.m_Resolution = m_ReconstructionSettings.m_GridResolutions[0];
        GridData.m_CubeSize = m_VolumeSizes[1];
        GridData.m_ParentSize = m_VolumeSizes[0];
        GridData.m_Offset = rRootGrid.m_Offset;

        BufferManager::UploadBufferData(m_GridRasterizationBufferPtr, &GridData);

        SIndirectBuffers IndirectBufferData = {};
        IndirectBufferData.m_Indexed.m_IndexCount = m_Grid8MeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices();
        BufferManager::UploadBufferData(rRootGrid.m_IndirectLevel1Buffer, &IndirectBufferData);
        
        ClearBuffer(m_VolumeAtomicCounterBufferPtr, GridData.m_Resolution * GridData.m_Resolution * GridData.m_Resolution);

        ContextManager::SetResourceBuffer(3, rRootGrid.m_Level1QueuePtr);
        ContextManager::SetResourceBuffer(4, m_VolumeAtomicCounterBufferPtr);
        ContextManager::SetResourceBuffer(5, rRootGrid.m_IndirectLevel1Buffer);

        const unsigned int IndexCount = m_Grid16MeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices();
        ContextManager::DrawIndexed(IndexCount, 0, 0);
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::RasterizeRootGridReverse(SRootVolume& rRootGrid)
    {
        ////////////////////////////////////////////////////////////////////////////////
        // Render point cloud into 3D texture
        ////////////////////////////////////////////////////////////////////////////////

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetShaderVS(m_PointsRootGridVSPtr);
        ContextManager::SetShaderGS(m_PointsRootGridGSPtr);
        ContextManager::SetShaderPS(m_PointsRootGridFSPtr);

        SPointRasterization BufferData;

        BufferData.m_Offset = rRootGrid.m_Offset;
        BufferData.m_Resolution = m_ReconstructionSettings.m_GridResolutions[0];
        BufferManager::UploadBufferData(m_PointRasterizationBufferPtr, &BufferData);

        ContextManager::SetConstantBuffer(3, m_PointRasterizationBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);

        ContextManager::Draw(m_pRGBDCameraControl->GetDepthPixelCount(), 0);

        ////////////////////////////////////////////////////////////////////////////////
        // Gather all tagged voxels
        ////////////////////////////////////////////////////////////////////////////////

        ContextManager::SetShaderCS(m_PointsRootGridCSPtr);

        SIndirectBuffers IndirectBufferData = {};
        IndirectBufferData.m_Indexed.m_IndexCount = m_Grid8MeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices();
        BufferManager::UploadBufferData(rRootGrid.m_IndirectLevel1Buffer, &IndirectBufferData);

        ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_RootGridVolumePtr));
        ContextManager::SetResourceBuffer(2, rRootGrid.m_Level1QueuePtr);
        ContextManager::SetResourceBuffer(3, rRootGrid.m_IndirectLevel1Buffer);

        ContextManager::Dispatch(1, 1, 16);

        ContextManager::Barrier();
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::RasterizeFullVolumeReverse(SRootVolume& rRootGrid)
    {
        ////////////////////////////////////////////////////////////////////////////////
        // Render point cloud into 3D texture
        ////////////////////////////////////////////////////////////////////////////////

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetShaderVS(m_PointsRootGridVSPtr);
        ContextManager::SetShaderGS(m_PointsRootGridGSPtr);
        ContextManager::SetShaderPS(m_PointsRootGridFSPtr);

        SPointRasterization BufferData;

        BufferData.m_Offset = rRootGrid.m_Offset;
        BufferData.m_Resolution = 128;
        BufferManager::UploadBufferData(m_PointRasterizationBufferPtr, &BufferData);

        ContextManager::SetConstantBuffer(3, m_PointRasterizationBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);

        ContextManager::Draw(m_pRGBDCameraControl->GetDepthPixelCount(), 0);
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::RasterizeLevel1Grid(SRootVolume& rRootGrid)
    {
        SGridRasterization GridData = {};
        GridData.m_Resolution = m_ReconstructionSettings.m_GridResolutions[1];
        GridData.m_CubeSize = m_VolumeSizes[2];
        GridData.m_ParentSize = m_VolumeSizes[1];
        GridData.m_Offset = rRootGrid.m_Offset;

        BufferManager::UploadBufferData(m_GridRasterizationBufferPtr, &GridData);
        
        ContextManager::SetResourceBuffer(2, rRootGrid.m_Level1QueuePtr);
        ContextManager::SetResourceBuffer(3, rRootGrid.m_Level2QueuePtr);
        ContextManager::SetResourceBuffer(4, m_VolumeAtomicCounterBufferPtr);
        ContextManager::SetResourceBuffer(5, rRootGrid.m_IndirectLevel2Buffer);

        SIndirectBuffers IndirectBufferData = {};
        IndirectBufferData.m_Indexed.m_IndexCount = m_Grid8MeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices();
        BufferManager::UploadBufferData(rRootGrid.m_IndirectLevel2Buffer, &IndirectBufferData);

        ClearBuffer(m_VolumeAtomicCounterBufferPtr, 128 * 128 * 128);
        
        ContextManager::DrawIndexedIndirect(rRootGrid.m_IndirectLevel1Buffer, SIndirectBuffers::s_IndexedOffset);
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
			MaxIndex[i] = static_cast<int>(BBMax[i] / m_VolumeSizes[0]);
			MinIndex[i] = static_cast<int>(BBMin[i] / m_VolumeSizes[0]);
		}

		SRootVolume RootVolume;
        RootVolume.m_PoolIndex = -1;

		for (int x = MinIndex[0] - 1; x <= MaxIndex[0]; ++ x)
		{
			for (int y = MinIndex[1] - 1; y <= MaxIndex[1]; ++ y)
			{
				for (int z = MinIndex[2] - 1; z <= MaxIndex[2]; ++ z)
				{
					Int3 Key = Int3(x, y, z);
					
					if (m_RootVolumeMap.count(Key) == 0 && RootGridInFrustum(Key))
					{
						RootVolume.m_Offset = Key;
						RootVolume.m_IsVisible = true;

						m_RootVolumeMap[Key] = RootVolume;
					}
				}
			}
		}

        ////////////////////////////////////////////////////////////////////////////////
        // Prepare instance buffers
        ////////////////////////////////////////////////////////////////////////////////

        ClearBuffer(m_AtomicCounterBufferPtr, m_RootVolumeMap.size());
        
        ////////////////////////////////////////////////////////////////////////////////
        // Create vector and instance buffer for root grid volumes
        ////////////////////////////////////////////////////////////////////////////////

        m_RootVolumeVector.clear();

        // todo: check if resizing is necessary
        SBufferDescriptor ConstantBufferDesc = {};
        ConstantBufferDesc.m_Binding = CBuffer::ResourceBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SInstanceData) * static_cast<unsigned int>(m_RootVolumeMap.size());
        ConstantBufferDesc.m_pBytes = nullptr;
        ConstantBufferDesc.m_Usage = CBuffer::GPURead;
        m_RootVolumeInstanceBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        SInstanceData* pInstanceData = static_cast<SInstanceData*>(BufferManager::MapBuffer(m_RootVolumeInstanceBufferPtr, CBuffer::Write));

		for (auto& rPair : m_RootVolumeMap)
		{
			auto& rRootGrid = rPair.second;

			rRootGrid.m_IsVisible = RootGridInFrustum(rRootGrid.m_Offset);
            
            m_RootVolumeVector.push_back(&rRootGrid);

            SInstanceData InstanceData;
            InstanceData.m_Offset = rRootGrid.m_Offset;
            InstanceData.m_Index = 0; // todo: remove

            *pInstanceData = InstanceData;
            ++ pInstanceData;
        }

        BufferManager::UnmapBuffer(m_RootVolumeInstanceBufferPtr);

        ////////////////////////////////////////////////////////////////////////////////
        // Check all possible root grid volumes for depth data
        ////////////////////////////////////////////////////////////////////////////////

        ContextManager::SetTargetSet(m_EmptyTargetSetPtr);
        ContextManager::SetViewPortSet(m_DepthViewPortSetPtr);

        Performance::BeginEvent("Check Root Volumes");

        ContextManager::ResetShaderCS();

        RasterizeRootVolumes();
        GatherVolumeCounters(static_cast<unsigned int>(m_RootVolumeMap.size()), m_AtomicCounterBufferPtr, m_VolumeQueueBufferPtr, m_IndexedIndirectBufferPtr);

        Performance::EndEvent();

        // todo: try to get rid of mapping
        SIndirectBuffers* pIndirectData = static_cast<SIndirectBuffers*>(BufferManager::MapBuffer(m_IndexedIndirectBufferPtr, CBuffer::ReadWrite));
        int VolumeCount = pIndirectData->m_Indexed.m_InstanceCount;
        BufferManager::UnmapBuffer(m_IndexedIndirectBufferPtr);

        ////////////////////////////////////////////////////////////////////////////////
        // Integrate depth into individual root volume grids
        ////////////////////////////////////////////////////////////////////////////////

        for (auto& rRootVolume : m_RootVolumeVector)
        {
            rRootVolume->m_IsVisible = false;
        }

        if (VolumeCount > 0)
        {
            std::vector<uint32_t> VolumeQueue(VolumeCount);
            uint32_t* pVoxelQueue = static_cast<uint32_t*>(BufferManager::MapBufferRange(m_VolumeQueueBufferPtr, CBuffer::Read, 0, VolumeCount * sizeof(uint32_t)));
            memcpy(VolumeQueue.data(), pVoxelQueue, sizeof(uint32_t) * VolumeCount);
            BufferManager::UnmapBuffer(m_VolumeQueueBufferPtr);

            Performance::BeginEvent("Create integration queues");
            CreateIntegrationQueues(VolumeQueue);
            Performance::EndEvent();
            
            Performance::BeginEvent("Integrate hierarchy");
            IntegrateHierarchies(VolumeQueue);
            Performance::EndEvent();

            // Compute the AABB for the whole reconstruction

            Base::Int3 TotalMinOffset = m_RootVolumeVector[VolumeQueue[0]]->m_Offset;
            Base::Int3 TotalMaxOffset = TotalMinOffset;

            for (int i = 1; i < VolumeQueue.size(); ++i)
            {
                Base::Int3 MinOffset = m_RootVolumeVector[VolumeQueue[i]]->m_Offset;
                Base::Int3 MaxOffset = MinOffset;

                TotalMinOffset[0] = Base::Min(TotalMinOffset[0], MinOffset[0]);
                TotalMinOffset[1] = Base::Min(TotalMinOffset[1], MinOffset[1]);
                TotalMinOffset[2] = Base::Min(TotalMinOffset[2], MinOffset[2]);

                TotalMaxOffset[0] = Base::Max(TotalMaxOffset[0], MaxOffset[0]);
                TotalMaxOffset[1] = Base::Max(TotalMaxOffset[1], MaxOffset[1]);
                TotalMaxOffset[2] = Base::Max(TotalMaxOffset[2], MaxOffset[2]);
            }

            const int CurrentWidth = m_VolumeBuffers.m_RootVolumeTotalWidth / 2;

            if (TotalMinOffset[0] <= -CurrentWidth || TotalMinOffset[1] <= -CurrentWidth || TotalMinOffset[2] <= -CurrentWidth ||
                TotalMaxOffset[0] >   CurrentWidth || TotalMaxOffset[1] >   CurrentWidth || TotalMaxOffset[2] >   CurrentWidth)
            {
                // TODO: resize buffer
                assert(false);
            }

            m_VolumeBuffers.m_MinOffset = TotalMinOffset;
            m_VolumeBuffers.m_MaxOffset = TotalMaxOffset;

            for (uint32_t VolumeIndex : VolumeQueue)
            {
                auto& rRootVolume = *m_RootVolumeVector[VolumeIndex];
                
                // Store pool indices in root volume position buffer

                const int Width = m_VolumeBuffers.m_RootVolumeTotalWidth;
                const Base::Int3 Offset = rRootVolume.m_Offset + Width / 2; // Offset value by half of width so it is positive
                const int Index = Offset[0] + (Offset[1] * Width) + (Offset[2] * Width * Width);

                BufferManager::UploadBufferData(m_VolumeBuffers.m_RootVolumePositionBufferPtr, &rRootVolume.m_PoolIndex, Index * sizeof(int32_t), sizeof(int32_t));
            }
        }
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

        TextureDescriptor.m_NumberOfPixelsU = 16;
        TextureDescriptor.m_NumberOfPixelsV = 16;
        TextureDescriptor.m_NumberOfPixelsW = 16;
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding = CTextureBase::RenderTarget | CTextureBase::ShaderResource;
        TextureDescriptor.m_Access = CTextureBase::CPUWrite;
        TextureDescriptor.m_Usage = CTextureBase::GPUReadWrite;
        TextureDescriptor.m_Semantic = CTextureBase::UndefinedSemantic;
        TextureDescriptor.m_Format = CTextureBase::R8_UINT;

        m_RootGridVolumePtr = TextureManager::CreateTexture3D(TextureDescriptor);
        m_RootGridVolumeTargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTextureBasePtr>(m_RootGridVolumePtr));

        TextureDescriptor.m_NumberOfPixelsU = 16 * 8;
        TextureDescriptor.m_NumberOfPixelsV = 16 * 8;
        TextureDescriptor.m_NumberOfPixelsW = 16 * 8;

        m_FullVolumePtr = TextureManager::CreateTexture3D(TextureDescriptor);;
        m_FullVolumeTargetSetPtr = TargetSetManager::CreateTargetSet(static_cast<CTextureBasePtr>(m_FullVolumePtr));

        m_EmptyTargetSetPtr = TargetSetManager::CreateEmptyTargetSet(m_pRGBDCameraControl->GetDepthWidth(), m_pRGBDCameraControl->GetDepthHeight());
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

        const int ICPRowCount = DivUp(m_pRGBDCameraControl->GetDepthWidth() , g_TileSize2D) *
                                DivUp(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

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

        ConstantBufferDesc.m_Usage = CBuffer::GPURead;
        ConstantBufferDesc.m_Binding = CBuffer::ResourceBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SIndirectBuffers);
        m_IndexedIndirectBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        
        ConstantBufferDesc.m_Binding = CBuffer::ConstantBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SGridRasterization);
        ConstantBufferDesc.m_pBytes = nullptr;
        ConstantBufferDesc.m_Usage = CBuffer::GPURead;
        m_GridRasterizationBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_NumberOfBytes = sizeof(SPointRasterization);
        m_PointRasterizationBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_Binding = CBuffer::ResourceBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = m_ReconstructionSettings.m_GridResolutions[0] * m_ReconstructionSettings.m_GridResolutions[1];
        ConstantBufferDesc.m_NumberOfBytes = ConstantBufferDesc.m_NumberOfBytes *
                                             ConstantBufferDesc.m_NumberOfBytes *
                                             ConstantBufferDesc.m_NumberOfBytes * sizeof(uint32_t);
        ConstantBufferDesc.m_pBytes = nullptr;
        ConstantBufferDesc.m_Usage = CBuffer::GPURead;
        m_VolumeAtomicCounterBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        m_VolumeQueueBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        ConstantBufferDesc.m_NumberOfBytes = sizeof(uint32_t) * 2048;
        m_AtomicCounterBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        const unsigned int RootVolumePositionBufferSize = g_AABB * g_AABB * g_AABB * sizeof(uint32_t);

        ConstantBufferDesc.m_NumberOfBytes = RootVolumePositionBufferSize;
        m_VolumeBuffers.m_RootVolumePositionBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        ConstantBufferDesc.m_NumberOfBytes = g_RootVolumePoolSize;
        m_VolumeBuffers.m_RootVolumePoolPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        ConstantBufferDesc.m_NumberOfBytes = g_RootGridPoolSize;
        m_VolumeBuffers.m_RootGridPoolPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        ConstantBufferDesc.m_NumberOfBytes = g_Level1GridPoolSize;
        m_VolumeBuffers.m_Level1PoolPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        ConstantBufferDesc.m_NumberOfBytes = g_TSDFPoolSize;
        m_VolumeBuffers.m_TSDFPoolPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_NumberOfBytes = sizeof(uint32_t) * 4;// m_ReconstructionSettings.GRID_LEVELS;
        m_VolumeBuffers.m_PoolItemCountBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        
        ConstantBufferDesc.m_NumberOfBytes = sizeof(int32_t) * 4;// 16 bytes = minimum
        m_VolumeIndexBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_Binding = CBuffer::ConstantBuffer;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SScalableRaycastConstantBuffer);
        m_VolumeBuffers.m_AABBBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
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

            BufferManager::UploadBufferData(m_TrackingDataConstantBufferPtr, &TrackingData);

            Performance::EndEvent();
        }

        //////////////////////////////////////////////////////////////////////////////////////
        // Read pool sizes
        //////////////////////////////////////////////////////////////////////////////////////

        int* pPoolSizes = static_cast<int*>(BufferManager::MapBuffer(m_VolumeBuffers.m_PoolItemCountBufferPtr, CBuffer::EMap::Read));
        m_VolumeBuffers.m_RootGridPoolSize = pPoolSizes[0];
        m_VolumeBuffers.m_Level1PoolSize = pPoolSizes[1];
        m_VolumeBuffers.m_TSDFPoolSize = pPoolSizes[2];
        BufferManager::UnmapBuffer(m_VolumeBuffers.m_PoolItemCountBufferPtr);
        
        if (m_VolumeBuffers.m_RootGridPoolSize * m_ReconstructionSettings.m_VoxelsPerGrid[0] * sizeof(SGridPoolItem) > g_RootGridPoolSize)
        {
            BASE_CONSOLE_ERROR("Rootgrid pool is full!");
        }
        if (m_VolumeBuffers.m_Level1PoolSize * m_ReconstructionSettings.m_VoxelsPerGrid[1] * sizeof(SGridPoolItem) > g_Level1GridPoolSize)
        {
            BASE_CONSOLE_ERROR("Level1 pool is full!");
        }
        if (m_VolumeBuffers.m_TSDFPoolSize * m_ReconstructionSettings.m_VoxelsPerGrid[2] * sizeof(STSDFPoolItem) > g_TSDFPoolSize)
        {
            BASE_CONSOLE_ERROR("TSDF pool buffer is full!");
        }

        //////////////////////////////////////////////////////////////////////////////////////
        // Integrate and raycast pyramid
        //////////////////////////////////////////////////////////////////////////////////////

        Performance::BeginEvent("TSDF Integration and Raycasting");

        if (!m_IsIntegrationPaused)
        {
            Performance::BeginEvent("Updating root grid");

            UpdateFrustum();
            UpdateRootrids();

            Performance::EndEvent();
        }
        Performance::BeginEvent("Raycasting");

        SScalableRaycastConstantBuffer Data;

        for (int i = 0; i < 3; ++i)
        {
            Data.m_AABBMin[i] = m_VolumeBuffers.m_MinOffset[i] * m_ReconstructionSettings.m_VolumeSize;
            Data.m_AABBMax[i] = (m_VolumeBuffers.m_MaxOffset[i] + 1.0f) * m_ReconstructionSettings.m_VolumeSize;
        }

        Data.m_VolumeTextureWidth = m_VolumeBuffers.m_RootVolumeTotalWidth;

        BufferManager::UploadBufferData(m_VolumeBuffers.m_AABBBufferPtr, &Data);

        Raycast();
        Performance::EndEvent();

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
        const int WorkGroupsX = DivUp(m_pRGBDCameraControl->GetDepthWidth(), g_TileSize2D);
        const int WorkGroupsY = DivUp(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

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
            const int WorkGroupsX = DivUp(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = DivUp(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);
            
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
            const int WorkGroupsX = DivUp(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = DivUp(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);

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
            const int WorkGroupsX = DivUp(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = DivUp(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);
                        
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
        const int WorkGroupsX = DivUp(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
        const int WorkGroupsY = DivUp(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);
        
        SIncBuffer TrackingData;
        TrackingData.m_PoseMatrix = rIncPoseMatrix;
        TrackingData.m_InvPoseMatrix = rIncPoseMatrix.GetInverted();
        TrackingData.m_PyramidLevel = PyramidLevel;
        
        BufferManager::UploadBufferData(m_IncPoseMatrixConstantBufferPtr, &TrackingData);

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
        const int SummandsX = DivUp(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
        const int SummandsY = DivUp(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);

        const int Summands = SummandsX * SummandsY;
        const float SummandsLog2 = Log2(static_cast<float>(Summands));
        const int SummandsPOT = 1 << (static_cast<int>(SummandsLog2) + 1);
        
        Base::Int2 BufferData;
        BufferData[0] = Summands / 2;
        BufferData[1] = SummandsPOT / 2;

        BufferManager::UploadBufferData(m_ICPSummationConstantBufferPtr, &BufferData);

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
        void* pICPBuffer = BufferManager::MapBufferRange(m_ICPResourceBufferPtr, CBuffer::EMap::Read, 0, sizeof(float) * g_ICPValueCount);
        memcpy(ICPValues, pICPBuffer, sizeof(ICPValues[0]) * g_ICPValueCount);
        BufferManager::UnmapBuffer(m_ICPResourceBufferPtr);

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

    void CScalableSLAMReconstructor::CreateRaycastPyramid()
    {
        ContextManager::SetShaderCS(m_RaycastPyramidCSPtr);

        ContextManager::SetConstantBuffer(0, m_RaycastPyramidConstantBufferPtr);
        
        for (int PyramidLevel = 1; PyramidLevel < m_ReconstructionSettings.m_PyramidLevelCount; ++PyramidLevel)
        {
            const int WorkGroupsX = DivUp(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = DivUp(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);

            float Normalized = 0.0f;
            BufferManager::UploadBufferData(m_RaycastPyramidConstantBufferPtr, &Normalized);
            
            ContextManager::Barrier();
            ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_RaycastVertexMapPtr[PyramidLevel - 1]));
            ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_RaycastVertexMapPtr[PyramidLevel]));
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

            Normalized = 1.0f;
            BufferManager::UploadBufferData(m_RaycastPyramidConstantBufferPtr, &Normalized);

            ContextManager::Barrier();
            ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_RaycastNormalMapPtr[PyramidLevel - 1]));
            ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_RaycastNormalMapPtr[PyramidLevel]));
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
        }
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::Raycast()
    {
        const int WorkGroupsX = DivUp(m_pRGBDCameraControl->GetDepthWidth(), g_TileSize2D);
        const int WorkGroupsY = DivUp(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

        ContextManager::SetShaderCS(m_RaycastCSPtr);
        
        ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_RaycastVertexMapPtr[0]));
        ContextManager::SetImageTexture(2, static_cast<CTextureBasePtr>(m_RaycastNormalMapPtr[0]));

        ContextManager::SetResourceBuffer(0, m_VolumeBuffers.m_RootVolumePoolPtr);
        ContextManager::SetResourceBuffer(1, m_VolumeBuffers.m_RootGridPoolPtr);
        ContextManager::SetResourceBuffer(2, m_VolumeBuffers.m_Level1PoolPtr);
        ContextManager::SetResourceBuffer(3, m_VolumeBuffers.m_TSDFPoolPtr);
        ContextManager::SetResourceBuffer(6, m_VolumeBuffers.m_RootVolumePositionBufferPtr);

        ContextManager::SetConstantBuffer(0, m_IntrinsicsConstantBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);
        ContextManager::SetConstantBuffer(2, m_VolumeBuffers.m_AABBBufferPtr);

        ContextManager::Barrier();

        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
    }
    
    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::ClearBuffer(CBufferPtr BufferPtr, size_t Size)
    {
        assert(Size > 0);
        assert(BufferPtr.IsValid());
        
        GLuint NativeBuffer = static_cast<CNativeBuffer*>(BufferPtr.GetPtr())->m_NativeBuffer;

        //glClearNamedBufferSubData(NativeBuffer, GL_R32UI, 0, Size * sizeof(uint32_t), GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
        glClearNamedBufferData(NativeBuffer, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::ClearPool()
    {
        const uint32_t DataSize = g_MegabyteSize / 4;

        std::vector<int> Data(g_MegabyteSize / sizeof(int));
        std::memset(Data.data(), -1, DataSize);

        for (int i = 0; i < g_RootVolumePoolSize / g_MegabyteSize; ++ i)
        {
            BufferManager::UploadBufferData(m_VolumeBuffers.m_RootVolumePoolPtr, Data.data(), i * DataSize, DataSize);
        }
        for (int i = 0; i < g_RootGridPoolSize / g_MegabyteSize; ++ i)
        {
            BufferManager::UploadBufferData(m_VolumeBuffers.m_RootGridPoolPtr, Data.data(), i * DataSize, DataSize);
        }
        for (int i = 0; i < g_Level1GridPoolSize / g_MegabyteSize; ++ i)
        {
            BufferManager::UploadBufferData(m_VolumeBuffers.m_Level1PoolPtr, Data.data(), i * DataSize, DataSize);
        }

        BufferManager::UploadBufferData(m_VolumeBuffers.m_RootVolumePositionBufferPtr, Data.data());
        ClearBuffer(m_VolumeBuffers.m_PoolItemCountBufferPtr, m_ReconstructionSettings.GRID_LEVELS);

        std::memset(Data.data(), 0, DataSize);
        for (int i = 0; i < g_TSDFPoolSize / g_MegabyteSize; ++ i)
        {
            BufferManager::UploadBufferData(m_VolumeBuffers.m_TSDFPoolPtr, Data.data(), i * DataSize, DataSize);
        }
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::ResetReconstruction(const SReconstructionSettings* pReconstructionSettings)
    {
        m_RootVolumeMap.clear();
        m_RootVolumeVector.clear();

        if (pReconstructionSettings != nullptr)
        {
			assert(pReconstructionSettings->m_IsScalable);
            m_ReconstructionSettings = *pReconstructionSettings;
        }

		SetupData();

		SetupTextures();
		SetupBuffers();
		SetupShaders();

        ClearPool();
    }

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

    CScalableSLAMReconstructor::CRootVolumeMap& CScalableSLAMReconstructor::GetRootVolumeMap()
    {
        return m_RootVolumeMap;
    }

    // -----------------------------------------------------------------------------

    CScalableSLAMReconstructor::CRootVolumeVector& CScalableSLAMReconstructor::GetRootVolumeVector()
    {
        return m_RootVolumeVector;
    }

    // -----------------------------------------------------------------------------

    CScalableSLAMReconstructor::SScalableVolume& CScalableSLAMReconstructor::GetVolume()
    {
        return m_VolumeBuffers;
    }

    // -----------------------------------------------------------------------------

    const std::vector<float>& CScalableSLAMReconstructor::GetVolumeSizes() const
    {
        return m_VolumeSizes;
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

    Gfx::CTexture2DPtr CScalableSLAMReconstructor::GetVertexMap()
    {
        return m_RawVertexMapPtr;
    }

    // -----------------------------------------------------------------------------

    int CScalableSLAMReconstructor::DivUp(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }
} // namespace MR
