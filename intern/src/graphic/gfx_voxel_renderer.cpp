
#include "graphic/gfx_precompiled.h"

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
#include "graphic/gfx_voxel_renderer.h"

#include "base/base_console.h"
#include "gfx_native_buffer.h"
#include "gfx_native_sampler.h"
#include "gfx_native_shader.h"
#include "gfx_native_target_set.h"

#include "mr/mr_rgbd_camera_control.h"
#include "mr/mr_kinect_control.h"
#include "mr/mr_realsense_control.h"

#include <gl/glew.h>

#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

using namespace Gfx;

namespace
{
    const float g_VolumeSize = 1.0f;
    const int g_VolumeResolution = 256;
    const float g_VoxelSize = g_VolumeSize / g_VolumeResolution;

    const Base::Float3 g_InitialCameraPosition = Base::Float3(g_VolumeSize * 0.5f, g_VolumeSize * 0.5f, g_VolumeSize * 1.5f);
    const Base::Float3 g_InitialCameraRotation = Base::Float3(3.14f, 0.0f, 0.0f);

    const float g_TruncatedDistance = 30.0f;
    
    const int g_MaxIntegrationWeight = 150;

    const int g_PyramidLevelCount = 3;

    const int g_ICPIterations[g_PyramidLevelCount] = { 10, 5, 4 };

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

    struct SDrawCallBufferData
    {
        Base::Float4x4 m_WorldMatrix;
    };    
    
    class CGfxVoxelRenderer : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxVoxelRenderer)
        
    public:
        CGfxVoxelRenderer();
        ~CGfxVoxelRenderer();
        
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

    private:

        void UpdateReconstruction();

        void CreateReferencePyramid();
        void Integrate();
        void Raycast();
        void DownSample();

        void PerformTracking();

        void DetermineSummands(int PyramidLevel, const Base::Float4x4& rIncPoseMatrix);
        void ReduceSum(int PyramidLevel);
        bool CalculatePoseMatrix(Base::Float4x4& rIncPoseMatrix);

        void Draw();

        int GetWorkGroupCount(int TotalShaderCount, int WorkGroupSize);

        // Just for debugging

        void RenderReconstructionData();

        void RenderDepth();
        void RenderVertexMap(GLuint VertexMap, GLuint NormalMap);
        void RenderVolume();       

        void RenderCamera();

        void SetSphere();
        
    private:

        GLuint m_DrawCallConstantBuffer;
        GLuint m_IntrinsicsConstantBuffer;
        GLuint m_TrackingDataConstantBuffer;
        GLuint m_RaycastPyramidConstantBuffer;
        GLuint m_ICPSummationConstantBuffer;
        GLuint m_IncPoseMatrixConstantBuffer;

        CShaderPtr m_VSVisualizeDepth;
        CShaderPtr m_FSVisualizeDepth;
        CShaderPtr m_VSVisualizeVertexMap;
        CShaderPtr m_FSVisualizeVertexMap;
        CShaderPtr m_VSVisualizeVolume;
        CShaderPtr m_FSVisualizeVolume;
        CShaderPtr m_VSCamera;
        CShaderPtr m_FSCamera;
        CShaderPtr m_CSSphere;

        CShaderPtr m_CSMirrorDepth;
        CShaderPtr m_CSBilateralFilter;
        CShaderPtr m_CSVertexMap;
        CShaderPtr m_CSNormalMap;
        CShaderPtr m_CSDownSampleDepth;
        CShaderPtr m_CSVolumeIntegration;
        CShaderPtr m_CSRaycast;
        CShaderPtr m_CSRaycastPyramid;
        CShaderPtr m_CSDetermineSummands;
        CShaderPtr m_CSReduceSum;        
        CShaderPtr m_VSRaycast;
        CShaderPtr m_FSRaycast;
        
        GLuint m_RawDepthBuffer;
        GLuint m_SmoothDepthBuffer[g_PyramidLevelCount];
        GLuint m_ReferenceVertexMap[g_PyramidLevelCount];
        GLuint m_ReferenceNormalMap[g_PyramidLevelCount];

        GLuint m_RaycastVertexMap[g_PyramidLevelCount];
        GLuint m_RaycastNormalMap[g_PyramidLevelCount];

        GLuint m_Volume;

        GLuint m_ICPBuffer;

        std::unique_ptr<MR::IRGBDCameraControl> m_pRGBDCameraControl;

        Base::Float4x4 m_PoseMatrix;

        Base::Float4x4 m_VertexMapWorldMatrix;
        Base::Float4x4 m_VolumeWorldMatrix;

        std::vector<unsigned short> m_DepthPixels;

        bool m_NewDepthDataAvailable;
        int m_IntegratedDepthFrameCount;
        int m_FrameCount;

        bool m_TrackingLost;

        // just for debugging

        GLuint m_CubeMesh[2];

        GLuint m_DebugBuffer[g_PyramidLevelCount];
        GLuint m_CameraVAO;

        GLuint m_RaycastBuffer;
    };
} // namespace

namespace
{
    using namespace Base;

    CGfxVoxelRenderer::CGfxVoxelRenderer()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxVoxelRenderer::~CGfxVoxelRenderer()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnStart()
    {
        Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxVoxelRenderer::OnResize));
        
        m_pRGBDCameraControl.reset(new MR::CKinectControl());
        BASE_CONSOLE_INFO("Using Kinect for SLAM");

        m_DepthPixels = std::vector<unsigned short>(m_pRGBDCameraControl->GetDepthPixelCount());

        Float4x4 PoseRotation, PoseTranslation;

        PoseRotation.SetRotation(g_InitialCameraRotation[0], g_InitialCameraRotation[1], g_InitialCameraRotation[2]);
        PoseTranslation.SetTranslation(g_InitialCameraPosition[0], g_InitialCameraPosition[1], g_InitialCameraPosition[2]);
        m_PoseMatrix = PoseTranslation * PoseRotation;

        m_NewDepthDataAvailable = false;
        m_IntegratedDepthFrameCount = 0;
        m_FrameCount = 0;
        m_TrackingLost = true;
    }

    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnExit()
    {
        m_VSVisualizeDepth = 0;
        m_FSVisualizeDepth = 0;
        m_VSVisualizeVertexMap = 0;
        m_FSVisualizeVertexMap = 0;
        m_VSVisualizeVolume = 0;
        m_FSVisualizeVolume = 0;
        m_VSCamera = 0;
        m_FSCamera = 0;
        m_CSSphere = 0;

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
        m_VSRaycast = 0;
        m_FSRaycast = 0;        

        glDeleteTextures(1, &m_RawDepthBuffer);
        glDeleteTextures(g_PyramidLevelCount, m_SmoothDepthBuffer);
        glDeleteTextures(g_PyramidLevelCount, m_ReferenceVertexMap);
        glDeleteTextures(g_PyramidLevelCount, m_ReferenceNormalMap);
        glDeleteTextures(g_PyramidLevelCount, m_RaycastVertexMap);
        glDeleteTextures(g_PyramidLevelCount, m_RaycastNormalMap);
        glDeleteTextures(1, &m_Volume);
        glDeleteTextures(g_PyramidLevelCount, m_DebugBuffer);

        glDeleteBuffers(1, &m_DrawCallConstantBuffer);
        glDeleteBuffers(1, &m_IntrinsicsConstantBuffer);
        glDeleteBuffers(1, &m_TrackingDataConstantBuffer);
        glDeleteBuffers(1, &m_RaycastPyramidConstantBuffer);
        glDeleteBuffers(1, &m_ICPBuffer);
        glDeleteBuffers(1, &m_ICPSummationConstantBuffer);
        glDeleteBuffers(1, &m_IncPoseMatrixConstantBuffer);
        glDeleteBuffers(1, &m_RaycastBuffer);

        glDeleteVertexArrays(1, &m_CameraVAO);
        glDeleteBuffers(2, m_CubeMesh);
    }
        
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupShader()
    {
        const int SummandsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth(), g_TileSize2D);
        const int SummandsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

        const int Summands = SummandsX * SummandsY;
        const float SummandsLog2 = Log2(static_cast<float>(Summands));
        const int SummandsPOT = 1 << (static_cast<int>(SummandsLog2) + 1);
        
        std::stringstream DefineStream;

        DefineStream
            << "#define VOLUME_RESOLUTION "          << g_VolumeResolution                       << " \n"
            << "#define VOXEL_SIZE "                 << g_VoxelSize                              << " \n"
            << "#define VOLUME_SIZE "                << g_VolumeSize                             << " \n"
            << "#define DEPTH_IMAGE_WIDTH "          << m_pRGBDCameraControl->GetDepthWidth()   << " \n"
            << "#define DEPTH_IMAGE_HEIGHT "         << m_pRGBDCameraControl->GetDepthHeight()  << " \n"
            << "#define TILE_SIZE1D "                << g_TileSize1D                             << " \n"
            << "#define TILE_SIZE2D "                << g_TileSize2D                             << " \n"
            << "#define TILE_SIZE3D "                << g_TileSize3D                             << " \n"
            << "#define INT16_MAX "                  << std::numeric_limits<int16_t>::max()      << " \n"
            << "#define TRUNCATED_DISTANCE "         << g_TruncatedDistance                      << " \n"
            << "#define TRUNCATED_DISTANCE_INVERSE " << 1.0f / g_TruncatedDistance               << " \n"
            << "#define MAX_INTEGRATION_WEIGHT "     << g_MaxIntegrationWeight                   << " \n"
            << "#define EPSILON_DISTANCE "           << g_EpsilonDistance                        << " \n"
            << "#define EPSILON_ANGLE "              << g_EpsilonAngle                           << " \n"
            << "#define ICP_VALUE_COUNT "            << g_ICPValueCount                          << " \n"
            << "#define REDUCTION_SHADER_COUNT "     << SummandsPOT / 2                          << " \n"
            << "#define ICP_SUMMAND_COUNT "          << Summands                                 << " \n";

        std::string DefineString = DefineStream.str();

        m_VSVisualizeDepth = ShaderManager::CompileVS("kinect_fusion\\vs_visualize_depth.glsl", "main", DefineString.c_str());
        m_FSVisualizeDepth = ShaderManager::CompilePS("kinect_fusion\\fs_visualize_depth.glsl", "main", DefineString.c_str());
        m_VSVisualizeVertexMap = ShaderManager::CompileVS("kinect_fusion\\vs_visualize_vertex_map.glsl", "main", DefineString.c_str());
        m_FSVisualizeVertexMap = ShaderManager::CompilePS("kinect_fusion\\fs_visualize_vertex_map.glsl", "main", DefineString.c_str());
        m_VSVisualizeVolume = ShaderManager::CompileVS("kinect_fusion\\vs_visualize_volume.glsl", "main", DefineString.c_str());
        m_FSVisualizeVolume = ShaderManager::CompilePS("kinect_fusion\\fs_visualize_volume.glsl", "main", DefineString.c_str());
        m_VSCamera = ShaderManager::CompileVS("kinect_fusion\\vs_camera.glsl", "main", DefineString.c_str());
        m_FSCamera = ShaderManager::CompilePS("kinect_fusion\\fs_camera.glsl", "main", DefineString.c_str());
        m_CSSphere = ShaderManager::CompileCS("kinect_fusion\\cs_sphere.glsl", "main", DefineString.c_str());

        m_CSMirrorDepth = ShaderManager::CompileCS("kinect_fusion\\cs_mirror_depth.glsl", "main", DefineString.c_str());
        m_CSBilateralFilter = ShaderManager::CompileCS("kinect_fusion\\cs_bilateral_filter.glsl", "main", DefineString.c_str());
        m_CSVertexMap = ShaderManager::CompileCS("kinect_fusion\\cs_vertex_map.glsl", "main", DefineString.c_str());
        m_CSNormalMap = ShaderManager::CompileCS("kinect_fusion\\cs_normal_map.glsl", "main", DefineString.c_str());
        m_CSDownSampleDepth = ShaderManager::CompileCS("kinect_fusion\\cs_downsample_depth.glsl", "main", DefineString.c_str());
        m_CSVolumeIntegration = ShaderManager::CompileCS("kinect_fusion\\cs_integrate_volume.glsl", "main", DefineString.c_str());
        m_CSRaycast = ShaderManager::CompileCS("kinect_fusion\\cs_raycast.glsl", "main", DefineString.c_str());
        m_CSRaycastPyramid = ShaderManager::CompileCS("kinect_fusion\\cs_raycast_pyramid.glsl", "main", DefineString.c_str());
        m_CSDetermineSummands = ShaderManager::CompileCS("kinect_fusion\\cs_determine_summands.glsl", "main", DefineString.c_str());
        m_CSReduceSum = ShaderManager::CompileCS("kinect_fusion\\cs_reduce_sum.glsl", "main", DefineString.c_str());
        m_VSRaycast = ShaderManager::CompileVS("kinect_fusion\\vs_raycast.glsl", "main", DefineString.c_str());
        m_FSRaycast = ShaderManager::CompilePS("kinect_fusion\\fs_raycast.glsl", "main", DefineString.c_str());
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupKernels()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupRenderTargets()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupStates()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupTextures()
    {
        glCreateTextures(GL_TEXTURE_2D, 1, &m_RawDepthBuffer);
        glCreateTextures(GL_TEXTURE_2D, g_PyramidLevelCount, m_SmoothDepthBuffer);
        glCreateTextures(GL_TEXTURE_2D, g_PyramidLevelCount, m_ReferenceVertexMap);
        glCreateTextures(GL_TEXTURE_2D, g_PyramidLevelCount, m_ReferenceNormalMap);
        glCreateTextures(GL_TEXTURE_2D, g_PyramidLevelCount, m_RaycastVertexMap);
        glCreateTextures(GL_TEXTURE_2D, g_PyramidLevelCount, m_RaycastNormalMap);
        glCreateTextures(GL_TEXTURE_3D, 1, &m_Volume);

        glCreateTextures(GL_TEXTURE_2D, g_PyramidLevelCount, m_DebugBuffer);
        
        glTextureStorage2D(m_RawDepthBuffer, 1, GL_R16UI, m_pRGBDCameraControl->GetDepthWidth(), m_pRGBDCameraControl->GetDepthHeight());
        
        for (int i = 0; i < g_PyramidLevelCount; ++i)
        {
            const int Width = m_pRGBDCameraControl->GetDepthWidth() >> i;
            const int Height = m_pRGBDCameraControl->GetDepthHeight() >> i;

            glTextureStorage2D(m_SmoothDepthBuffer[i], 1, GL_R16UI, Width, Height);
            glTextureStorage2D(m_ReferenceVertexMap[i], 1, GL_RGBA32F, Width, Height);
            glTextureStorage2D(m_ReferenceNormalMap[i], 1, GL_RGBA32F, Width, Height);
            glTextureStorage2D(m_RaycastVertexMap[i], 1, GL_RGBA32F, Width, Height);
            glTextureStorage2D(m_RaycastNormalMap[i], 1, GL_RGBA32F, Width, Height);
            glTextureStorage2D(m_DebugBuffer[i], 1, GL_RGBA32F, Width, Height);
        }

        glTextureStorage3D(m_Volume, 1, GL_RG16I, g_VolumeResolution, g_VolumeResolution, g_VolumeResolution);        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupBuffers()
    {
        const float FocalLengthX0 = m_pRGBDCameraControl->GetDepthFocalLengthX();
        const float FocalLengthY0 = m_pRGBDCameraControl->GetDepthFocalLengthY();
        const float FocalPointX0 = m_pRGBDCameraControl->GetDepthFocalPointX();
        const float FocalPointY0 = m_pRGBDCameraControl->GetDepthFocalPointY();

        glCreateBuffers(1, &m_DrawCallConstantBuffer);
        glNamedBufferData(m_DrawCallConstantBuffer, sizeof(Float4x4), nullptr, GL_DYNAMIC_DRAW);

        SIntrinsics Intrinsics[g_PyramidLevelCount];

        for (int i = 0; i < g_PyramidLevelCount; ++ i)
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

        glCreateBuffers(1, &m_IntrinsicsConstantBuffer);
        glNamedBufferData(m_IntrinsicsConstantBuffer, sizeof(SIntrinsics) * g_PyramidLevelCount, Intrinsics, GL_STATIC_DRAW);
        
        STrackingData TrackingData;
        TrackingData.m_PoseMatrix = m_PoseMatrix;
        TrackingData.m_InvPoseMatrix = m_PoseMatrix.GetInverted();

        glCreateBuffers(1, &m_TrackingDataConstantBuffer);
        glNamedBufferData(m_TrackingDataConstantBuffer, sizeof(STrackingData), &TrackingData, GL_DYNAMIC_DRAW);

        glCreateBuffers(1, &m_RaycastPyramidConstantBuffer);
        glNamedBufferData(m_RaycastPyramidConstantBuffer, 16, nullptr, GL_DYNAMIC_DRAW);

        const int ICPRowCount = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth(), g_TileSize2D) *
                                GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

        glCreateBuffers(1, &m_ICPBuffer);
        glNamedBufferData(m_ICPBuffer, sizeof(float) * ICPRowCount * g_ICPValueCount, nullptr, GL_DYNAMIC_COPY);

        glCreateBuffers(1, &m_ICPSummationConstantBuffer);
        glNamedBufferData(m_ICPSummationConstantBuffer, 16, nullptr, GL_DYNAMIC_DRAW);
        
        glCreateBuffers(1, &m_IncPoseMatrixConstantBuffer);
        glNamedBufferData(m_IncPoseMatrixConstantBuffer, sizeof(Float4x4) * 2, nullptr, GL_DYNAMIC_DRAW);

        glCreateBuffers(1, &m_RaycastBuffer);
        glNamedBufferData(m_RaycastBuffer, sizeof(Float4) * 2, nullptr, GL_DYNAMIC_DRAW);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupModels()
    {
        Float3 Vertices[] =
        {
            Float3(-1.0f, -0.5f, 2.0f),
            Float3( 1.0f, -0.5f, 2.0f),
            Float3( 1.0f,  0.5f, 2.0f),
            Float3(-1.0f,  0.5f, 2.0f),
            Float3(-0.0f,  0.0f, 0.0f),
        };

        unsigned int Indices[] =
        {
            0, 1, 2,
            0, 2, 3,
            1, 4, 2,
            2, 4, 3,
            0, 4, 1,
            0, 3, 4,
        };

        GLuint m_CubeMesh[2];
        glCreateBuffers(2, m_CubeMesh);
        glNamedBufferData(m_CubeMesh[0], sizeof(Vertices), &Vertices, GL_STATIC_DRAW);
        glNamedBufferData(m_CubeMesh[1], sizeof(Indices), &Indices, GL_STATIC_DRAW);

        glCreateVertexArrays(1, &m_CameraVAO);
        glBindVertexArray(m_CameraVAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_CubeMesh[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_CubeMesh[1]);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindVertexArray(0);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnReload()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnNewMap()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnUnloadMap()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::OnResize(unsigned int _Width, unsigned int _Height)
    {
        BASE_UNUSED(_Width);
        BASE_UNUSED(_Height);
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::Update()
    {
        //////////////////////////////////////////////////////////////////////////////////////
        // Get camera Data
        //////////////////////////////////////////////////////////////////////////////////////

        Float4x4 TranslationMatrix;
        Float4x4 ScalingMatrix;
        Float4x4 RotationMatrix;

        ScalingMatrix.SetScale(0.005f);
        RotationMatrix.SetRotationX(3.14f);
        
        m_VertexMapWorldMatrix = RotationMatrix * ScalingMatrix;

        ScalingMatrix.SetScale(5.0f / static_cast<float>(g_VolumeResolution));
        TranslationMatrix.SetTranslation(0.0f, 0.0f, 0.0f);

        m_VolumeWorldMatrix = TranslationMatrix * RotationMatrix * ScalingMatrix;
        
        if (m_pRGBDCameraControl->GetDepthBuffer(m_DepthPixels.data()))
        {
            glTextureSubImage2D(m_RawDepthBuffer, 0, 0, 0,
                m_pRGBDCameraControl->GetDepthWidth(), m_pRGBDCameraControl->GetDepthHeight(),
                GL_RED_INTEGER, GL_UNSIGNED_SHORT, m_DepthPixels.data());

            //////////////////////////////////////////////////////////////////////////////////////
            // Mirror depth data
            //////////////////////////////////////////////////////////////////////////////////////

            const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() / 2, g_TileSize2D);
            const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);
            
            Gfx::ContextManager::SetShaderCS(m_CSMirrorDepth);
            glBindImageTexture(0, m_RawDepthBuffer, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R16UI);
            glDispatchCompute(WorkGroupsX, WorkGroupsY, 1);

            m_NewDepthDataAvailable = true;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::Draw()
    {
        Performance::BeginEvent("Rendering");
        
        glBindVertexArray(1); // dummy vao because opengl needs vertex data even when it does not use it

        CTargetSetPtr DefaultTargetSetPtr = TargetSetManager::GetDefaultTargetSet();
        CNativeTargetSet NativeTargetSet = *static_cast<CNativeTargetSet*>(DefaultTargetSetPtr.GetPtr());

        glBindFramebuffer(GL_FRAMEBUFFER, NativeTargetSet.m_NativeTargetSet);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);
        
        CSamplerPtr Sampler = Gfx::SamplerManager::GetSampler(Gfx::CSampler::ESampler::MinMagMipLinearClamp);
        CNativeSampler* NativeSampler = static_cast<CNativeSampler*>(Sampler.GetPtr());

        Float4* pData = static_cast<Float4*>(glMapNamedBuffer(m_RaycastBuffer, GL_WRITE_ONLY));
        m_PoseMatrix.GetTranslation((*pData)[0], (*pData)[1], (*pData)[2]);
        (*pData)[3] = 1.0f;
        (*(pData + 1)) = m_TrackingLost ? Float4(1.0f, 0.0f, 0.0f, 1.0f) : Float4(0.0f, 1.0f, 0.0f, 1.0f);
        glUnmapNamedBuffer(m_RaycastBuffer);
        
        Gfx::ContextManager::SetShaderVS(m_VSRaycast);
        Gfx::ContextManager::SetShaderPS(m_FSRaycast);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, m_Volume);
        glBindSampler(0, NativeSampler->m_NativeSampler);

        CBufferPtr FrameConstantBufferPtr = Gfx::Main::GetPerFrameConstantBuffer();
        CNativeBuffer NativeBufer = *static_cast<CNativeBuffer*>(FrameConstantBufferPtr.GetPtr());
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, NativeBufer.m_NativeBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_RaycastBuffer);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::CreateReferencePyramid()
    {
        const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth(), g_TileSize2D);
        const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

        //////////////////////////////////////////////////////////////////////////////////////
        // Bilateral Filter
        //////////////////////////////////////////////////////////////////////////////////////

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        Gfx::ContextManager::SetShaderCS(m_CSBilateralFilter);
        glBindImageTexture(0, m_RawDepthBuffer, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
        glBindImageTexture(1, m_SmoothDepthBuffer[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16UI);
        glBindImageTexture(2, m_DebugBuffer[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glDispatchCompute(WorkGroupsX, WorkGroupsY, 1);

        //////////////////////////////////////////////////////////////////////////////////////
        // Downsample depth buffer
        //////////////////////////////////////////////////////////////////////////////////////

        for (int PyramidLevel = 1; PyramidLevel < g_PyramidLevelCount; ++ PyramidLevel)
        {
            const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);
            
            Gfx::ContextManager::SetShaderCS(m_CSDownSampleDepth);

            CSamplerPtr Sampler = Gfx::SamplerManager::GetSampler(Gfx::CSampler::ESampler::MinMagMipLinearClamp);
            CNativeSampler* NativeSampler = static_cast<CNativeSampler*>(Sampler.GetPtr());
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_SmoothDepthBuffer[PyramidLevel - 1]);
            glBindSampler(0, NativeSampler->m_NativeSampler);

            glBindImageTexture(1, m_SmoothDepthBuffer[PyramidLevel], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16UI);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
            glDispatchCompute(WorkGroupsX, WorkGroupsY, 1);
        }

        /////////////////////////////////////////////////////////////////////////////////////
        // Generate vertex and normal map
        /////////////////////////////////////////////////////////////////////////////////////

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_IntrinsicsConstantBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_TrackingDataConstantBuffer);
        
        for (int PyramidLevel = 0; PyramidLevel < g_PyramidLevelCount; ++ PyramidLevel)
        {
            const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);

            Gfx::ContextManager::SetShaderCS(m_CSVertexMap);
            glBindImageTexture(0, m_SmoothDepthBuffer[PyramidLevel], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
            glBindImageTexture(1, m_ReferenceVertexMap[PyramidLevel], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            glDispatchCompute(WorkGroupsX, WorkGroupsY, 1);			
        }

        for (int PyramidLevel = 0; PyramidLevel < g_PyramidLevelCount; ++ PyramidLevel)
        {
            const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);

            Gfx::ContextManager::SetShaderCS(m_CSNormalMap);
            glBindImageTexture(0, m_ReferenceVertexMap[PyramidLevel], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, m_ReferenceNormalMap[PyramidLevel], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            glDispatchCompute(WorkGroupsX, WorkGroupsY, 1);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::PerformTracking()
    {
        Float4x4 IncPoseMatrix = m_PoseMatrix;

        for (int PyramidLevel = g_PyramidLevelCount - 1; PyramidLevel >= 0; -- PyramidLevel)
        {
            for (int Iteration = 0; Iteration < g_ICPIterations[PyramidLevel]; ++ Iteration)
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

    void CGfxVoxelRenderer::DetermineSummands(int PyramidLevel, const Float4x4& rIncPoseMatrix)
    {
        const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
        const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);
        
        STrackingData TrackingData;
        TrackingData.m_PoseMatrix = rIncPoseMatrix;
        TrackingData.m_InvPoseMatrix = rIncPoseMatrix.GetInverted();

        Float4x4* pData = static_cast<Float4x4*>(glMapNamedBufferRange(m_IncPoseMatrixConstantBuffer, 0, sizeof(Float4x4) * 2, GL_MAP_WRITE_BIT));
        memcpy(pData, &TrackingData, sizeof(STrackingData));
        glUnmapNamedBuffer(m_IncPoseMatrixConstantBuffer);

        Gfx::ContextManager::SetShaderCS(m_CSDetermineSummands);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ICPBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_TrackingDataConstantBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_IncPoseMatrixConstantBuffer);
        
        glBindImageTexture(0, m_ReferenceVertexMap[PyramidLevel], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(1, m_ReferenceNormalMap[PyramidLevel], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(2, m_RaycastVertexMap[PyramidLevel], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(3, m_RaycastNormalMap[PyramidLevel], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(4, m_DebugBuffer[PyramidLevel], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glDispatchCompute(WorkGroupsX, WorkGroupsY, 1);
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::ReduceSum(int PyramidLevel)
    {
        const int SummandsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
        const int SummandsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);

        const int Summands = SummandsX * SummandsY;
        const float SummandsLog2 = Log2(static_cast<float>(Summands));
        const int SummandsPOT = 1 << (static_cast<int>(SummandsLog2) + 1);

        int* pData = static_cast<int*>(glMapNamedBuffer(m_ICPSummationConstantBuffer, GL_WRITE_ONLY));
        *pData = Summands / 2;
        *(pData + 1) = SummandsPOT / 2;
        glUnmapNamedBuffer(m_ICPSummationConstantBuffer);

        Gfx::ContextManager::SetShaderCS(m_CSReduceSum);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ICPBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_ICPSummationConstantBuffer);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glDispatchCompute(1, g_ICPValueCount, 1);
    }

    // -----------------------------------------------------------------------------

    bool CGfxVoxelRenderer::CalculatePoseMatrix(Float4x4& rIncPoseMatrix)
    {
        double A[36];
        double b[6];

        int ValueIndex = 0;

        float* pICPBufferData = static_cast<float*>(glMapNamedBufferRange(m_ICPBuffer, 0, sizeof(float) * g_ICPValueCount, GL_MAP_READ_BIT));
        for (int i = 0; i < 6; ++ i)
        {
            for (int j = i; j < 7; ++ j)
            {
                float Value = pICPBufferData[ValueIndex++];
                
                if (j == 6)
                {
                    b[i] = Value;
                }
                else
                {
                    A[j * 6 + i] = A[i * 6 + j] = Value;
                }
            }
        }
        glUnmapNamedBuffer(m_ICPBuffer);

        double L[36];

        for (int i = 0; i < 6; ++ i)
        {
            for (int j = 0; j <= i; ++ j)
            {
                double Sum = 0.0;
                for (int k = 0; k < j; ++ k)
                {
                    Sum += L[k * 6 + i] * L[k * 6 + j];
                }
                L[j * 6 + i] = i == j ? sqrt(A[i * 6 + i] - Sum) : ((1.0 / L[j * 6 + j]) * (A[j * 6 + i] - Sum));
            }
        }

        const double Det = L[0] * L[0] * L[7] * L[7] * L[14] * L[14] * L[21] * L[21] * L[28] * L[28] * L[35] * L[35];
        
        if (std::isnan(Det) || abs(Det) < 1e-12)
        {
            return false;
        }

        double y[6];
        
        y[0] = b[0] / L[0];
        y[1] = (b[1] - L[1] * y[0]) / L[7];
        y[2] = (b[2] - L[2] * y[0] - L[8] * y[1]) / L[14];
        y[3] = (b[3] - L[3] * y[0] - L[9] * y[1] - L[15] * y[2]) / L[21];
        y[4] = (b[4] - L[4] * y[0] - L[10] * y[1] - L[16] * y[2] - L[22] * y[3]) / L[28];
        y[5] = (b[5] - L[5] * y[0] - L[11] * y[1] - L[17] * y[2] - L[23] * y[3] - L[29] * y[4]) / L[35];

        double x[6];

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

        /*Float4x4 Rotation, Translation;
        Rotation.SetRotation(x[0], x[1], x[2]);
        Translation.SetTranslation(x[3], x[4], x[5]);

        rIncPoseMatrix = Translation * Rotation * rIncPoseMatrix;*/

        return true;
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::Integrate()
    {
        const int WorkGroups = GetWorkGroupCount(g_VolumeResolution, g_TileSize3D);

        Gfx::ContextManager::SetShaderCS(m_CSVolumeIntegration);

        glBindImageTexture(0, m_Volume, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RG16I);
        glBindImageTexture(1, m_RawDepthBuffer, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_IntrinsicsConstantBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_TrackingDataConstantBuffer);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glDispatchCompute(WorkGroups, WorkGroups, 1);
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::DownSample()
    {
        Gfx::ContextManager::SetShaderCS(m_CSRaycastPyramid);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_RaycastPyramidConstantBuffer);
        
        for (int PyramidLevel = 1; PyramidLevel < g_PyramidLevelCount; ++PyramidLevel)
        {
            const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth() >> PyramidLevel, g_TileSize2D);
            const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight() >> PyramidLevel, g_TileSize2D);

            float* pData = static_cast<float*>(glMapNamedBuffer(m_RaycastPyramidConstantBuffer, GL_WRITE_ONLY));
            *pData = 0.0f;
            glUnmapNamedBuffer(m_RaycastPyramidConstantBuffer);

            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            glBindImageTexture(0, m_RaycastVertexMap[PyramidLevel - 1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, m_RaycastVertexMap[PyramidLevel], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glDispatchCompute(WorkGroupsX, WorkGroupsY, 1);

            pData = static_cast<float*>(glMapNamedBuffer(m_RaycastPyramidConstantBuffer, GL_WRITE_ONLY));
            *pData = 1.0f;
            glUnmapNamedBuffer(m_RaycastPyramidConstantBuffer);

            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            glBindImageTexture(0, m_RaycastNormalMap[PyramidLevel - 1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, m_RaycastNormalMap[PyramidLevel], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glDispatchCompute(WorkGroupsX, WorkGroupsY, 1);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::Raycast()
    {
        const int WorkGroupsX = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthWidth(), g_TileSize2D);
        const int WorkGroupsY = GetWorkGroupCount(m_pRGBDCameraControl->GetDepthHeight(), g_TileSize2D);

        Gfx::ContextManager::SetShaderCS(m_CSRaycast);

        CSamplerPtr Sampler = Gfx::SamplerManager::GetSampler(Gfx::CSampler::ESampler::MinMagMipLinearClamp);
        CNativeSampler* NativeSampler = static_cast<CNativeSampler*>(Sampler.GetPtr());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, m_Volume);
        glBindSampler(0, NativeSampler->m_NativeSampler);
        glBindImageTexture(1, m_RaycastVertexMap[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(2, m_RaycastNormalMap[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_IntrinsicsConstantBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_TrackingDataConstantBuffer);

        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glDispatchCompute(WorkGroupsX, WorkGroupsY, 1);
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::UpdateReconstruction()
    {
        Performance::BeginEvent("Data Input");

        CreateReferencePyramid();

        Performance::EndEvent();

        if (m_IntegratedDepthFrameCount > 0)
        {
            Performance::BeginEvent("Tracking");

            PerformTracking();

            STrackingData TrackingData;
            TrackingData.m_PoseMatrix = m_PoseMatrix;
            TrackingData.m_InvPoseMatrix = m_PoseMatrix.GetInverted();

            STrackingData* pTrackingData = static_cast<STrackingData*>(glMapNamedBuffer(m_TrackingDataConstantBuffer, GL_WRITE_ONLY));
            memcpy(pTrackingData, &TrackingData, sizeof(STrackingData));
            glUnmapNamedBuffer(m_TrackingDataConstantBuffer);

            Performance::EndEvent();
        }

        Performance::BeginEvent("TSDF Integration and Raycasting");

        Integrate();
        Raycast();

        DownSample();

        Performance::EndEvent();

        ++m_IntegratedDepthFrameCount;
    }

    // -----------------------------------------------------------------------------

    int CGfxVoxelRenderer::GetWorkGroupCount(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::Render()
    {
        Performance::BeginEvent("Kinect Fusion");

        //if (m_NewDepthDataAvailable)
        {
            UpdateReconstruction();
            m_NewDepthDataAvailable = false;
        }

        RenderReconstructionData();

        Draw();
        RenderCamera();

        ++m_FrameCount;

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::RenderReconstructionData()
    {
        Performance::BeginEvent("Tracking Data Rendering");

        CTargetSetPtr DefaultTargetSetPtr = TargetSetManager::GetDefaultTargetSet();
        CNativeTargetSet NativeTargetSet = *static_cast<CNativeTargetSet*>(DefaultTargetSetPtr.GetPtr());
        
        glBindFramebuffer(GL_FRAMEBUFFER, NativeTargetSet.m_NativeTargetSet);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        RenderDepth();

        //GLint ViewPort[4];
        //glGetIntegerv(GL_VIEWPORT, ViewPort);

        //glViewport(0, 0, ViewPort[2] / 2, ViewPort[3]);
        //RenderVertexMap(m_KinectVertexMap[0], m_KinectNormalMap[0]);
        
        //glViewport(ViewPort[2] / 2, 0, ViewPort[2] / 2, ViewPort[3]);
        //RenderVertexMap(m_RaycastVertexMap[0], m_RaycastNormalMap[0]);

        //glViewport(ViewPort[0], ViewPort[1], ViewPort[2], ViewPort[3]);

        //RenderVolume();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::RenderDepth()
    {
        glBindVertexArray(1); // dummy vao because opengl needs vertex data even when it does not use it

        Gfx::ContextManager::SetShaderVS(m_VSVisualizeDepth);
        Gfx::ContextManager::SetShaderPS(m_FSVisualizeDepth);

        GLint ViewPort[4];
        glGetIntegerv(GL_VIEWPORT, ViewPort);

        glViewport(0, 0, ViewPort[2] / 2, ViewPort[3]);
        glBindImageTexture(0, m_RawDepthBuffer, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
        glViewport(ViewPort[2] / 2, 0, ViewPort[2] / 2, ViewPort[3]);
        glBindImageTexture(0, m_SmoothDepthBuffer[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

        glViewport(ViewPort[0], ViewPort[1], ViewPort[2], ViewPort[3]);

        glBindVertexArray(0);
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::RenderVertexMap(GLuint VertexMap, GLuint NormalMap)
    {
        Float4x4* pData = static_cast<Float4x4*>(glMapNamedBuffer(m_DrawCallConstantBuffer, GL_WRITE_ONLY));
        *pData = m_PoseMatrix * m_VertexMapWorldMatrix;
        glUnmapNamedBuffer(m_DrawCallConstantBuffer);

        glBindVertexArray(1); // dummy vao because opengl needs vertex data even when it does not use it

        glBindImageTexture(0, VertexMap, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(1, NormalMap, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

        CBufferPtr FrameConstantBufferPtr = Gfx::Main::GetPerFrameConstantBuffer();
        CNativeBuffer NativeBufer = *static_cast<CNativeBuffer*>(FrameConstantBufferPtr.GetPtr());
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, NativeBufer.m_NativeBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_DrawCallConstantBuffer);

        Gfx::ContextManager::SetShaderVS(m_VSVisualizeVertexMap);
        Gfx::ContextManager::SetShaderPS(m_FSVisualizeVertexMap);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glDrawArrays(GL_POINTS, 0, m_pRGBDCameraControl->GetDepthPixelCount());

        glBindVertexArray(0);
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::RenderVolume()
    {
        Float4x4* pData = static_cast<Float4x4*>(glMapNamedBuffer(m_DrawCallConstantBuffer, GL_WRITE_ONLY));
        *pData = m_VolumeWorldMatrix;
        glUnmapNamedBuffer(m_DrawCallConstantBuffer);

        glBindVertexArray(1); // dummy vao because opengl needs vertex data even when it does not use it

        Gfx::ContextManager::SetShaderVS(m_VSVisualizeVolume);
        Gfx::ContextManager::SetShaderPS(m_FSVisualizeVolume);

        CBufferPtr FrameConstantBufferPtr = Gfx::Main::GetPerFrameConstantBuffer();
        CNativeBuffer NativeBufer = *static_cast<CNativeBuffer*>(FrameConstantBufferPtr.GetPtr());
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, NativeBufer.m_NativeBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_DrawCallConstantBuffer);

        glBindImageTexture(0, m_Volume, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG16I);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glDrawArrays(GL_POINTS, 0, g_VolumeResolution * g_VolumeResolution * g_VolumeResolution);

        glBindVertexArray(0);
    }

    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::RenderCamera()
    {
        CTargetSetPtr DefaultTargetSetPtr = TargetSetManager::GetDefaultTargetSet();
        CNativeTargetSet NativeTargetSet = *static_cast<CNativeTargetSet*>(DefaultTargetSetPtr.GetPtr());

        glBindFramebuffer(GL_FRAMEBUFFER, NativeTargetSet.m_NativeTargetSet);
        //glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);

        Gfx::ContextManager::SetShaderVS(m_VSCamera);
        Gfx::ContextManager::SetShaderPS(m_FSCamera);

        Float4x4 WorldMatrix;
        WorldMatrix.SetScale(0.1f);
        WorldMatrix = m_PoseMatrix * WorldMatrix;

        Float4x4* pData = static_cast<Float4x4*>(glMapNamedBuffer(m_DrawCallConstantBuffer, GL_WRITE_ONLY));
        *pData = WorldMatrix;
        glUnmapNamedBuffer(m_DrawCallConstantBuffer);

        CBufferPtr FrameConstantBufferPtr = Gfx::Main::GetPerFrameConstantBuffer();
        CNativeBuffer NativeBufer = *static_cast<CNativeBuffer*>(FrameConstantBufferPtr.GetPtr());
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, NativeBufer.m_NativeBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_DrawCallConstantBuffer);
        
        glBindVertexArray(m_CameraVAO);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBindVertexArray(0);
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::SetSphere()
    {
        Gfx::ContextManager::SetShaderCS(m_CSSphere);

        glBindImageTexture(0, m_Volume, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG16I);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        const int WorkGroupSize = GetWorkGroupCount(g_VolumeResolution, g_TileSize3D);

        glDispatchCompute(WorkGroupSize, WorkGroupSize, WorkGroupSize);
    }
} // namespace

namespace Gfx
{
namespace VoxelRenderer
{
    void OnStart()
    {
        CGfxVoxelRenderer::GetInstance().OnStart();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnExit()
    {
        CGfxVoxelRenderer::GetInstance().OnExit();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupShader()
    {
        CGfxVoxelRenderer::GetInstance().OnSetupShader();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupKernels()
    {
        CGfxVoxelRenderer::GetInstance().OnSetupKernels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupRenderTargets()
    {
        CGfxVoxelRenderer::GetInstance().OnSetupRenderTargets();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupStates()
    {
        CGfxVoxelRenderer::GetInstance().OnSetupStates();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupTextures()
    {
        CGfxVoxelRenderer::GetInstance().OnSetupTextures();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupBuffers()
    {
        CGfxVoxelRenderer::GetInstance().OnSetupBuffers();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupResources()
    {
        CGfxVoxelRenderer::GetInstance().OnSetupResources();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupModels()
    {
        CGfxVoxelRenderer::GetInstance().OnSetupModels();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnSetupEnd()
    {
        CGfxVoxelRenderer::GetInstance().OnSetupEnd();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnReload()
    {
        CGfxVoxelRenderer::GetInstance().OnReload();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnNewMap()
    {
        CGfxVoxelRenderer::GetInstance().OnNewMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void OnUnloadMap()
    {
        CGfxVoxelRenderer::GetInstance().OnUnloadMap();
    }
    
    // -----------------------------------------------------------------------------
    
    void Update()
    {
        CGfxVoxelRenderer::GetInstance().Update();
    }
    
    // -----------------------------------------------------------------------------
    
    void Render()
    {
        CGfxVoxelRenderer::GetInstance().Render();
    }
} // namespace Voxel
} // namespace Gfx

