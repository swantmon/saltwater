
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

#include "mr/mr_slam_reconstructor.h"
#include "mr/mr_rgbd_camera_control.h"
#include "mr/mr_kinect_control.h"
#include "mr/mr_realsense_control.h"

#include <limits>
#include <memory>
#include <sstream>

using namespace MR;
using namespace Gfx;

namespace
{
    const Base::Float3 g_InitialCameraPosition = Base::Float3(0.5f, 0.5f, 1.5f);
    const Base::Float3 g_InitialCameraRotation = Base::Float3(3.14f, 0.0f, 0.0f);
    
    const float g_EpsilonDistance = 0.1f;
    const float g_EpsilonAngle = 0.4f;


    const int g_ICPValueCount = 27;

    const unsigned int g_TileSize1D = 512;
    const unsigned int g_TileSize2D = 16;
    const unsigned int g_TileSize3D = 8;

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

    struct SDrawCallBufferData
    {
        Base::Float4x4 m_WorldMatrix;
    };    
    
} // namespace

namespace MR
{
    using namespace Base;
    
    // -----------------------------------------------------------------------------

    CSLAMReconstructor::CSLAMReconstructor(const ReconstructionSettings* pReconstructionSettings)
    {
        if (pReconstructionSettings != nullptr)
        {
            m_ReconstructionSettings = *pReconstructionSettings;
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
        return m_TrackingLost;
    }

    // -----------------------------------------------------------------------------

    Float4x4 CSLAMReconstructor::GetPoseMatrix() const
    {
        return m_PoseMatrix;
    }

    // -----------------------------------------------------------------------------

    Gfx::CTexture3DPtr CSLAMReconstructor::GetVolume()
    {
        return m_Volume;
    }

    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::Start()
    {
        m_pRGBDCameraControl.reset(new MR::CKinectControl);
        BASE_CONSOLE_INFO("Using Kinect for SLAM");

        m_DepthPixels = std::vector<unsigned short>(m_pRGBDCameraControl->GetDepthPixelCount());

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

        m_IntegratedDepthFrameCount = 0;
        m_FrameCount = 0;
        m_TrackingLost = true;

        SetupShaders();
        SetupTextures();
        SetupBuffers();
    }

    // -----------------------------------------------------------------------------
    
    void CSLAMReconstructor::Exit()
    {        
        m_CSMirrorDepth = 0;
        m_CSBilateralFilter = 0;
        m_CSVertexMap = 0;
        m_CSNormalMap = 0;
        m_CSDownSampleDepth = 0;
        m_CSVolumeIntegration = 0;
        m_CSRaycast = 0;
        m_CSRaycastPyramid = 0;
        m_CSDetermineSummands = 0;
        m_CSReduceSum = 0;
        m_CSClearVolume = 0;

        m_RawDepthBuffer = 0;

        for (int i = 0; i < m_ReconstructionSettings.m_PyramidLevelCount; ++ i)
        {
            m_SmoothDepthBuffer[i] = 0;
            m_ReferenceVertexMap[i] = 0;
            m_ReferenceNormalMap[i] = 0;
            m_RaycastVertexMap[i] = 0;
            m_RaycastNormalMap[i] = 0;
        }

        m_IntrinsicsConstantBuffer = 0;
        m_TrackingDataConstantBuffer = 0;
        m_RaycastPyramidConstantBuffer = 0;
        m_ICPBuffer = 0;
        m_ICPSummationConstantBuffer = 0;
        m_IncPoseMatrixConstantBuffer = 0;
        m_BilateralFilterConstantBuffer = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CSLAMReconstructor::SetupShaders()
    {
        const int SummandsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth(), g_TileSize2D);
        const int SummandsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

        const int Summands = SummandsX * SummandsY;
        const float SummandsLog2 = Log2(static_cast<float>(Summands));
        const int SummandsPOT = 1 << (static_cast<int>(SummandsLog2) + 1);
        
        const float VoxelSize = m_ReconstructionSettings.m_VolumeSize / m_ReconstructionSettings.m_VolumeResolution;

        std::stringstream DefineStream;

        DefineStream
            << "#define PYRAMID_LEVELS "             << m_ReconstructionSettings.m_PyramidLevelCount    << " \n"
            << "#define VOLUME_RESOLUTION "          << m_ReconstructionSettings.m_VolumeResolution     << " \n"
            << "#define VOXEL_SIZE "                 << VoxelSize                                       << " \n"
            << "#define VOLUME_SIZE "                << m_ReconstructionSettings.m_VolumeSize           << " \n"
            << "#define DEPTH_IMAGE_WIDTH "          << m_pRGBDCameraControl->GetDepthWidth()           << " \n"
            << "#define DEPTH_IMAGE_HEIGHT "         << m_pRGBDCameraControl->GetDepthHeight()          << " \n"
            << "#define TILE_SIZE1D "                << g_TileSize1D                                    << " \n"
            << "#define TILE_SIZE2D "                << g_TileSize2D                                    << " \n"
            << "#define TILE_SIZE3D "                << g_TileSize3D                                    << " \n"
            << "#define INT16_MAX "                  << std::numeric_limits<int16_t>::max()             << " \n"
            << "#define TRUNCATED_DISTANCE "         << m_ReconstructionSettings.m_TruncatedDistance    << " \n"
            << "#define MAX_INTEGRATION_WEIGHT "     << m_ReconstructionSettings.m_MaxIntegrationWeight << " \n"
            << "#define EPSILON_DISTANCE "           << g_EpsilonDistance                               << " \n"
            << "#define EPSILON_ANGLE "              << g_EpsilonAngle                                  << " \n"
            << "#define ICP_VALUE_COUNT "            << g_ICPValueCount                                 << " \n"
            << "#define REDUCTION_SHADER_COUNT "     << SummandsPOT / 2                                 << " \n"
            << "#define ICP_SUMMAND_COUNT "          << Summands                                        << " \n";

        std::string DefineString = DefineStream.str();
        
        m_CSMirrorDepth       = ShaderManager::CompileCS("kinect_fusion\\cs_mirror_depth.glsl"      , "main", DefineString.c_str());
        m_CSBilateralFilter   = ShaderManager::CompileCS("kinect_fusion\\cs_bilateral_filter.glsl"  , "main", DefineString.c_str());
        m_CSVertexMap         = ShaderManager::CompileCS("kinect_fusion\\cs_vertex_map.glsl"        , "main", DefineString.c_str());
        m_CSNormalMap         = ShaderManager::CompileCS("kinect_fusion\\cs_normal_map.glsl"        , "main", DefineString.c_str());
        m_CSDownSampleDepth   = ShaderManager::CompileCS("kinect_fusion\\cs_downsample_depth.glsl"  , "main", DefineString.c_str());
        m_CSVolumeIntegration = ShaderManager::CompileCS("kinect_fusion\\cs_integrate_volume.glsl"  , "main", DefineString.c_str());
        m_CSRaycast           = ShaderManager::CompileCS("kinect_fusion\\cs_raycast.glsl"           , "main", DefineString.c_str());
        m_CSRaycastPyramid    = ShaderManager::CompileCS("kinect_fusion\\cs_raycast_pyramid.glsl"   , "main", DefineString.c_str());
        m_CSDetermineSummands = ShaderManager::CompileCS("kinect_fusion\\cs_determine_summands.glsl", "main", DefineString.c_str());
        m_CSReduceSum         = ShaderManager::CompileCS("kinect_fusion\\cs_reduce_sum.glsl"        , "main", DefineString.c_str());
        m_CSClearVolume       = ShaderManager::CompileCS("kinect_fusion\\cs_clear_volume.glsl"      , "main", DefineString.c_str());
    }
    
    // -----------------------------------------------------------------------------
    
    void CSLAMReconstructor::SetupTextures()
    {
        m_SmoothDepthBuffer.resize(m_ReconstructionSettings.m_PyramidLevelCount);
        m_ReferenceVertexMap.resize(m_ReconstructionSettings.m_PyramidLevelCount);
        m_ReferenceNormalMap.resize(m_ReconstructionSettings.m_PyramidLevelCount);
        m_RaycastVertexMap.resize(m_ReconstructionSettings.m_PyramidLevelCount);
        m_RaycastNormalMap.resize(m_ReconstructionSettings.m_PyramidLevelCount);

        STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU = m_pRGBDCameraControl->GetDepthWidth();
        TextureDescriptor.m_NumberOfPixelsV = m_pRGBDCameraControl->GetDepthHeight();
        TextureDescriptor.m_NumberOfPixelsW = 1;
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding = CTextureBase::ShaderResource;
        TextureDescriptor.m_Access = CTextureBase::CPUWrite;        
        TextureDescriptor.m_Usage = CTextureBase::GPUReadWrite;
        TextureDescriptor.m_Semantic = CTextureBase::Diffuse;
        TextureDescriptor.m_pFileName = 0;
        TextureDescriptor.m_pPixels = 0;
        TextureDescriptor.m_Format = CTextureBase::R16_UINT;
        
        m_RawDepthBuffer = TextureManager::CreateTexture2D(TextureDescriptor);

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

            m_SmoothDepthBuffer[i] = TextureManager::CreateTexture2D(TextureDescriptor);

            TextureDescriptor.m_Format = CTextureBase::R32G32B32A32_FLOAT;

            m_ReferenceVertexMap[i] = TextureManager::CreateTexture2D(TextureDescriptor);
            m_ReferenceNormalMap[i] = TextureManager::CreateTexture2D(TextureDescriptor);
            m_RaycastVertexMap[i] = TextureManager::CreateTexture2D(TextureDescriptor);
            m_RaycastNormalMap[i] = TextureManager::CreateTexture2D(TextureDescriptor);
        }
        
        TextureDescriptor.m_NumberOfPixelsU = m_ReconstructionSettings.m_VolumeResolution;
        TextureDescriptor.m_NumberOfPixelsV = m_ReconstructionSettings.m_VolumeResolution;
        TextureDescriptor.m_NumberOfPixelsW = m_ReconstructionSettings.m_VolumeResolution;
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding = CTextureBase::ShaderResource;
        TextureDescriptor.m_Access = CTextureBase::CPUWrite;
        TextureDescriptor.m_Usage = CTextureBase::GPUReadWrite;
        TextureDescriptor.m_Semantic = CTextureBase::UndefinedSemantic;
        TextureDescriptor.m_pFileName = 0;
        TextureDescriptor.m_pPixels = 0;
        TextureDescriptor.m_Format = CTextureBase::R16G16_INT;

        m_Volume = TextureManager::CreateTexture3D(TextureDescriptor);
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

        SBufferDescriptor ConstantBufferDesc;

        ConstantBufferDesc.m_Stride = 0;
        ConstantBufferDesc.m_Usage = CBuffer::EUsage::GPURead;
        ConstantBufferDesc.m_Binding = CBuffer::ConstantBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SIntrinsics) * m_ReconstructionSettings.m_PyramidLevelCount;
        ConstantBufferDesc.m_pBytes = Intrinsics.data();
        ConstantBufferDesc.m_pClassKey = 0;

        m_IntrinsicsConstantBuffer = BufferManager::CreateBuffer(ConstantBufferDesc);
        
        STrackingData TrackingData;
        TrackingData.m_PoseMatrix = m_PoseMatrix;
        TrackingData.m_InvPoseMatrix = m_PoseMatrix.GetInverted();

        ConstantBufferDesc.m_NumberOfBytes = sizeof(STrackingData);
        ConstantBufferDesc.m_pBytes        = &TrackingData;
        m_TrackingDataConstantBuffer = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_pBytes = 0;
        ConstantBufferDesc.m_NumberOfBytes = 16;
        ConstantBufferDesc.m_Usage = CBuffer::GPUReadWrite;
        m_RaycastPyramidConstantBuffer = BufferManager::CreateBuffer(ConstantBufferDesc);
        
        ConstantBufferDesc.m_Usage = CBuffer::GPURead;
        ConstantBufferDesc.m_NumberOfBytes = 16;
        m_ICPSummationConstantBuffer = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_NumberOfBytes = sizeof(SIncBuffer);
        m_IncPoseMatrixConstantBuffer = BufferManager::CreateBuffer(ConstantBufferDesc);

        ConstantBufferDesc.m_NumberOfBytes = 16;
        ConstantBufferDesc.m_pBytes = &m_ReconstructionSettings.m_DepthThreshold;
        m_BilateralFilterConstantBuffer = BufferManager::CreateBuffer(ConstantBufferDesc);

        const int ICPRowCount = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth(), g_TileSize2D) *
            GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

        ConstantBufferDesc.m_Usage = CBuffer::GPUToCPU;
        ConstantBufferDesc.m_Binding = CBuffer::ResourceBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPURead;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(float) * ICPRowCount * g_ICPValueCount;
        ConstantBufferDesc.m_pBytes = nullptr;
        m_ICPBuffer = BufferManager::CreateBuffer(ConstantBufferDesc);
    }

    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::CreateReferencePyramid()
    {
        const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth(), g_TileSize2D);
        const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

        //////////////////////////////////////////////////////////////////////////////////////
        // Bilateral Filter
        //////////////////////////////////////////////////////////////////////////////////////

        //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        ContextManager::SetShaderCS(m_CSBilateralFilter);
        ContextManager::SetConstantBuffer(0, m_BilateralFilterConstantBuffer);
        ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_RawDepthBuffer));
        ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_SmoothDepthBuffer[0]));
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        //////////////////////////////////////////////////////////////////////////////////////
        // Downsample depth buffer
        //////////////////////////////////////////////////////////////////////////////////////

        for (int PyramidLevel = 1; PyramidLevel < m_ReconstructionSettings.m_PyramidLevelCount; ++ PyramidLevel)
        {
            const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);
            
            ContextManager::SetShaderCS(m_CSDownSampleDepth);

            ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(m_SmoothDepthBuffer[PyramidLevel - 1]));
            ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::ESampler::MinMagMipLinearClamp));

            ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_SmoothDepthBuffer[PyramidLevel]));
            //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            //glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
        }

        /////////////////////////////////////////////////////////////////////////////////////
        // Generate vertex and normal map
        /////////////////////////////////////////////////////////////////////////////////////

        ContextManager::SetConstantBuffer(0, m_IntrinsicsConstantBuffer);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBuffer);

        for (int PyramidLevel = 0; PyramidLevel < m_ReconstructionSettings.m_PyramidLevelCount; ++ PyramidLevel)
        {
            const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);

            ContextManager::SetShaderCS(m_CSVertexMap);
            ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_SmoothDepthBuffer[PyramidLevel]));
            ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_ReferenceVertexMap[PyramidLevel]));
            //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);			
        }

        for (int PyramidLevel = 0; PyramidLevel < m_ReconstructionSettings.m_PyramidLevelCount; ++ PyramidLevel)
        {
            const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);

            ContextManager::SetShaderCS(m_CSNormalMap);
            ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_ReferenceVertexMap[PyramidLevel]));
            ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_ReferenceNormalMap[PyramidLevel]));
            //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
        }
    }

    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::PerformTracking()
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

    void CSLAMReconstructor::DetermineSummands(int PyramidLevel, const Float4x4& rIncPoseMatrix)
    {
        const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
        const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);
        
        SIncBuffer TrackingData;
        TrackingData.m_PoseMatrix = rIncPoseMatrix;
        TrackingData.m_InvPoseMatrix = rIncPoseMatrix.GetInverted();
        TrackingData.m_PyramidLevel = PyramidLevel;
        
        BufferManager::UploadConstantBufferData(m_IncPoseMatrixConstantBuffer, &TrackingData);

        ContextManager::SetShaderCS(m_CSDetermineSummands);
        ContextManager::SetResourceBuffer(0, m_ICPBuffer);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBuffer);
        ContextManager::SetConstantBuffer(2, m_IncPoseMatrixConstantBuffer);
        
        ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_ReferenceVertexMap[PyramidLevel]));
        ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_ReferenceNormalMap[PyramidLevel]));
        ContextManager::SetImageTexture(2, static_cast<CTextureBasePtr>(m_RaycastVertexMap[PyramidLevel]));
        ContextManager::SetImageTexture(3, static_cast<CTextureBasePtr>(m_RaycastNormalMap[PyramidLevel]));

        //glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
    }

    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::ReduceSum(int PyramidLevel)
    {
        const int SummandsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
        const int SummandsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);

        const int Summands = SummandsX * SummandsY;
        const float SummandsLog2 = Log2(static_cast<float>(Summands));
        const int SummandsPOT = 1 << (static_cast<int>(SummandsLog2) + 1);
        
        Base::Int2 BufferData;
        BufferData[0] = Summands / 2;
        BufferData[1] = SummandsPOT / 2;

        BufferManager::UploadConstantBufferData(m_ICPSummationConstantBuffer, &BufferData);

        ContextManager::SetShaderCS(m_CSReduceSum);
        ContextManager::SetResourceBuffer(0, m_ICPBuffer);
        ContextManager::SetConstantBuffer(2, m_ICPSummationConstantBuffer);
        //glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        ContextManager::Dispatch(1, g_ICPValueCount, 1);
    }

    // -----------------------------------------------------------------------------

    bool CSLAMReconstructor::CalculatePoseMatrix(Float4x4& rIncPoseMatrix)
    {
        typedef double Scalar;

        Scalar A[36];
        Scalar b[6];
        
        float ICPValues[g_ICPValueCount];
        void* pICPBuffer = BufferManager::MapConstantBufferRange(m_ICPBuffer, CBuffer::EMap::Read, g_ICPValueCount * sizeof(float));
        memcpy(ICPValues, pICPBuffer, sizeof(ICPValues[0]) * g_ICPValueCount);

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
        BufferManager::UnmapConstantBuffer(m_ICPBuffer);

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
        
        if (std::isnan(Det) || abs(Det) < 1e-2)
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

    void CSLAMReconstructor::Integrate()
    {
        const int WorkGroups = GetWorkGroupCount(m_ReconstructionSettings.m_VolumeResolution, g_TileSize3D);

        ContextManager::SetShaderCS(m_CSVolumeIntegration);

        ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_Volume));
        ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_RawDepthBuffer));

        ContextManager::SetConstantBuffer(0, m_IntrinsicsConstantBuffer);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBuffer);
        
        //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        ContextManager::Dispatch(WorkGroups, WorkGroups, 1);
    }

    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::CreateRaycastPyramid()
    {
        ContextManager::SetShaderCS(m_CSRaycastPyramid);

        ContextManager::SetConstantBuffer(0, m_RaycastPyramidConstantBuffer);
        
        for (int PyramidLevel = 1; PyramidLevel < m_ReconstructionSettings.m_PyramidLevelCount; ++PyramidLevel)
        {
            const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);

            float Normalized = 0.0f;
            BufferManager::UploadConstantBufferData(m_RaycastPyramidConstantBuffer, &Normalized);
            
            //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_RaycastVertexMap[PyramidLevel - 1]));
            ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_RaycastVertexMap[PyramidLevel]));
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

            Normalized = 1.0f;
            BufferManager::UploadConstantBufferData(m_RaycastPyramidConstantBuffer, &Normalized);

            //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_RaycastNormalMap[PyramidLevel - 1]));
            ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_RaycastNormalMap[PyramidLevel]));
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
        }
    }

    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::Raycast()
    {
        const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth(), g_TileSize2D);
        const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

        ContextManager::SetShaderCS(m_CSRaycast);

        ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(m_Volume));
        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::ESampler::MinMagMipLinearClamp));

        ContextManager::SetImageTexture(1, static_cast<CTextureBasePtr>(m_RaycastVertexMap[0]));
        ContextManager::SetImageTexture(2, static_cast<CTextureBasePtr>(m_RaycastNormalMap[0]));

        ContextManager::SetConstantBuffer(0, m_IntrinsicsConstantBuffer);
        ContextManager::SetConstantBuffer(1, m_TrackingDataConstantBuffer);
        
        //glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);
    }

    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::Update()
    {        
        Performance::BeginEvent("Kinect Fusion");

        if (m_pRGBDCameraControl->GetDepthBuffer(m_DepthPixels.data()))
        {
            Performance::BeginEvent("Data Input");

            STextureDescriptor TextureDescriptor;

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
            TextureDescriptor.m_pPixels = m_DepthPixels.data();
            TextureDescriptor.m_Format = CTextureBase::R16_UINT;

            m_RawDepthBuffer = TextureManager::CreateTexture2D(TextureDescriptor);
            
            //////////////////////////////////////////////////////////////////////////////////////
            // Mirror depth data
            //////////////////////////////////////////////////////////////////////////////////////

            const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() / 2, g_TileSize2D);
            const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

            ContextManager::SetShaderCS(m_CSMirrorDepth);
            ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_RawDepthBuffer));
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

            //////////////////////////////////////////////////////////////////////////////////////
            // Create reference data
            //////////////////////////////////////////////////////////////////////////////////////

            CreateReferencePyramid();

            Performance::EndEvent();

            //////////////////////////////////////////////////////////////////////////////////////
            // Tracking
            //////////////////////////////////////////////////////////////////////////////////////

            if (m_IntegratedDepthFrameCount > 0)
            {
                Performance::BeginEvent("Tracking");

                PerformTracking();

                STrackingData TrackingData;
                TrackingData.m_PoseMatrix = m_PoseMatrix;
                TrackingData.m_InvPoseMatrix = m_PoseMatrix.GetInverted();
                
                BufferManager::UploadConstantBufferData(m_TrackingDataConstantBuffer, &TrackingData);
                
                Performance::EndEvent();
            }

            //////////////////////////////////////////////////////////////////////////////////////
            // Integrate and raycast pyramid
            //////////////////////////////////////////////////////////////////////////////////////

            Performance::BeginEvent("TSDF Integration and Raycasting");

            Integrate();
            Raycast();
            CreateRaycastPyramid();

            Performance::EndEvent();

            ++m_IntegratedDepthFrameCount;
            ++m_FrameCount;
        }

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::ResetReconstruction(const ReconstructionSettings* pReconstructionSettings)
    {
        if (pReconstructionSettings != nullptr)
        {
            Exit();

            m_ReconstructionSettings = *pReconstructionSettings;
            
            SetupTextures();
            SetupBuffers();
            SetupShaders();
        }

        Float4x4 PoseRotation, PoseTranslation;

        PoseRotation.SetRotation(g_InitialCameraRotation[0], g_InitialCameraRotation[1], g_InitialCameraRotation[2]);
        PoseTranslation.SetTranslation
        (
            g_InitialCameraPosition[0] * m_ReconstructionSettings.m_VolumeSize,
            g_InitialCameraPosition[1] * m_ReconstructionSettings.m_VolumeSize,
            g_InitialCameraPosition[2] * m_ReconstructionSettings.m_VolumeSize
        );
        m_PoseMatrix = PoseTranslation * PoseRotation;

        m_IntegratedDepthFrameCount = 0;
        m_FrameCount = 0;
        m_TrackingLost = true;

        STrackingData TrackingData;
        TrackingData.m_PoseMatrix = m_PoseMatrix;
        TrackingData.m_InvPoseMatrix = m_PoseMatrix.GetInverted();
        
        BufferManager::UploadConstantBufferData(m_TrackingDataConstantBuffer, &TrackingData);
                
        ClearVolume();
    }

    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::ClearVolume()
    {
        ContextManager::SetShaderCS(m_CSClearVolume);

        ContextManager::SetImageTexture(0, static_cast<CTextureBasePtr>(m_Volume));

        //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        const int WorkGroupSize = GetWorkGroupCount(m_ReconstructionSettings.m_VolumeResolution, g_TileSize3D);

        ContextManager::Dispatch(WorkGroupSize, WorkGroupSize, WorkGroupSize);
    }
        
    // -----------------------------------------------------------------------------

    void CSLAMReconstructor::GetReconstructionData(ReconstructionSettings& rReconstructionSettings)
    {
        rReconstructionSettings = m_ReconstructionSettings;
    }

    // -----------------------------------------------------------------------------

    int CSLAMReconstructor::GetWorkGroupCount(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }

} // namespace MR
