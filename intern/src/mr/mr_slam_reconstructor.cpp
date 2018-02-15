
#include "mr/mr_precompiled.h"

#include "base/base_include_glm.h"
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
#include "graphic/gfx_texture.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

#include "base/base_console.h"
#include "base/base_program_parameters.h"

#include "mr/mr_slam_reconstructor.h"
#include "mr/mr_rgbd_camera_control.h"
#include "mr/mr_kinect_control.h"

#include <gl/glew.h>

#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

using namespace MR;
using namespace Gfx;

namespace
{
    //*
    const glm::vec3 g_InitialCameraPosition = glm::vec3(0.5f, 0.5f, 1.5f);
    const glm::vec3 g_InitialCameraRotation = glm::vec3(3.14f, 0.0f, 0.0f);
    /*/
    const glm::vec3 g_InitialCameraPosition = glm::vec3(0.5f, 0.5f, -0.5f);
    const glm::vec3 g_InitialCameraRotation = glm::vec3(0.0f, 0.0f, 0.0f);
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

    struct SDrawCallBufferData
    {
        glm::mat4 m_WorldMatrix;
    };
    
} // namespace

namespace MR
{
    using namespace Base;
    
    // -----------------------------------------------------------------------------

    CSLAMReconstructor::CSLAMReconstructor(const SReconstructionSettings* pReconstructionSettings)
    {
        if (pReconstructionSettings != nullptr)
        {
			assert(!pReconstructionSettings->m_IsScalable);
            m_ReconstructionSettings = *pReconstructionSettings;
        }
        else
        {
            SReconstructionSettings::SetDefaultSettings(m_ReconstructionSettings);
        }
        Start();
    }
    
    // -----------------------------------------------------------------------------
    
    CSLAMReconstructor::~CSLAMReconstructor()
    {
        Exit();
    }
    
    // -----------------------------------------------------------------------------
    
    bool CSLAMReconstructor::IsTrackingLost() const
    {
        return !m_IsTrackingPaused && m_TrackingLost;
    }

    // -----------------------------------------------------------------------------

    glm::mat4 CSLAMReconstructor::GetPoseMatrix() const
    {
        return m_PoseMatrix;
    }

    // -----------------------------------------------------------------------------

    Gfx::CTexturePtr CSLAMReconstructor::GetTSDFVolume()
    {
        return m_TSDFVolumePtr;
    }

    // -----------------------------------------------------------------------------

    Gfx::CTexturePtr CSLAMReconstructor::GetColorVolume()
    {
        return m_ColorVolumePtr;
    }

    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::Start()
    {
        m_pRGBDCameraControl.reset(new MR::CKinectControl);
        BASE_CONSOLE_INFO("Using Kinect for SLAM");

        m_DepthPixels = std::vector<unsigned short>(m_pRGBDCameraControl->GetDepthPixelCount());
        m_CameraPixels = std::vector<char>(m_pRGBDCameraControl->GetDepthPixelCount());

        const float VolumeSize = m_ReconstructionSettings.m_VolumeSize;
        glm::mat4 PoseRotation, PoseTranslation;

        PoseRotation = glm::eulerAngleXYZ(g_InitialCameraRotation.x, g_InitialCameraRotation.y, g_InitialCameraRotation.z);
        PoseTranslation = glm::translate(g_InitialCameraPosition * VolumeSize);
        m_PoseMatrix = PoseTranslation * PoseRotation;

        m_IntegratedFrameCount = 0;
        m_FrameCount = 0;
        m_TrackingLost = true;
        m_IsIntegrationPaused = false;
        m_IsTrackingPaused = false;

        SetupShaders();
        SetupTextures();
        SetupBuffers();

        const int Width = m_pRGBDCameraControl->GetDepthWidth();
        const int Height = m_pRGBDCameraControl->GetDepthHeight();
        m_pTracker.reset(new CICPTracker(Width, Height, m_ReconstructionSettings));
    }

    // -----------------------------------------------------------------------------
    
    void CSLAMReconstructor::Exit()
    {
        m_BilateralFilterCSPtr = 0;
        m_VertexMapCSPtr = 0;
        m_NormalMapCSPtr = 0;
        m_DownSampleDepthCSPtr = 0;
        m_IntegrationCSPtr = 0;
        m_RaycastCSPtr = 0;
        m_RaycastPyramidCSPtr = 0;
        m_ClearVolumeCSPtr = 0;

        m_RawDepthBufferPtr = 0;
        m_RawCameraFramePtr = 0;

        for (int i = 0; i < m_ReconstructionSettings.m_PyramidLevelCount; ++ i)
        {
            m_SmoothDepthBufferPtr[i] = 0;
            m_ReferenceVertexMapPtr[i] = 0;
            m_ReferenceNormalMapPtr[i] = 0;
            m_RaycastVertexMapPtr[i] = 0;
            m_RaycastNormalMapPtr[i] = 0;
        }

        m_TSDFVolumePtr = 0;
        m_ColorVolumePtr = 0;

        m_IntrinsicsConstantBufferPtr = 0;
        m_TrackingDataConstantBufferPtr = 0;
        m_RaycastPyramidConstantBufferPtr = 0;
        m_BilateralFilterConstantBufferPtr = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CSLAMReconstructor::SetupShaders()
    {
        const int SummandsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth(), g_TileSize2D);
        const int SummandsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

        const int Summands = SummandsX * SummandsY;
        const float SummandsLog2 = glm::log2(static_cast<float>(Summands));
        const int SummandsPOT = 1 << (static_cast<int>(SummandsLog2) + 1);
        
        const float VoxelSize = m_ReconstructionSettings.m_VolumeSize / m_ReconstructionSettings.m_VolumeResolution;

        const std::string InternalFormatString = g_UseHighPrecisionMaps ? "rgba32f" : "rgba16f";

        std::stringstream DefineStream;

        DefineStream
            << "#define PYRAMID_LEVELS "         << m_ReconstructionSettings.m_PyramidLevelCount    << " \n"
            << "#define VOLUME_RESOLUTION "      << m_ReconstructionSettings.m_VolumeResolution     << " \n"
            << "#define VOXEL_SIZE "             << VoxelSize                                       << " \n"
            << "#define VOLUME_SIZE "            << m_ReconstructionSettings.m_VolumeSize           << " \n"
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
            << "#define MAP_TEXTURE_FORMAT "     << InternalFormatString                            << " \n";

        if (m_ReconstructionSettings.m_CaptureColor)
        {
            DefineStream << "#define CAPTURE_COLOR\n";
        }
        
        std::string DefineString = DefineStream.str();
        
        m_BilateralFilterCSPtr   = ShaderManager::CompileCS("slam\\kinect_fusion\\cs_bilateral_filter.glsl"  , "main", DefineString.c_str());
        m_VertexMapCSPtr         = ShaderManager::CompileCS("slam\\kinect_fusion\\cs_vertex_map.glsl"        , "main", DefineString.c_str());
        m_NormalMapCSPtr         = ShaderManager::CompileCS("slam\\kinect_fusion\\cs_normal_map.glsl"        , "main", DefineString.c_str());
        m_DownSampleDepthCSPtr   = ShaderManager::CompileCS("slam\\kinect_fusion\\cs_downsample_depth.glsl"  , "main", DefineString.c_str());
        m_IntegrationCSPtr       = ShaderManager::CompileCS("slam\\kinect_fusion\\cs_integrate.glsl"         , "main", DefineString.c_str());        
        m_RaycastCSPtr           = ShaderManager::CompileCS("slam\\kinect_fusion\\cs_raycast.glsl"           , "main", DefineString.c_str());
        m_RaycastPyramidCSPtr    = ShaderManager::CompileCS("slam\\kinect_fusion\\cs_raycast_pyramid.glsl"   , "main", DefineString.c_str());
        m_ClearVolumeCSPtr       = ShaderManager::CompileCS("slam\\kinect_fusion\\cs_clear_volume.glsl"      , "main", DefineString.c_str());
    }
    
    // -----------------------------------------------------------------------------
    
    void CSLAMReconstructor::SetupTextures()
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
            TextureDescriptor.m_Binding = CTexture::ShaderResource;
            TextureDescriptor.m_Access = CTexture::CPUWrite;
            TextureDescriptor.m_Usage = CTexture::GPUReadWrite;
            TextureDescriptor.m_Semantic = CTexture::UndefinedSemantic;
            TextureDescriptor.m_pFileName = 0;
            TextureDescriptor.m_pPixels = 0;
            TextureDescriptor.m_Format = CTexture::R16_UINT;

            m_SmoothDepthBufferPtr[i] = TextureManager::CreateTexture2D(TextureDescriptor);

            TextureDescriptor.m_Format = g_UseHighPrecisionMaps ? CTexture::R32G32B32A32_FLOAT : CTexture::R16G16B16A16_FLOAT;

            m_ReferenceVertexMapPtr[i] = TextureManager::CreateTexture2D(TextureDescriptor);
            m_ReferenceNormalMapPtr[i] = TextureManager::CreateTexture2D(TextureDescriptor);
            m_RaycastVertexMapPtr[i] = TextureManager::CreateTexture2D(TextureDescriptor);
            m_RaycastNormalMapPtr[i] = TextureManager::CreateTexture2D(TextureDescriptor);
        }
        
        TextureDescriptor.m_NumberOfPixelsU = m_ReconstructionSettings.m_VolumeResolution;
        TextureDescriptor.m_NumberOfPixelsV = m_ReconstructionSettings.m_VolumeResolution;
        TextureDescriptor.m_NumberOfPixelsW = m_ReconstructionSettings.m_VolumeResolution;
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding = CTexture::ShaderResource;
        TextureDescriptor.m_Access = CTexture::CPUWrite;
        TextureDescriptor.m_Usage = CTexture::GPUReadWrite;
        TextureDescriptor.m_Semantic = CTexture::UndefinedSemantic;
        TextureDescriptor.m_pFileName = 0;
        TextureDescriptor.m_pPixels = 0;
        TextureDescriptor.m_Format = CTexture::R16G16_FLOAT;

        m_TSDFVolumePtr = TextureManager::CreateTexture3D(TextureDescriptor);

        TextureDescriptor.m_NumberOfPixelsU = m_pRGBDCameraControl->GetDepthWidth();
        TextureDescriptor.m_NumberOfPixelsV = m_pRGBDCameraControl->GetDepthHeight();
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

        if (m_ReconstructionSettings.m_CaptureColor)
        {
            TextureDescriptor.m_NumberOfPixelsU = m_pRGBDCameraControl->GetDepthWidth();
            TextureDescriptor.m_NumberOfPixelsV = m_pRGBDCameraControl->GetDepthHeight();
            TextureDescriptor.m_Format = CTexture::R8G8B8A8_UBYTE;

            m_RawCameraFramePtr = TextureManager::CreateTexture2D(TextureDescriptor);

            TextureDescriptor.m_NumberOfPixelsU = m_ReconstructionSettings.m_VolumeResolution;
            TextureDescriptor.m_NumberOfPixelsV = m_ReconstructionSettings.m_VolumeResolution;
            TextureDescriptor.m_NumberOfPixelsW = m_ReconstructionSettings.m_VolumeResolution;
            TextureDescriptor.m_Format = CTexture::R8G8B8A8_UBYTE;

            m_ColorVolumePtr = TextureManager::CreateTexture3D(TextureDescriptor);
        }
        else
        {
            m_RawCameraFramePtr = 0;
            m_ColorVolumePtr = 0;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CSLAMReconstructor::SetupBuffers()
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

            glm::mat4 KMatrix(
                FocalLengthX, 0.0f, 0.0f, 0.0f,
                0.0f, FocalLengthY, 0.0f, 0.0f,
                FocalPointX, FocalPointY, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );

            Intrinsics[i].m_FocalPoint = glm::vec2(FocalPointX, FocalPointY);
            Intrinsics[i].m_FocalLength = glm::vec2(FocalLengthX, FocalLengthY);
            Intrinsics[i].m_InvFocalLength = 1.0f / glm::vec2(FocalLengthX, FocalLengthY);
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
    }

    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::Update()
    {
        const bool CaptureColor = m_ReconstructionSettings.m_CaptureColor;

        unsigned short* pDepth = m_DepthPixels.data();
        char* pColor = m_CameraPixels.data();

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
        TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_pRGBDCameraControl->GetDepthWidth(), m_pRGBDCameraControl->GetDepthHeight()));
        TextureManager::CopyToTexture2D(m_RawDepthBufferPtr, TargetRect, m_pRGBDCameraControl->GetDepthWidth(), pDepth);

        if (CaptureColor)
        {
            TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_pRGBDCameraControl->GetDepthWidth(), m_pRGBDCameraControl->GetDepthHeight()));
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

        //////////////////////////////////////////////////////////////////////////////////////
        // Integrate and raycast pyramid
        //////////////////////////////////////////////////////////////////////////////////////

        Performance::BeginEvent("TSDF Integration and Raycasting");

        if (!m_IsIntegrationPaused)
        {
            Integrate();            
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

    void CSLAMReconstructor::CreateReferencePyramid()
    {
        const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth(), g_TileSize2D);
        const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

        //////////////////////////////////////////////////////////////////////////////////////
        // Bilateral Filter
        //////////////////////////////////////////////////////////////////////////////////////

        ContextManager::Barrier();

        ContextManager::SetShaderCS(m_BilateralFilterCSPtr);
        ContextManager::SetConstantBuffer(0, m_BilateralFilterConstantBufferPtr);
        ContextManager::SetImageTexture(0, static_cast<CTexturePtr>(m_RawDepthBufferPtr));
        ContextManager::SetImageTexture(1, static_cast<CTexturePtr>(m_SmoothDepthBufferPtr[0]));
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        //////////////////////////////////////////////////////////////////////////////////////
        // Downsample depth buffer
        //////////////////////////////////////////////////////////////////////////////////////

        for (int PyramidLevel = 1; PyramidLevel < m_ReconstructionSettings.m_PyramidLevelCount; ++ PyramidLevel)
        {
            const int PyramidWorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int PyramidWorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);
            
            ContextManager::SetShaderCS(m_DownSampleDepthCSPtr);

            ContextManager::SetImageTexture(0, static_cast<CTexturePtr>(m_SmoothDepthBufferPtr[PyramidLevel - 1]));
            ContextManager::SetImageTexture(1, static_cast<CTexturePtr>(m_SmoothDepthBufferPtr[PyramidLevel]));
            ContextManager::Barrier();

            ContextManager::Dispatch(PyramidWorkGroupsX, PyramidWorkGroupsY, 1);
        }

        /////////////////////////////////////////////////////////////////////////////////////
        // Generate vertex and normal map
        /////////////////////////////////////////////////////////////////////////////////////

        ContextManager::SetConstantBuffer(0, m_IntrinsicsConstantBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);

        for (int PyramidLevel = 0; PyramidLevel < m_ReconstructionSettings.m_PyramidLevelCount; ++ PyramidLevel)
        {
            const int PyramidWorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int PyramidWorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);

            ContextManager::SetShaderCS(m_VertexMapCSPtr);
            ContextManager::SetImageTexture(0, static_cast<CTexturePtr>(m_SmoothDepthBufferPtr[PyramidLevel]));
            ContextManager::SetImageTexture(1, static_cast<CTexturePtr>(m_ReferenceVertexMapPtr[PyramidLevel]));
            ContextManager::Barrier();
            ContextManager::Dispatch(PyramidWorkGroupsX, PyramidWorkGroupsY, 1);
        }

        for (int PyramidLevel = 0; PyramidLevel < m_ReconstructionSettings.m_PyramidLevelCount; ++ PyramidLevel)
        {
            const int PyramidWorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int PyramidWorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);

            ContextManager::SetShaderCS(m_NormalMapCSPtr);
            ContextManager::SetImageTexture(0, static_cast<CTexturePtr>(m_ReferenceVertexMapPtr[PyramidLevel]));
            ContextManager::SetImageTexture(1, static_cast<CTexturePtr>(m_ReferenceNormalMapPtr[PyramidLevel]));
            ContextManager::Barrier();
            ContextManager::Dispatch(PyramidWorkGroupsX, PyramidWorkGroupsY, 1);
        }
    }
    
    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::Integrate()
    {
        const int WorkGroups = GetWorkGroupCount(m_ReconstructionSettings.m_VolumeResolution, g_TileSize2D);

        ContextManager::SetShaderCS(m_IntegrationCSPtr);

        ContextManager::SetImageTexture(0, static_cast<CTexturePtr>(m_TSDFVolumePtr));
        ContextManager::SetImageTexture(1, static_cast<CTexturePtr>(m_RawDepthBufferPtr));
        
        if (m_ReconstructionSettings.m_CaptureColor)
        {
            ContextManager::SetImageTexture(2, static_cast<CTexturePtr>(m_ColorVolumePtr));
            ContextManager::SetImageTexture(3, static_cast<CTexturePtr>(m_RawCameraFramePtr));
        }
        else
        {
            ContextManager::ResetImageTexture(2);
            ContextManager::ResetImageTexture(3);
        }
        
        ContextManager::SetConstantBuffer(0, m_IntrinsicsConstantBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);        
        
        ContextManager::Barrier();

        ContextManager::Dispatch(WorkGroups, WorkGroups, 1);
    }
    
    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::CreateRaycastPyramid()
    {
        ContextManager::SetShaderCS(m_RaycastPyramidCSPtr);

        ContextManager::SetConstantBuffer(0, m_RaycastPyramidConstantBufferPtr);
        
        for (int PyramidLevel = 1; PyramidLevel < m_ReconstructionSettings.m_PyramidLevelCount; ++PyramidLevel)
        {
            const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);

            float Normalized = 0.0f;
            BufferManager::UploadBufferData(m_RaycastPyramidConstantBufferPtr, &Normalized);
            
            ContextManager::Barrier();
            ContextManager::SetImageTexture(0, static_cast<CTexturePtr>(m_RaycastVertexMapPtr[PyramidLevel - 1]));
            ContextManager::SetImageTexture(1, static_cast<CTexturePtr>(m_RaycastVertexMapPtr[PyramidLevel]));
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

            Normalized = 1.0f;
            BufferManager::UploadBufferData(m_RaycastPyramidConstantBufferPtr, &Normalized);

            ContextManager::Barrier();
            ContextManager::SetImageTexture(0, static_cast<CTexturePtr>(m_RaycastNormalMapPtr[PyramidLevel - 1]));
            ContextManager::SetImageTexture(1, static_cast<CTexturePtr>(m_RaycastNormalMapPtr[PyramidLevel]));
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
        }
    }

    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::Raycast()
    {
        const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth(), g_TileSize2D);
        const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

        ContextManager::SetShaderCS(m_RaycastCSPtr);

        ContextManager::SetTexture(0, static_cast<CTexturePtr>(m_TSDFVolumePtr));
        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::ESampler::MinMagMipLinearClamp));

        ContextManager::SetImageTexture(1, static_cast<CTexturePtr>(m_RaycastVertexMapPtr[0]));
        ContextManager::SetImageTexture(2, static_cast<CTexturePtr>(m_RaycastNormalMapPtr[0]));

        ContextManager::SetConstantBuffer(0, m_IntrinsicsConstantBufferPtr);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBufferPtr);
        
        ContextManager::Barrier();

        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
    }

    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::ResetReconstruction(const SReconstructionSettings* pReconstructionSettings)
    {
        if (pReconstructionSettings != nullptr)
        {
			assert(!pReconstructionSettings->m_IsScalable);
            m_ReconstructionSettings = *pReconstructionSettings;
            
            SetupTextures();
            SetupBuffers();
            SetupShaders();
        }

        glm::mat4 PoseRotation, PoseTranslation;

        PoseRotation = glm::eulerAngleXYZ(g_InitialCameraRotation.x, g_InitialCameraRotation.y, g_InitialCameraRotation.z);
        PoseTranslation = glm::translate(g_InitialCameraPosition * m_ReconstructionSettings.m_VolumeSize);
        m_PoseMatrix = PoseTranslation * PoseRotation;

        m_IntegratedFrameCount = 0;
        m_FrameCount = 0;
        m_TrackingLost = true;

        STrackingData TrackingData;
        TrackingData.m_PoseMatrix = m_PoseMatrix;
        TrackingData.m_InvPoseMatrix = glm::inverse(m_PoseMatrix);
        
        BufferManager::UploadBufferData(m_TrackingDataConstantBufferPtr, &TrackingData);
                
        ClearVolume();
    }

    // -----------------------------------------------------------------------------
    
    void CSLAMReconstructor::PauseIntegration(bool _Paused)
    {
        m_IsIntegrationPaused = _Paused;
    }
    
    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::PauseTracking(bool _Paused)
    {
        m_IsTrackingPaused = _Paused;
    }

    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::ClearVolume()
    {
        ContextManager::SetShaderCS(m_ClearVolumeCSPtr);

        ContextManager::SetImageTexture(0, static_cast<CTexturePtr>(m_TSDFVolumePtr));

        if (m_ReconstructionSettings.m_CaptureColor)
        {
            ContextManager::SetImageTexture(1, static_cast<CTexturePtr>(m_ColorVolumePtr));
        }
        else
        {
            ContextManager::ResetImageTexture(1);
        }

        ContextManager::Barrier();

        const int WorkGroupSize = GetWorkGroupCount(m_ReconstructionSettings.m_VolumeResolution, g_TileSize3D);

        ContextManager::Dispatch(WorkGroupSize, WorkGroupSize, WorkGroupSize);
    }
        
    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::GetReconstructionSettings(SReconstructionSettings* pReconstructionSettings)
    {
        assert(pReconstructionSettings != nullptr);
        *pReconstructionSettings = m_ReconstructionSettings;
    }

    // -----------------------------------------------------------------------------

    int CSLAMReconstructor::GetWorkGroupCount(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }
} // namespace MR
