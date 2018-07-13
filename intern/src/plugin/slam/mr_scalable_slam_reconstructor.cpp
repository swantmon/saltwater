
#include "plugin/slam/slam_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_console.h"
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

#include "plugin/slam/mr_scalable_slam_reconstructor.h"
#include "plugin/slam/mr_rgbd_camera_control.h"
#include "plugin/slam/mr_kinect_control.h"

#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

#include <gl/glew.h>

#include "engine/graphic/gfx_native_buffer.h"

using namespace MR;
using namespace Gfx;

namespace
{
	//*
	const glm::vec3 g_InitialCameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	const glm::vec3 g_InitialCameraRotation = glm::vec3(3.14f, 0.0f, 0.0f);
	/*/
	const glm::vec3 g_InitialCameraPosition = glm::vec3(0.5f, 0.5f, -0.5f);
	const glm::vec3 g_InitialCameraRotation = glm::vec3(0.0f, 0.0f, 0.0f);
	//*/
    
    const unsigned int g_MegabyteSize = 1024u * 1024u;

    const int g_AABB = 16;

    const int g_MaxVolumeInstanceCount = 512;

    /*

    const unsigned int g_MaxRootVolumePoolSize =        g_MegabyteSize;
    const unsigned int g_MaxRootGridPoolSize   =  16u * g_MegabyteSize;
    const unsigned int g_MaxLevel1GridPoolSize =  64u * g_MegabyteSize;
    const unsigned int g_MaxTSDFPoolSize       = 128u * g_MegabyteSize;
    /*/
    const unsigned int g_MaxRootVolumePoolSize =              g_MegabyteSize;
    const unsigned int g_MaxRootGridPoolSize   =       128u * g_MegabyteSize;
    const unsigned int g_MaxLevel1GridPoolSize =       128u * g_MegabyteSize;
    const unsigned int g_MaxTSDFPoolSize       = 16u * 128u * g_MegabyteSize;
    //*/
            
    const float g_EpsilonDistance = 0.1f;
    const float g_EpsilonAngle = 0.75f;
    
    const int g_ICPValueCount = 27;

    const int g_TileSize1D = 64;
    const int g_TileSize2D = 16;
    const int g_TileSize3D = 8;

    glm::vec3 CubeVertices[] =
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
        glm::mat4 m_KMatrix;
        glm::mat4 m_InvKMatrix;
        glm::vec2 m_FocalPoint;
        glm::vec2 m_FocalLength;
        glm::vec2 m_InvFocalLength;
        glm::vec2 Padding;
    };

    struct STrackingData
    {
        glm::mat4 m_PoseMatrix;
        glm::mat4 m_InvPoseMatrix;
    };

    struct SIncBuffer
    {
        glm::mat4 m_PoseMatrix;
        glm::mat4 m_InvPoseMatrix;
        int m_PyramidLevel;
        float Padding[3];
    };

	struct SPositionBuffer
	{
		glm::vec3 m_Position;
		int m_Index;
	};

    struct SDrawCallBufferData
    {
        glm::mat4 m_WorldMatrix;
    };

    struct SInstanceData
    {
        glm::ivec3 m_Offset;
        int m_Index;
    };
    
    struct SPointRasterization
    {
        glm::ivec3 m_Offset;
        int32_t m_BufferOffset;
    };

    struct SVolumePoolItem
    {
        glm::ivec3 m_Offset;
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

    struct STSDFColorPoolItem
    {
        uint16_t m_TSDF;
        uint16_t m_Weight;
        uint32_t m_Color;
    };

    int DivUp(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }

} // namespace

namespace MR
{
    using namespace Base;
    
    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::Start()
    {
        assert(m_DepthFrameSize.x != 0 && m_DepthFrameSize.y != 0);
        assert(m_ColorFrameSize.x != 0 && m_ColorFrameSize.y != 0);
        assert(m_FocalLength.x != 0 && m_FocalLength.y != 0);
        assert(m_FocalPoint.x != 0.0f && m_FocalPoint.y != 0.0f);
        assert(m_DepthBounds.x != 0.0f && m_DepthBounds.y != 0.0f);

        ////////////////////////////////////////////////////////////////////////////////
        // Check if conservative rasterization is available
        ////////////////////////////////////////////////////////////////////////////////

        const bool EnableConservativeRaster = Core::CProgramParameters::GetInstance().Get("mr:slam:conservative_raster_enable", true);

        m_UseConservativeRasterization = false;

        if (EnableConservativeRaster)
        {
            m_UseConservativeRasterization = Main::IsExtensionAvailable("GL_NV_conservative_raster");

            if (!m_UseConservativeRasterization)
            {
                ENGINE_CONSOLE_INFO("Conservative rasterization is not available. Will use fallback method");
            }
        }

        ////////////////////////////////////////////////////////////////////////////////
        // Setup data, buffer etc.
        ////////////////////////////////////////////////////////////////////////////////
        
        m_DepthPixels = std::vector<unsigned short>(m_DepthFrameSize.x * m_DepthFrameSize.y);
        m_CameraPixels = std::vector<char>(m_DepthFrameSize.x * m_DepthFrameSize.y * 4);

        SetupData();
        SetupMeshes();
        SetupRenderStates();
		SetupShaders();
		SetupTextures();
		SetupBuffers();

        ClearPool();

        //m_PlaneDetector.SetImages(m_ReferenceVertexMapPtr[2], m_ReferenceNormalMapPtr[2]);

        m_pTracker.reset(new CICPTracker(m_DepthFrameSize.x, m_DepthFrameSize.y, m_ReconstructionSettings));
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::SetupRenderStates()
    {
        SViewPortDescriptor ViewPortDescriptor = {};

        ViewPortDescriptor.m_MinDepth = 0.0f;
        ViewPortDescriptor.m_MaxDepth = 1.0f;
        ViewPortDescriptor.m_TopLeftX = 0.0f;
        ViewPortDescriptor.m_TopLeftY = 0.0f;
        ViewPortDescriptor.m_Width = static_cast<float>(m_DepthFrameSize.x);
        ViewPortDescriptor.m_Height = static_cast<float>(m_DepthFrameSize.y);

        Gfx::CViewPortPtr DepthViewPort = ViewManager::CreateViewPort(ViewPortDescriptor);

        m_DepthViewPortSetPtr = ViewManager::CreateViewPortSet(DepthViewPort);

        const int Width = m_ReconstructionSettings.m_GridResolutions[0] * m_ReconstructionSettings.m_GridResolutions[1];
        ViewPortDescriptor.m_Width = static_cast<float>(Width);
        ViewPortDescriptor.m_Height = static_cast<float>(Width);

        Gfx::CViewPortPtr FullVolumeViewPort = ViewManager::CreateViewPort(ViewPortDescriptor);

        m_FullVolumeViewPort = ViewManager::CreateViewPortSet(FullVolumeViewPort);
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::SetupMeshes()
    {
        m_CubeMeshPtr = Gfx::MeshManager::CreateBox(1.0f, 1.0f, 1.0f);
    }
    
	// -----------------------------------------------------------------------------

	void CScalableSLAMReconstructor::SetupData()
	{
        m_IsIntegrationPaused = false;
        m_IsTrackingPaused = false;

        m_PoolFull = false;

        m_CreateNormalsFromTSDF = Core::CProgramParameters::GetInstance().Get("mr:slam:normals_from_tsdf", false);
        m_RaycastBackSides = Core::CProgramParameters::GetInstance().Get("mr:slam:raycast_backsides", true);

        m_RootGridPoolSize = Core::CProgramParameters::GetInstance().Get("mr:slam:pool_sizes:level0", g_MaxRootGridPoolSize / g_MegabyteSize) * g_MegabyteSize;
        m_Level1GridPoolSize = Core::CProgramParameters::GetInstance().Get("mr:slam:pool_sizes:level1", g_MaxLevel1GridPoolSize / g_MegabyteSize) * g_MegabyteSize;
        m_TSDFPoolSize = Core::CProgramParameters::GetInstance().Get("mr:slam:pool_sizes:level2", g_MaxTSDFPoolSize / g_MegabyteSize) * g_MegabyteSize;

        m_ReconstructionSize = 0.0f;

        m_MinWeight = Core::CProgramParameters::GetInstance().Get("mr:slam:min_weight", 15);

        m_VolumeDepthThreshold = Core::CProgramParameters::GetInstance().Get("mr:slam:volume_min_depth_count", 2000);

        const int GridLevelCount = MR::SReconstructionSettings::GRID_LEVELS;

        m_VolumeSizes.resize(GridLevelCount);
        m_VolumeSizes[GridLevelCount - 1] = m_ReconstructionSettings.m_VoxelSize * m_ReconstructionSettings.m_GridResolutions[GridLevelCount - 1];
        for (int i = GridLevelCount - 2; i >= 0; --i)
        {
            m_VolumeSizes[i] = m_VolumeSizes[i + 1] * m_ReconstructionSettings.m_GridResolutions[i];
        }

		const float VolumeSize = m_VolumeSizes[0];
		glm::mat4 PoseRotation, PoseTranslation;
		
		PoseRotation = glm::eulerAngleXYZ(g_InitialCameraRotation[0], g_InitialCameraRotation[1], g_InitialCameraRotation[2]);
        PoseTranslation = glm::translate(g_InitialCameraPosition * VolumeSize);
		
		m_PoseMatrix = PoseTranslation * PoseRotation;
        
        m_RootVolumePoolItemCount = 0;
		m_IntegratedFrameCount = 0;
		m_FrameCount = 0;
		m_TrackingLost = true;

        m_VolumeBuffers.m_RootGridPoolSize = 0;
        m_VolumeBuffers.m_Level1PoolSize= 0;
        m_VolumeBuffers.m_TSDFPoolSize = 0;
        m_VolumeBuffers.m_RootVolumeTotalWidth = g_AABB;

		UpdateFrustum();
	}

	// -----------------------------------------------------------------------------

	void CScalableSLAMReconstructor::UpdateFrustum()
	{
		//Todo: remove magic numbers (focal length/point)

		//float x = (-0.50602675f) / 0.72113f;
		//float y = (-0.499133f) / 0.870799f;
        
        float x = (-m_FocalPoint.x / m_DepthFrameSize.x) / (m_FocalLength.x / m_DepthFrameSize.x);
        float y = (-m_FocalPoint.y / m_DepthFrameSize.y) / (m_FocalLength.y / m_DepthFrameSize.y);
                
        // TODO: use camera near parameter and find out why frustum culling does not work correctly
        // Volumes that touch the pyramid top but are not between near and far are still valid hits for some reason
        // and they are even pass the rasterization step even though they cannot contain valid samples

        const float Near = 2.0f; // m_pRGBDCameraControl->GetMinDepth();
		const float Far = m_DepthBounds.y;

		// near

		m_FrustumPoints[0] = glm::vec3( x * Near,  y * Near, Near);
		m_FrustumPoints[1] = glm::vec3(-x * Near,  y * Near, Near);
		m_FrustumPoints[2] = glm::vec3(-x * Near, -y * Near, Near);
		m_FrustumPoints[3] = glm::vec3( x * Near, -y * Near, Near);

		// far

		m_FrustumPoints[4] = glm::vec3( x * Far,  y * Far, Far);
		m_FrustumPoints[5] = glm::vec3(-x * Far,  y * Far, Far);
		m_FrustumPoints[6] = glm::vec3(-x * Far, -y * Far, Far);
		m_FrustumPoints[7] = glm::vec3( x * Far, -y * Far, Far);

		for (int i = 0; i < m_FrustumPoints.size(); ++i)
		{
			glm::vec4 Corner = glm::vec4(m_FrustumPoints[i], 1.0f);
			Corner = m_PoseMatrix * Corner;
			m_FrustumPoints[i] = glm::vec3(Corner[0], Corner[1], Corner[2]);
		}

		m_FrustumPlanes[0] = GetHessianNormalForm(m_FrustumPoints[0], m_FrustumPoints[2], m_FrustumPoints[1]); // near
		m_FrustumPlanes[1] = GetHessianNormalForm(m_FrustumPoints[6], m_FrustumPoints[7], m_FrustumPoints[4]); // far
		m_FrustumPlanes[2] = GetHessianNormalForm(m_FrustumPoints[4], m_FrustumPoints[3], m_FrustumPoints[0]); // right
		m_FrustumPlanes[3] = GetHessianNormalForm(m_FrustumPoints[1], m_FrustumPoints[6], m_FrustumPoints[5]); // left
		m_FrustumPlanes[4] = GetHessianNormalForm(m_FrustumPoints[4], m_FrustumPoints[1], m_FrustumPoints[5]); // top
		m_FrustumPlanes[5] = GetHessianNormalForm(m_FrustumPoints[7], m_FrustumPoints[6], m_FrustumPoints[2]); // bottom
	}

	// -----------------------------------------------------------------------------

	glm::vec4 CScalableSLAMReconstructor::GetHessianNormalForm(const glm::vec3& rA, const glm::vec3& rB, const glm::vec3& rC)
	{
		glm::vec3 V1 = rB - rA;
        glm::vec3 V2 = rC - rA;

        glm::vec3 Normal = glm::normalize(glm::cross(V1, V2));

        float D = glm::dot(rA, Normal);

		return glm::vec4(Normal, D);
	}

	// -----------------------------------------------------------------------------

	float CScalableSLAMReconstructor::GetPointPlaneDistance(const glm::vec3& rPoint, const glm::vec4& rPlane)
	{
        const float Dot = glm::dot(rPoint, glm::vec3(rPlane));
		return Dot - rPlane[3];
	}

    // -----------------------------------------------------------------------------
    
    void CScalableSLAMReconstructor::Exit()
    {
        const int TSDFItemSize = m_ReconstructionSettings.m_CaptureColor ? sizeof(STSDFColorPoolItem) : sizeof(STSDFPoolItem);

        int* pPoolSizes = static_cast<int*>(BufferManager::MapBuffer(m_VolumeBuffers.m_PoolItemCountBufferPtr, CBuffer::EMap::ReadWrite));
        
        const float Megabyte = 1024.0f * 1024.0f;
        std::stringstream Stream[3];
        Stream[0] << "Rootgrid pool size: " << m_RootVolumePoolItemCount * m_ReconstructionSettings.m_VoxelsPerGrid[0] * sizeof(SGridPoolItem) / Megabyte << " MB";
        Stream[1] << "Level1 pool size  : " << pPoolSizes[1] * m_ReconstructionSettings.m_VoxelsPerGrid[1] * sizeof(SGridPoolItem) / Megabyte << " MB";
        Stream[2] << "TSDF pool size    : " << pPoolSizes[2] * m_ReconstructionSettings.m_VoxelsPerGrid[2] * TSDFItemSize / Megabyte << " MB";

        BufferManager::UnmapBuffer(m_VolumeBuffers.m_PoolItemCountBufferPtr);

        ENGINE_CONSOLE_INFO(Stream[0].str().c_str());
        ENGINE_CONSOLE_INFO(Stream[1].str().c_str());
        ENGINE_CONSOLE_INFO(Stream[2].str().c_str());

        m_BilateralFilterCSPtr = 0;
        m_VertexMapCSPtr = 0;
        m_NormalMapCSPtr = 0;
        m_DownSampleDepthCSPtr = 0;
        m_IntegrateTSDFCSPtr = 0;
        m_RaycastCSPtr = 0;
        m_RaycastPyramidCSPtr = 0;
        m_VolumeCountersCSPtr = 0;
        
        m_RasterizeRootVolumeVSPtr = 0;
        m_RasterizeRootVolumeFSPtr = 0;
        
        m_ClearAtomicCountersCSPtr = 0;

        m_IntegrateRootGridCSPtr = 0;
        m_IntegrateLevel1GridCSPtr = 0;

        m_CubeMeshPtr = 0;
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
        
        m_IntrinsicsConstantBufferPtr = 0;
        m_TrackingDataConstantBufferPtr = 0;
        m_RaycastPyramidConstantBufferPtr = 0;
        m_BilateralFilterConstantBufferPtr = 0;
        m_AtomicCounterBufferPtr = 0;
        m_IndexedIndirectBufferPtr = 0;

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
        const float VoxelSize = m_ReconstructionSettings.m_VoxelSize;

        const std::string InternalFormatString = Core::CProgramParameters::GetInstance().Get("mr:slam:map_format", "rgba16f");

        std::stringstream DefineStream;

        const float TruncatedDistance = m_ReconstructionSettings.m_TruncatedDistance / 1000.0f;

        DefineStream
            << "#define PYRAMID_LEVELS "         << m_ReconstructionSettings.m_PyramidLevelCount    << " \n"
            << "#define VOXEL_SIZE "             << VoxelSize                                       << " \n"
            << "#define VOLUME_SIZE "            << m_VolumeSizes[0]                                << " \n"
            << "#define DEPTH_IMAGE_WIDTH "      << m_DepthFrameSize.x                              << " \n"
            << "#define DEPTH_IMAGE_HEIGHT "     << m_DepthFrameSize.y                              << " \n"
            << "#define TILE_SIZE1D "            << g_TileSize1D                                    << " \n"
            << "#define TILE_SIZE2D "            << g_TileSize2D                                    << " \n"
            << "#define TILE_SIZE3D "            << g_TileSize3D                                    << " \n"
            << "#define TRUNCATED_DISTANCE "     << TruncatedDistance                               << " \n"
            << "#define MAX_INTEGRATION_WEIGHT " << m_ReconstructionSettings.m_MaxIntegrationWeight << " \n"
            << "#define EPSILON_DISTANCE "       << g_EpsilonDistance                               << " \n"
            << "#define EPSILON_ANGLE "          << g_EpsilonAngle                                  << " \n"
            << "#define ICP_VALUE_COUNT "        << g_ICPValueCount                                 << " \n"
            << "#define MAP_TEXTURE_FORMAT "     << InternalFormatString                            << " \n"
            << "#define ROOT_RESOLUTION "        << m_ReconstructionSettings.m_GridResolutions[0]   << " \n"
            << "#define LEVEL1_RESOLUTION "      << m_ReconstructionSettings.m_GridResolutions[1]   << " \n"
            << "#define LEVEL2_RESOLUTION "      << m_ReconstructionSettings.m_GridResolutions[2]   << " \n"
            << "#define VOXELS_PER_ROOTGRID "    << m_ReconstructionSettings.m_VoxelsPerGrid[0]     << " \n"
            << "#define VOXELS_PER_LEVEL1GRID "  << m_ReconstructionSettings.m_VoxelsPerGrid[1]     << " \n"
            << "#define VOXELS_PER_LEVEL2GRID "  << m_ReconstructionSettings.m_VoxelsPerGrid[2]     << " \n"
            << "#define RAYCAST_NEAR "           << m_DepthBounds.x                                 << " \n"
            << "#define RAYCAST_FAR "            << m_DepthBounds.y                                 << " \n"
            << "#define VOLUME_DEPTH_THRESHLD "  << m_VolumeDepthThreshold                          << " \n"
            << "#define MIN_TREE_WEIGHT "        << m_MinWeight                                     << " \n";
        
        if (m_ReconstructionSettings.m_CaptureColor)
        {
            DefineStream << "#define CAPTURE_COLOR\n";
        }
        if (m_UseConservativeRasterization)
        {
            DefineStream << "#define CONSERVATIVE_RASTERIZATION_AVAILABLE\n";
        }
        if (m_CreateNormalsFromTSDF)
        {
            DefineStream << "#define NORMAL_MAP_FROM_TSDF\n";
        }
        if (m_RaycastBackSides)
        {
            DefineStream << "#define RAYCAST_BACKSIDES\n";
        }

        std::string DefineString = DefineStream.str();
        
        m_BilateralFilterCSPtr     = ShaderManager::CompileCS("slam\\scalable_kinect_fusion\\pyramid_creation\\cs_bilateral_filter.glsl"  , "main", DefineString.c_str());
        m_VertexMapCSPtr           = ShaderManager::CompileCS("slam\\scalable_kinect_fusion\\pyramid_creation\\cs_vertex_map.glsl"        , "main", DefineString.c_str());
        m_NormalMapCSPtr           = ShaderManager::CompileCS("slam\\scalable_kinect_fusion\\pyramid_creation\\cs_normal_map.glsl"        , "main", DefineString.c_str());
        m_DownSampleDepthCSPtr     = ShaderManager::CompileCS("slam\\scalable_kinect_fusion\\pyramid_creation\\cs_downsample_depth.glsl"  , "main", DefineString.c_str());
        m_RaycastPyramidCSPtr      = ShaderManager::CompileCS("slam\\scalable_kinect_fusion\\pyramid_creation\\cs_raycast_pyramid.glsl"   , "main", DefineString.c_str());
        m_RaycastCSPtr             = ShaderManager::CompileCS("slam\\scalable_kinect_fusion\\cs_raycast.glsl"                             , "main", DefineString.c_str());
        m_ClearAtomicCountersCSPtr = ShaderManager::CompileCS("slam\\scalable_kinect_fusion\\cs_clear_atomic_buffer.glsl"                 , "main", DefineString.c_str());
        m_VolumeCountersCSPtr      = ShaderManager::CompileCS("slam\\scalable_kinect_fusion\\cs_volume_counters.glsl"                     , "main", DefineString.c_str());
        m_RasterizeRootVolumeVSPtr = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rasterization\\vs_rasterize_rootvolume.glsl" , "main", DefineString.c_str());
        m_RasterizeRootVolumeFSPtr = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rasterization\\fs_rasterize_rootvolume.glsl" , "main", DefineString.c_str());
        m_PointCloudVSPtr          = ShaderManager::CompileVS("slam\\scalable_kinect_fusion\\rasterization\\vs_rootgrid.glsl"             , "main", DefineString.c_str());
        m_PointCloudGSPtr          = ShaderManager::CompileGS("slam\\scalable_kinect_fusion\\rasterization\\gs_rootgrid.glsl"             , "main", DefineString.c_str());
        m_PointCloudFSPtr          = ShaderManager::CompilePS("slam\\scalable_kinect_fusion\\rasterization\\fs_rootgrid.glsl"             , "main", DefineString.c_str());
        m_PointsFullCSPtr          = ShaderManager::CompileCS("slam\\scalable_kinect_fusion\\rasterization\\cs_gather_full.glsl"          , "main", DefineString.c_str());
        m_IntegrateRootGridCSPtr   = ShaderManager::CompileCS("slam\\scalable_kinect_fusion\\integration\\cs_integrate_rootgrid.glsl"     , "main", DefineString.c_str());
        m_IntegrateLevel1GridCSPtr = ShaderManager::CompileCS("slam\\scalable_kinect_fusion\\integration\\cs_integrate_level1grid.glsl"   , "main", DefineString.c_str());
        m_IntegrateTSDFCSPtr       = ShaderManager::CompileCS("slam\\scalable_kinect_fusion\\integration\\cs_integrate_tsdf.glsl"         , "main", DefineString.c_str());
        m_FillIndirectBufferCSPtr  = ShaderManager::CompileCS("slam\\scalable_kinect_fusion\\cs_fill_indirect.glsl"                       , "main", DefineString.c_str());

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
    
	bool CScalableSLAMReconstructor::RootGridInFrustum(const glm::ivec3& rKey)
	{
		float AABB[6];

		for (int PlaneIndex = 0; PlaneIndex < 3; ++PlaneIndex)
		{
			AABB[PlaneIndex * 2] = rKey[PlaneIndex] * m_VolumeSizes[0];
			AABB[PlaneIndex * 2 + 1] = AABB[PlaneIndex * 2] + m_VolumeSizes[0];
		}

		glm::vec3 Cube[8] =
		{
			glm::vec3(AABB[0], AABB[2], AABB[4]),
			glm::vec3(AABB[0], AABB[3], AABB[4]),
			glm::vec3(AABB[0], AABB[2], AABB[5]),
			glm::vec3(AABB[0], AABB[3], AABB[5]),
			glm::vec3(AABB[1], AABB[2], AABB[4]),
			glm::vec3(AABB[1], AABB[3], AABB[4]),
			glm::vec3(AABB[1], AABB[2], AABB[5]),
			glm::vec3(AABB[1], AABB[3], AABB[5]),
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
        ContextManager::SetVertexBuffer(m_CubeMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_CubeMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);
        ContextManager::SetInputLayout(m_CubeInputLayoutPtr);
        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::SetConstantBuffer(0, m_IntrinsicsConstantBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);

        ContextManager::SetShaderVS(m_RasterizeRootVolumeVSPtr);
        ContextManager::SetShaderPS(m_RasterizeRootVolumeFSPtr);

        ContextManager::SetImageTexture(0, m_RawVertexMapPtr);

        ContextManager::SetResourceBuffer(0, m_AtomicCounterBufferPtr);
        ContextManager::SetResourceBuffer(1, m_RootVolumeInstanceBufferPtr);

        ContextManager::Barrier();
        
        const unsigned int IndexCount = m_CubeMeshPtr->GetLOD(0)->GetSurface()->GetNumberOfIndices();
        const unsigned int InstanceCount = static_cast<unsigned int>(m_RootVolumeVector.size());
        assert(InstanceCount < g_MaxVolumeInstanceCount);
        ClearBuffer(m_AtomicCounterBufferPtr, InstanceCount * sizeof(int32_t));
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
    
    void CScalableSLAMReconstructor::CreateIntegrationQueues(std::vector<uint32_t>& rVolumeQueue)
    {
        ////////////////////////////////////////////////////////////////////////////////
        // Create buffers for new volumes
        ////////////////////////////////////////////////////////////////////////////////

        ContextManager::SetConstantBuffer(0, m_IntrinsicsConstantBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);

        ContextManager::SetImageTexture(0, m_RawVertexMapPtr);

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

        ////////////////////////////////////////////////////////////////////////////////////////////////
        // Render point cloud to full volume
        ////////////////////////////////////////////////////////////////////////////////////////////////

        Performance::BeginEvent("Rasterize point cloud");

        ContextManager::SetImageTexture(1, m_FullVolumePtr);
        ContextManager::SetViewPortSet(m_FullVolumeViewPort);

        ContextManager::SetShaderVS(m_PointCloudVSPtr);
        ContextManager::SetShaderPS(m_PointCloudGSPtr);
        ContextManager::SetShaderPS(m_PointCloudFSPtr);
        ContextManager::Barrier();
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_MULTISAMPLE);
        // Just set a dummy mesh
        ContextManager::SetVertexBuffer(m_CubeMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(m_CubeMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);
        ContextManager::SetInputLayout(m_CubeInputLayoutPtr);
        ContextManager::SetTopology(STopology::PointList);
        
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetConstantBuffer(3, m_PointRasterizationBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);

        ContextManager::SetShaderVS(m_PointCloudVSPtr);
        ContextManager::SetShaderGS(m_PointCloudGSPtr);
        ContextManager::SetShaderPS(m_PointCloudFSPtr);

        ContextManager::SetShaderCS(m_PointsFullCSPtr);

        if (m_UseConservativeRasterization)
        {
            glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
        }
        for (uint32_t VolumeIndex : rVolumeQueue)
        {
            auto& rRootVolume = *m_RootVolumeVector[VolumeIndex];

            SIndirectBuffers IndirectBufferData = {};
            IndirectBufferData.m_Indexed.m_IndexCount = m_CubeMeshPtr->GetLOD(0)->GetSurface()->GetNumberOfIndices();
            BufferManager::UploadBufferData(rRootVolume.m_IndirectLevel1Buffer, &IndirectBufferData);
            BufferManager::UploadBufferData(rRootVolume.m_IndirectLevel2Buffer, &IndirectBufferData);
            
            Performance::BeginEvent("Render single point cloud");

            SPointRasterization BufferData;
            BufferData.m_Offset = rRootVolume.m_Offset;
            BufferData.m_BufferOffset = 0;
            BufferManager::UploadBufferData(m_PointRasterizationBufferPtr, &BufferData);

            ContextManager::Draw(m_DepthFrameSize.x * m_DepthFrameSize.y, 0);

            Performance::EndEvent();

            Performance::BeginEvent("Gather point cloud data");
                        
            ContextManager::SetResourceBuffer(0, rRootVolume.m_IndirectLevel1Buffer);
            ContextManager::SetResourceBuffer(1, rRootVolume.m_IndirectLevel2Buffer);
            ContextManager::SetResourceBuffer(2, rRootVolume.m_Level1QueuePtr);
            ContextManager::SetResourceBuffer(3, rRootVolume.m_Level2QueuePtr);

            const int WorkGroups = m_ReconstructionSettings.m_GridResolutions[0];
            ContextManager::Dispatch(WorkGroups, WorkGroups, WorkGroups);

            ContextManager::Barrier();
            Performance::EndEvent();
        }
        if (m_UseConservativeRasterization)
        {
            glDisable(GL_CONSERVATIVE_RASTERIZATION_NV);
        }

        Performance::EndEvent();

        ////////////////////////////////////////////////////////////////////////////////////////////////
        // Fill the indirect buffers
        ////////////////////////////////////////////////////////////////////////////////////////////////

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

        ContextManager::SetImageTexture(0, m_RawDepthBufferPtr);
        if (m_ReconstructionSettings.m_CaptureColor)
        {
            ContextManager::SetImageTexture(1, m_RawCameraFramePtr);
        }
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

	void CScalableSLAMReconstructor::UpdateRootrids()
	{
        ////////////////////////////////////////////////////////////////////////////////
        // Create all root grid volumes that are in the view frustum 
        ////////////////////////////////////////////////////////////////////////////////

		glm::vec3 BBMax = m_FrustumPoints[0];
        glm::vec3 BBMin = m_FrustumPoints[0];

		for (int i = 1; i < m_FrustumPoints.size(); ++ i)
		{
			for (int j = 0; j < 3; ++ j)
			{
				BBMax[j] = glm::max(m_FrustumPoints[i][j], BBMax[j]);
				BBMin[j] = glm::min(m_FrustumPoints[i][j], BBMin[j]);
			}
		}
		
        glm::ivec3 MaxIndex;
        glm::ivec3 MinIndex;

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
                    glm::ivec3 Key = glm::ivec3(x, y, z);
					
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

        ClearBuffer(m_AtomicCounterBufferPtr, m_RootVolumeMap.size() * sizeof(int32_t));
        
        ////////////////////////////////////////////////////////////////////////////////
        // Create vector and instance buffer for root grid volumes
        ////////////////////////////////////////////////////////////////////////////////

        m_RootVolumeVector.clear();
        
        SInstanceData* pInstanceData = static_cast<SInstanceData*>(BufferManager::MapBuffer(m_RootVolumeInstanceBufferPtr, CBuffer::Write));

		for (auto& rPair : m_RootVolumeMap)
		{
			auto& rRootGrid = rPair.second;

			rRootGrid.m_IsVisible = RootGridInFrustum(rRootGrid.m_Offset);
            
            if (rRootGrid.m_IsVisible)
            {
                m_RootVolumeVector.push_back(&rRootGrid);

                SInstanceData InstanceData;
                InstanceData.m_Offset = rRootGrid.m_Offset;
                InstanceData.m_Index = 0; // todo: remove

                *pInstanceData = InstanceData;
                ++pInstanceData;
            }
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
        GatherVolumeCounters(static_cast<unsigned int>(m_RootVolumeVector.size()), m_AtomicCounterBufferPtr, m_VolumeQueueBufferPtr, m_IndexedIndirectBufferPtr);

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
            Performance::BeginEvent("Update volume");

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

            glm::ivec3 TotalMinOffset = m_RootVolumeVector[VolumeQueue[0]]->m_Offset;
            glm::ivec3 TotalMaxOffset = TotalMinOffset;

            for (int i = 1; i < VolumeQueue.size(); ++i)
            {
                glm::ivec3 MinOffset = m_RootVolumeVector[VolumeQueue[i]]->m_Offset;
                glm::ivec3 MaxOffset = MinOffset;

                TotalMinOffset[0] = glm::min(TotalMinOffset[0], MinOffset[0]);
                TotalMinOffset[1] = glm::min(TotalMinOffset[1], MinOffset[1]);
                TotalMinOffset[2] = glm::min(TotalMinOffset[2], MinOffset[2]);

                TotalMaxOffset[0] = glm::max(TotalMaxOffset[0], MaxOffset[0]);
                TotalMaxOffset[1] = glm::max(TotalMaxOffset[1], MaxOffset[1]);
                TotalMaxOffset[2] = glm::max(TotalMaxOffset[2], MaxOffset[2]);
            }

            const int CurrentWidth = m_VolumeBuffers.m_RootVolumeTotalWidth / 2;

            if (TotalMinOffset[0] <= -CurrentWidth || TotalMinOffset[1] <= -CurrentWidth || TotalMinOffset[2] <= -CurrentWidth ||
                TotalMaxOffset[0] >   CurrentWidth || TotalMaxOffset[1] >   CurrentWidth || TotalMaxOffset[2] >   CurrentWidth)
            {
                // TODO: resize buffer
                assert(false);
            }

            m_VolumeBuffers.m_MinOffset[0] = glm::min(TotalMinOffset[0], m_VolumeBuffers.m_MinOffset[0]);
            m_VolumeBuffers.m_MinOffset[1] = glm::min(TotalMinOffset[1], m_VolumeBuffers.m_MinOffset[1]);
            m_VolumeBuffers.m_MinOffset[2] = glm::min(TotalMinOffset[2], m_VolumeBuffers.m_MinOffset[2]);

            m_VolumeBuffers.m_MaxOffset[0] = glm::max(TotalMaxOffset[0], m_VolumeBuffers.m_MaxOffset[0]);
            m_VolumeBuffers.m_MaxOffset[1] = glm::max(TotalMaxOffset[1], m_VolumeBuffers.m_MaxOffset[1]);
            m_VolumeBuffers.m_MaxOffset[2] = glm::max(TotalMaxOffset[2], m_VolumeBuffers.m_MaxOffset[2]);

            for (uint32_t VolumeIndex : VolumeQueue)
            {
                auto& rRootVolume = *m_RootVolumeVector[VolumeIndex];
                
                // Store pool indices in root volume position buffer

                const int Width = m_VolumeBuffers.m_RootVolumeTotalWidth;
                const glm::ivec3 Offset = rRootVolume.m_Offset + Width / 2; // Offset value by half of width so it is positive
                const int Index = Offset[0] + (Offset[1] * Width) + (Offset[2] * Width * Width);

                BufferManager::UploadBufferData(m_VolumeBuffers.m_RootVolumePositionBufferPtr, &rRootVolume.m_PoolIndex, Index * sizeof(int32_t), sizeof(int32_t));
            }

            Performance::EndEvent();
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

        std::string MapFormatString = Core::CProgramParameters::GetInstance().Get("mr:slam:map_format", "rgba16f");
        assert(MapFormatString == "rgba16f" || MapFormatString == "rgba32f");
        CTexture::EFormat MapFormat = MapFormatString == "rgba16f" ? CTexture::R16G16B16A16_FLOAT : CTexture::R32G32B32A32_FLOAT;

        STextureDescriptor TextureDescriptor = {};
        
        for (int i = 0; i < m_ReconstructionSettings.m_PyramidLevelCount; ++i)
        {
            TextureDescriptor.m_NumberOfPixelsU = m_DepthFrameSize.x >> i;
            TextureDescriptor.m_NumberOfPixelsV = m_DepthFrameSize.y >> i;
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_NumberOfMipMaps = 1;
            TextureDescriptor.m_NumberOfTextures = 1;
            TextureDescriptor.m_Binding = CTexture::ShaderResource;
            TextureDescriptor.m_Access = CTexture::CPUWrite;
            TextureDescriptor.m_Usage = CTexture::GPUReadWrite;
            TextureDescriptor.m_Semantic = CTexture::UndefinedSemantic;
            TextureDescriptor.m_pFileName = 0;
            TextureDescriptor.m_pPixels = 0;
            TextureDescriptor.m_Format = CTexture::R16_UINT;

            m_SmoothDepthBufferPtr[i] = TextureManager::CreateTexture2D(TextureDescriptor);

            TextureDescriptor.m_Format = MapFormat;

            m_ReferenceVertexMapPtr[i] = TextureManager::CreateTexture2D(TextureDescriptor);
            m_ReferenceNormalMapPtr[i] = TextureManager::CreateTexture2D(TextureDescriptor);
            m_RaycastVertexMapPtr[i] = TextureManager::CreateTexture2D(TextureDescriptor);
            m_RaycastNormalMapPtr[i] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        TextureDescriptor.m_NumberOfPixelsU = m_DepthFrameSize.x;
        TextureDescriptor.m_NumberOfPixelsV = m_DepthFrameSize.y;
        TextureDescriptor.m_NumberOfPixelsW = 1;
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding = CTexture::ShaderResource;
        TextureDescriptor.m_Access = CTexture::CPUWrite;
        TextureDescriptor.m_Usage = CTexture::GPUReadWrite;
        TextureDescriptor.m_Semantic = CTexture::UndefinedSemantic;
        TextureDescriptor.m_pFileName = nullptr;
        TextureDescriptor.m_pPixels = 0;
        TextureDescriptor.m_Format = CTexture::R16_UINT;

        m_RawDepthBufferPtr = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureDescriptor.m_Format = MapFormat;

        m_RawVertexMapPtr = TextureManager::CreateTexture2D(TextureDescriptor);

		if (m_ReconstructionSettings.m_CaptureColor)
		{
			TextureDescriptor.m_NumberOfPixelsU = m_DepthFrameSize.x;
			TextureDescriptor.m_NumberOfPixelsV = m_DepthFrameSize.y;
			TextureDescriptor.m_Format = CTexture::R8G8B8A8_UBYTE;

			m_RawCameraFramePtr = TextureManager::CreateTexture2D(TextureDescriptor);
		}

        const int VolumeWidth = m_ReconstructionSettings.m_GridResolutions[0] * m_ReconstructionSettings.m_GridResolutions[1];

        TextureDescriptor.m_NumberOfPixelsU = VolumeWidth;
        TextureDescriptor.m_NumberOfPixelsV = VolumeWidth;
        TextureDescriptor.m_NumberOfPixelsW = VolumeWidth;
        TextureDescriptor.m_Binding = CTexture::ShaderResource;
        TextureDescriptor.m_Access = CTexture::CPUWrite;
        TextureDescriptor.m_Usage = CTexture::GPUReadWrite;
        TextureDescriptor.m_Semantic = CTexture::UndefinedSemantic;
        TextureDescriptor.m_Format = CTexture::R8_UINT;

        m_FullVolumePtr = TextureManager::CreateTexture3D(TextureDescriptor);

        m_EmptyFullVolumePtr = TargetSetManager::CreateEmptyTargetSet(VolumeWidth, VolumeWidth);

        m_EmptyTargetSetPtr = TargetSetManager::CreateEmptyTargetSet(m_DepthFrameSize.x, m_DepthFrameSize.y);

        Gfx::TextureManager::ClearTexture(m_FullVolumePtr);
    }
    
    // -----------------------------------------------------------------------------
    
    void CScalableSLAMReconstructor::SetupBuffers(bool _CreatePool)
    {
        const float FocalLengthX0 = m_FocalLength.x;
        const float FocalLengthY0 = m_FocalLength.y;
        const float FocalPointX0 = m_FocalPoint.x;
        const float FocalPointY0 = m_FocalPoint.y;
        
        std::vector<SIntrinsics> Intrinsics(m_ReconstructionSettings.m_PyramidLevelCount);

        for (int i = 0; i < m_ReconstructionSettings.m_PyramidLevelCount; ++ i)
        {
            const int PyramidFactor = 1 << i;

            const float FocalLengthX = FocalLengthX0 / PyramidFactor;
            const float FocalLengthY = FocalLengthY0 / PyramidFactor;
            const float FocalPointX = FocalPointX0 / PyramidFactor;
            const float FocalPointY = FocalPointY0 / PyramidFactor;

            glm::mat4 KMatrix(
                FocalLengthX, 0.0f, 0.0f, 0.0f,
                0.0f, FocalLengthY, 0.0f, 0.0f,
                FocalPointX, FocalPointY, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );

            Intrinsics[i].m_FocalPoint = glm::vec2(FocalPointX, FocalPointY);
            Intrinsics[i].m_FocalLength = glm::vec2(FocalLengthX, FocalLengthY);
            Intrinsics[i].m_InvFocalLength = glm::vec2(1.0f / FocalLengthX, 1.0f / FocalLengthY);
            Intrinsics[i].m_KMatrix = KMatrix;
            Intrinsics[i].m_InvKMatrix = glm::inverse(KMatrix);
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
        TrackingData.m_InvPoseMatrix = glm::inverse(m_PoseMatrix);

        ConstantBufferDesc.m_NumberOfBytes = sizeof(STrackingData);
        ConstantBufferDesc.m_pBytes        = &TrackingData;
        m_TrackingDataConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_pBytes = 0;
        ConstantBufferDesc.m_NumberOfBytes = 16;
        ConstantBufferDesc.m_Usage = CBuffer::GPUReadWrite;
        m_RaycastPyramidConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        
        ConstantBufferDesc.m_Usage = CBuffer::GPURead;

        ConstantBufferDesc.m_NumberOfBytes = 16;
        ConstantBufferDesc.m_pBytes = &m_ReconstructionSettings.m_DepthThreshold;
        m_BilateralFilterConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        
        SIndirectBuffers ZeroIndirectData = {};
        ConstantBufferDesc.m_Usage = CBuffer::GPURead;
        ConstantBufferDesc.m_Binding = CBuffer::ResourceBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SIndirectBuffers);
        ConstantBufferDesc.m_pBytes = &ZeroIndirectData;
        m_IndexedIndirectBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        SPointRasterization ZeroPointRasterization = {};
        ConstantBufferDesc.m_Binding = CBuffer::ConstantBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_pBytes = &ZeroPointRasterization;
        ConstantBufferDesc.m_Usage = CBuffer::GPURead;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SPointRasterization);
        m_PointRasterizationBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_Binding = CBuffer::ResourceBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SInstanceData) * g_MaxVolumeInstanceCount;
        ConstantBufferDesc.m_pBytes = nullptr;
        ConstantBufferDesc.m_Usage = CBuffer::GPURead;
        m_RootVolumeInstanceBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_Binding = CBuffer::ResourceBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = m_ReconstructionSettings.m_GridResolutions[0] * m_ReconstructionSettings.m_GridResolutions[1];
        ConstantBufferDesc.m_NumberOfBytes = ConstantBufferDesc.m_NumberOfBytes *
                                             ConstantBufferDesc.m_NumberOfBytes *
                                             ConstantBufferDesc.m_NumberOfBytes * sizeof(uint32_t);
        ConstantBufferDesc.m_pBytes = nullptr;
        ConstantBufferDesc.m_Usage = CBuffer::GPURead;
        m_VolumeQueueBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        ConstantBufferDesc.m_NumberOfBytes = sizeof(uint32_t) * g_MaxVolumeInstanceCount;
        m_AtomicCounterBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        if (_CreatePool)
        {
            const unsigned int RootVolumePositionBufferSize = g_AABB * g_AABB * g_AABB * sizeof(uint32_t);

            ConstantBufferDesc.m_NumberOfBytes = RootVolumePositionBufferSize;
            m_VolumeBuffers.m_RootVolumePositionBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
            ConstantBufferDesc.m_NumberOfBytes = g_MaxRootVolumePoolSize;
            m_VolumeBuffers.m_RootVolumePoolPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
            ConstantBufferDesc.m_NumberOfBytes = m_RootGridPoolSize;
            m_VolumeBuffers.m_RootGridPoolPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
            ConstantBufferDesc.m_NumberOfBytes = m_Level1GridPoolSize;
            m_VolumeBuffers.m_Level1PoolPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
            ConstantBufferDesc.m_NumberOfBytes = m_TSDFPoolSize;
            m_VolumeBuffers.m_TSDFPoolPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        }

        uint32_t Zero[] = { 0, 0, 0, 0 };

        ConstantBufferDesc.m_pBytes = Zero;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(uint32_t) * 4;// m_ReconstructionSettings.GRID_LEVELS;
        m_VolumeBuffers.m_PoolItemCountBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
        
        ConstantBufferDesc.m_NumberOfBytes = sizeof(int32_t) * 4;// 16 bytes = minimum
        m_VolumeIndexBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);

        SScalableRaycastConstantBuffer ScalableRaycastZero = {};

        ConstantBufferDesc.m_pBytes = &ScalableRaycastZero;
        ConstantBufferDesc.m_Binding = CBuffer::ConstantBuffer;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SScalableRaycastConstantBuffer);
        m_VolumeBuffers.m_AABBBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
    }
    
    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::SetImageSizes(glm::ivec2 _DepthFrameSize, glm::ivec2 _ColorFrameSize)
    {
        m_DepthFrameSize = _DepthFrameSize;
        m_ColorFrameSize = _ColorFrameSize;
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::SetIntrinsics(glm::vec2 _FocalLength, glm::vec2 _FocalPoint)
    {
        m_FocalLength = _FocalLength;
        m_FocalPoint = _FocalPoint;
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::SetDepthBounds(float _Min, float _Max)
    {
        m_DepthBounds = glm::vec2(_Min, _Max);
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::OnNewFrame(const uint16_t* pDepthBuffer, const char* pColorBuffer, const glm::mat4* pTransform)
    {
        BASE_UNUSED(pColorBuffer);

        const bool CaptureColor = m_ReconstructionSettings.m_CaptureColor;
        
        char* pColor = m_CameraPixels.data();

        if (m_IsTrackingPaused)
        {
            return;
        }
                
        Performance::BeginEvent("Scalable Kinect Fusion");

        Performance::BeginEvent("Data Input");

        Base::AABB2UInt TargetRect;
        TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_DepthFrameSize.x, m_DepthFrameSize.y));
        TextureManager::CopyToTexture2D(m_RawDepthBufferPtr, TargetRect, m_DepthFrameSize.x, const_cast<uint16_t*>(pDepthBuffer));

        if (CaptureColor)
        {
            TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_DepthFrameSize.x, m_DepthFrameSize.y));
            TextureManager::CopyToTexture2D(m_RawCameraFramePtr, TargetRect, m_DepthFrameSize.x, pColor);
        }

        //////////////////////////////////////////////////////////////////////////////////////
        // Create reference data
        //////////////////////////////////////////////////////////////////////////////////////

        CreateReferencePyramid();

        Performance::EndEvent();

        //////////////////////////////////////////////////////////////////////////////////////
        // Detect Planes
        //////////////////////////////////////////////////////////////////////////////////////
        
        //m_PlaneDetector.DetectPlanes(m_PoseMatrix);

        //////////////////////////////////////////////////////////////////////////////////////
        // Tracking
        //////////////////////////////////////////////////////////////////////////////////////

        if (m_IntegratedFrameCount > m_MinWeight || pTransform == nullptr)
        {
            Performance::BeginEvent("Tracking");

            glm::mat4 NewPoseMatrix = m_pTracker->Track(m_PoseMatrix,
                m_ReferenceVertexMapPtr,
                m_ReferenceNormalMapPtr,
                m_RaycastVertexMapPtr,
                m_RaycastNormalMapPtr,
                m_IntrinsicsConstantBufferPtr
            );

            m_TrackingLost = m_pTracker->IsTrackingLost();

            if (!m_TrackingLost)
            {
                m_PoseMatrix = NewPoseMatrix;

                STrackingData TrackingData;
                TrackingData.m_PoseMatrix = NewPoseMatrix;
                TrackingData.m_InvPoseMatrix = glm::inverse(NewPoseMatrix);

                BufferManager::UploadBufferData(m_TrackingDataConstantBufferPtr, &TrackingData);
            }

            Performance::EndEvent();
        }
        else
        {
            m_TrackingLost = false;

            m_PoseMatrix = *pTransform;

            STrackingData TrackingData;
            TrackingData.m_PoseMatrix = *pTransform;
            TrackingData.m_InvPoseMatrix = glm::inverse(*pTransform);

            BufferManager::UploadBufferData(m_TrackingDataConstantBufferPtr, &TrackingData);
        }

        //////////////////////////////////////////////////////////////////////////////////////
        // Store root grid count on gpu and read other pool sizes form gpu
        //////////////////////////////////////////////////////////////////////////////////////

        int* pPoolSizes = static_cast<int*>(BufferManager::MapBuffer(m_VolumeBuffers.m_PoolItemCountBufferPtr, CBuffer::EMap::ReadWrite));
        pPoolSizes[0] = m_RootVolumePoolItemCount;
        m_VolumeBuffers.m_RootGridPoolSize = m_RootVolumePoolItemCount;
        m_VolumeBuffers.m_Level1PoolSize = pPoolSizes[1];
        m_VolumeBuffers.m_TSDFPoolSize = pPoolSizes[2];
        BufferManager::UnmapBuffer(m_VolumeBuffers.m_PoolItemCountBufferPtr);
        
        const unsigned int TSDFItemSize = m_ReconstructionSettings.m_CaptureColor ? sizeof(STSDFColorPoolItem) : sizeof(STSDFPoolItem);

        if (!m_PoolFull)
        {
            if (m_VolumeBuffers.m_RootGridPoolSize * m_ReconstructionSettings.m_VoxelsPerGrid[0] * sizeof(SGridPoolItem) > m_RootGridPoolSize)
            {
                m_PoolFull = true;
                m_IsIntegrationPaused = true;
                ENGINE_CONSOLE_ERROR("Rootgrid pool is full!");
            }
            if (m_VolumeBuffers.m_Level1PoolSize * m_ReconstructionSettings.m_VoxelsPerGrid[1] * sizeof(SGridPoolItem) > m_Level1GridPoolSize)
            {
                m_PoolFull = true;
                m_IsIntegrationPaused = true;
                ENGINE_CONSOLE_ERROR("Level1 pool is full!");
            }
            if (m_VolumeBuffers.m_TSDFPoolSize * m_ReconstructionSettings.m_VoxelsPerGrid[2] * TSDFItemSize > m_TSDFPoolSize)
            {
                m_PoolFull = true;
                m_IsIntegrationPaused = true;
                ENGINE_CONSOLE_ERROR("TSDF pool buffer is full!");
            }
        }
         
        unsigned int ReconstructionSizeBytes = m_VolumeBuffers.m_RootGridPoolSize * m_ReconstructionSettings.m_VoxelsPerGrid[0] * sizeof(SGridPoolItem);
        ReconstructionSizeBytes += m_VolumeBuffers.m_Level1PoolSize * m_ReconstructionSettings.m_VoxelsPerGrid[1] * sizeof(SGridPoolItem);
        ReconstructionSizeBytes += m_VolumeBuffers.m_TSDFPoolSize * m_ReconstructionSettings.m_VoxelsPerGrid[2] * TSDFItemSize;

        m_ReconstructionSize = static_cast<float>(ReconstructionSizeBytes) / g_MegabyteSize;

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
        Performance::BeginEvent("Raycasting for tracking");

        SScalableRaycastConstantBuffer Data;

        for (int i = 0; i < 3; ++i)
        {
            Data.m_AABBMin[i] = m_VolumeBuffers.m_MinOffset[i] * m_ReconstructionSettings.m_VolumeSize;
            Data.m_AABBMax[i] = (m_VolumeBuffers.m_MaxOffset[i] + 1.0f) * m_ReconstructionSettings.m_VolumeSize;
        }

        Data.m_MinWeight = m_MinWeight;
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

        if (m_IntegratedFrameCount == m_MinWeight)
        {
            ClearMarkerStatistics();
        }
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::CreateReferencePyramid()
    {
        const int WorkGroupsX = DivUp(m_DepthFrameSize.x, g_TileSize2D);
        const int WorkGroupsY = DivUp(m_DepthFrameSize.y, g_TileSize2D);

        //////////////////////////////////////////////////////////////////////////////////////
        // Bilateral Filter
        //////////////////////////////////////////////////////////////////////////////////////

        ContextManager::Barrier();

        ContextManager::SetShaderCS(m_BilateralFilterCSPtr);
        ContextManager::SetConstantBuffer(0, m_BilateralFilterConstantBufferPtr);
        ContextManager::SetImageTexture(0, m_RawDepthBufferPtr);
        ContextManager::SetImageTexture(1, m_SmoothDepthBufferPtr[0]);
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        //////////////////////////////////////////////////////////////////////////////////////
        // Downsample depth buffer
        //////////////////////////////////////////////////////////////////////////////////////

        for (int PyramidLevel = 1; PyramidLevel < m_ReconstructionSettings.m_PyramidLevelCount; ++ PyramidLevel)
        {
            const int PyramidWorkGroupsX = DivUp(m_DepthFrameSize.x >> PyramidLevel, g_TileSize2D);
            const int PyramidWorkGroupsY = DivUp(m_DepthFrameSize.y >> PyramidLevel, g_TileSize2D);
            
            ContextManager::SetShaderCS(m_DownSampleDepthCSPtr);

            ContextManager::SetImageTexture(0, m_SmoothDepthBufferPtr[PyramidLevel - 1]);
            ContextManager::SetImageTexture(1, m_SmoothDepthBufferPtr[PyramidLevel]);
            ContextManager::Barrier();

            ContextManager::Dispatch(PyramidWorkGroupsX, PyramidWorkGroupsY, 1);
        }

        /////////////////////////////////////////////////////////////////////////////////////
        // Generate vertex map pyramid
        /////////////////////////////////////////////////////////////////////////////////////

        ContextManager::SetConstantBuffer(0, m_IntrinsicsConstantBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);

        ContextManager::SetShaderCS(m_VertexMapCSPtr);
        for (int PyramidLevel = 0; PyramidLevel < m_ReconstructionSettings.m_PyramidLevelCount; ++ PyramidLevel)
        {
            const int PyramidWorkGroupsX = DivUp(m_DepthFrameSize.x >> PyramidLevel, g_TileSize2D);
            const int PyramidWorkGroupsY = DivUp(m_DepthFrameSize.y >> PyramidLevel, g_TileSize2D);

            ContextManager::SetImageTexture(0, m_SmoothDepthBufferPtr[PyramidLevel]);
            ContextManager::SetImageTexture(1, m_ReferenceVertexMapPtr[PyramidLevel]);
            ContextManager::Barrier();
            ContextManager::Dispatch(PyramidWorkGroupsX, PyramidWorkGroupsY, 1);
        }

        /////////////////////////////////////////////////////////////////////////////////////
        // Generate raw vertex map
        /////////////////////////////////////////////////////////////////////////////////////
        
        ContextManager::SetShaderCS(m_VertexMapCSPtr);

        ContextManager::SetImageTexture(0, m_RawDepthBufferPtr);
        ContextManager::SetImageTexture(1, m_RawVertexMapPtr);
        ContextManager::Barrier();
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        /////////////////////////////////////////////////////////////////////////////////////
        // Generate normal map pyramid
        /////////////////////////////////////////////////////////////////////////////////////

        ContextManager::SetShaderCS(m_NormalMapCSPtr);
        for (int PyramidLevel = 0; PyramidLevel < m_ReconstructionSettings.m_PyramidLevelCount; ++ PyramidLevel)
        {
            const int PyramidWorkGroupsX = DivUp(m_DepthFrameSize.x >> PyramidLevel, g_TileSize2D);
            const int PyramidWorkGroupsY = DivUp(m_DepthFrameSize.y >> PyramidLevel, g_TileSize2D);
                        
            ContextManager::SetImageTexture(0, m_ReferenceVertexMapPtr[PyramidLevel]);
            ContextManager::SetImageTexture(1, m_ReferenceNormalMapPtr[PyramidLevel]);
            ContextManager::Barrier();
            ContextManager::Dispatch(PyramidWorkGroupsX, PyramidWorkGroupsY, 1);
        }
    }
      
    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::CreateRaycastPyramid()
    {
        ////////////////////////////////////////////////////////////////////////////////////
        // If the normal map is not generated from the TSDF
        // generate it here from the vertex map
        /////////////////////////////////////////////////////////////////////////////////////

        if (!m_CreateNormalsFromTSDF)
        {
            ContextManager::SetShaderCS(m_NormalMapCSPtr);
            const int WorkGroupsX = DivUp(m_DepthFrameSize.x, g_TileSize2D);
            const int WorkGroupsY = DivUp(m_DepthFrameSize.y, g_TileSize2D);

            ContextManager::SetImageTexture(0, m_RaycastVertexMapPtr[0]);
            ContextManager::SetImageTexture(1, m_RaycastNormalMapPtr[0]);
            ContextManager::Barrier();
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
        }

        /////////////////////////////////////////////////////////////////////////////////////
        // Downsample vertex and normals maps to create the pyramid
        /////////////////////////////////////////////////////////////////////////////////////

        ContextManager::SetShaderCS(m_RaycastPyramidCSPtr);

        ContextManager::SetConstantBuffer(0, m_RaycastPyramidConstantBufferPtr);
        
        for (int PyramidLevel = 1; PyramidLevel < m_ReconstructionSettings.m_PyramidLevelCount; ++PyramidLevel)
        {
            const int WorkGroupsX = DivUp(m_DepthFrameSize.x >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = DivUp(m_DepthFrameSize.y >> PyramidLevel, g_TileSize2D);

            float Normalized = 0.0f;
            BufferManager::UploadBufferData(m_RaycastPyramidConstantBufferPtr, &Normalized);
            
            ContextManager::Barrier();
            ContextManager::SetImageTexture(0, m_RaycastVertexMapPtr[PyramidLevel - 1]);
            ContextManager::SetImageTexture(1, m_RaycastVertexMapPtr[PyramidLevel]);
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

            Normalized = 1.0f;
            BufferManager::UploadBufferData(m_RaycastPyramidConstantBufferPtr, &Normalized);

            ContextManager::Barrier();
            ContextManager::SetImageTexture(0, m_RaycastNormalMapPtr[PyramidLevel - 1]);
            ContextManager::SetImageTexture(1, m_RaycastNormalMapPtr[PyramidLevel]);
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
        }
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::Raycast()
    {
        const int WorkGroupsX = DivUp(m_DepthFrameSize.x, g_TileSize2D);
        const int WorkGroupsY = DivUp(m_DepthFrameSize.y, g_TileSize2D);

        ContextManager::SetShaderCS(m_RaycastCSPtr);
        
        ContextManager::SetImageTexture(1, m_RaycastVertexMapPtr[0]);
        ContextManager::SetImageTexture(2, m_RaycastNormalMapPtr[0]);

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

    void CScalableSLAMReconstructor::ClearBuffer(CBufferPtr BufferPtr)
    {
        assert(BufferPtr.IsValid());

        ClearBuffer(BufferPtr, BufferPtr->GetNumberOfBytes());
    }
    
    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::ClearBuffer(CBufferPtr BufferPtr, size_t Size)
    {
        if (Size > m_ClearVector.size())
        {
            m_ClearVector.resize(Size, 0);
        }
        
        assert(Size > 0);
        assert(BufferPtr.IsValid());

        BufferManager::UploadBufferData(BufferPtr, m_ClearVector.data());
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::ClearPool()
    {
        const uint32_t DataSize = g_MegabyteSize / 4;

        std::vector<int> Data(g_MegabyteSize / sizeof(int));
        std::memset(Data.data(), -1, DataSize);

        for (unsigned int i = 0; i < g_MaxRootVolumePoolSize / g_MegabyteSize; ++ i)
        {
            BufferManager::UploadBufferData(m_VolumeBuffers.m_RootVolumePoolPtr, Data.data(), i * DataSize, DataSize);
        }
        for (unsigned int i = 0; i < m_RootGridPoolSize / g_MegabyteSize; ++ i)
        {
            BufferManager::UploadBufferData(m_VolumeBuffers.m_RootGridPoolPtr, Data.data(), i * DataSize, DataSize);
        }
        for (unsigned int i = 0; i < m_Level1GridPoolSize / g_MegabyteSize; ++ i)
        {
            BufferManager::UploadBufferData(m_VolumeBuffers.m_Level1PoolPtr, Data.data(), i * DataSize, DataSize);
        }

        BufferManager::UploadBufferData(m_VolumeBuffers.m_RootVolumePositionBufferPtr, Data.data());

        uint32_t Zero[] = { 0, 0, 0, 0 };
        BufferManager::UploadBufferData(m_VolumeBuffers.m_PoolItemCountBufferPtr, &Zero);

        std::memset(Data.data(), 0, DataSize);
        for (unsigned int i = 0; i < m_TSDFPoolSize / g_MegabyteSize; ++ i)
        {
            BufferManager::UploadBufferData(m_VolumeBuffers.m_TSDFPoolPtr, Data.data(), i * DataSize, DataSize);
        }
    }

    // -----------------------------------------------------------------------------

    void CScalableSLAMReconstructor::ClearMarkerStatistics()
    {
        Performance::ResetEventStatistics("Rasterize point cloud");
        Performance::ResetEventStatistics("Render single point cloud");
        Performance::ResetEventStatistics("Gather point cloud data");
        Performance::ResetEventStatistics("Fill indirect buffers");
        Performance::ResetEventStatistics("Fill root grids");
        Performance::ResetEventStatistics("Fill internal grids");
        Performance::ResetEventStatistics("Compute new TSDF");
        Performance::ResetEventStatistics("Check Root Volumes");
        Performance::ResetEventStatistics("Update volume");
        Performance::ResetEventStatistics("Create integration queues");
        Performance::ResetEventStatistics("Integrate hierarchy");
        Performance::ResetEventStatistics("Scalable Kinect Fusion");
        Performance::ResetEventStatistics("Data Input");
        Performance::ResetEventStatistics("Tracking");
        Performance::ResetEventStatistics("TSDF Integration and Raycasting");
        Performance::ResetEventStatistics("Updating root grid");
        Performance::ResetEventStatistics("Raycasting for tracking");
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
		SetupBuffers(false);
		SetupShaders();

        ClearPool();

        ClearMarkerStatistics();
    }

    // -----------------------------------------------------------------------------

    CScalableSLAMReconstructor::CScalableSLAMReconstructor(const SReconstructionSettings* pReconstructionSettings)
    {
        m_DepthFrameSize = glm::ivec2(0);
        m_ColorFrameSize = glm::ivec2(0);
        m_FocalLength = glm::vec2(0.0f);
        m_FocalPoint = glm::vec2(0.0f);
        m_DepthBounds = glm::vec2(0.0f);

        if (pReconstructionSettings != nullptr)
        {
            assert(pReconstructionSettings->m_IsScalable);
            m_ReconstructionSettings = *pReconstructionSettings;
        }
        else
        {
            SReconstructionSettings::SetDefaultSettings(m_ReconstructionSettings);
        }
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

    glm::mat4 CScalableSLAMReconstructor::GetPoseMatrix() const
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

    Gfx::CTexturePtr CScalableSLAMReconstructor::GetVertexMap()
    {
        return m_RawVertexMapPtr;
    }

    // -----------------------------------------------------------------------------

    Gfx::CTexturePtr CScalableSLAMReconstructor::GetNormalMap()
    {
        return m_RaycastNormalMapPtr[0];
    }

    // -----------------------------------------------------------------------------

    glm::ivec2 CScalableSLAMReconstructor::GetDepthImageSize()
    {
        return m_DepthFrameSize;
    }
    
    // -----------------------------------------------------------------------------

    float CScalableSLAMReconstructor::GetReconstructionSize()
    {
        return m_ReconstructionSize;
    }
} // namespace MR
