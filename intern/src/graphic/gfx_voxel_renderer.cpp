
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
#include "gfx_native_shader.h"
#include "gfx_native_target_set.h"

#include "mr/mr_depth_sensor_control.h"
#include "mr/mr_kinect_control.h"
#include "mr/mr_realsense_control.h"

#include <gl/glew.h>

#include <memory>
#include <sstream>

using namespace Gfx;

namespace
{
    const float g_VolumeSize = 1.0f;
    const int g_VolumeResolution = 256;
    const float g_VoxelSize = g_VolumeSize / g_VolumeResolution;

    const Base::Float3 g_InitialCameraPosition = Base::Float3(g_VolumeSize * 0.5f, g_VolumeSize * 0.5f, -g_VolumeSize * 0.5f);
    const Base::Float3 g_InitialCameraRotation = Base::Float3(0.0f, 0.0f, 0.0f);

    const int g_MaxIntegrationWeight = 100;

    const int g_PyramidLevels = 3;

    const int g_ICPIterations[g_PyramidLevels] = { 10, 5, 4 };
    const float g_EpsilonVertex = 0.1f;
    const float g_EpsilonNormal = 0.342f;
    const float g_TruncatedDistance = 30.0f;
    const float g_TruncatedDistanceInverse = 1.0f / g_TruncatedDistance;

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
        Base::Float4x4 m_PoseRotationMatrix;
        Base::Float4x4 m_InvPoseRotationMatrix;
        Base::Float4x4 m_PoseTranslationMatrix;
        Base::Float4x4 m_InvPoseTranslationMatrix;
        Base::Float4x4 m_PoseMatrix;
        Base::Float4x4 m_InvPoseMatrix;
    };

    struct SDrawCallBufferData
    {
        Base::Float4x4 m_WorldMatrix;
    };
    
    const unsigned int g_TileSize2D = 16;
    const unsigned int g_TileSize3D = 8;
    
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

        void ReadKinectData();
        void PerformTracking();
        void Integrate();
        void Raycast();
        void DownSample();

        // Just for debugging

        void RenderReconstructionData();

        void RenderDepth();
        void RenderVertexMap(GLuint VertexMap);
        void RenderVolume();
        void SetSphere();

        GLuint m_DebugBuffer;

    private:

        GLuint m_DrawCallConstantBuffer;
        GLuint m_IntrinsicsConstantBuffer;
        GLuint m_TrackingDataConstantBuffer;
        GLuint m_RaycastPyramidConstantBuffer;

        CShaderPtr m_VSVisualizeDepth;
        CShaderPtr m_FSVisualizeDepth;
        CShaderPtr m_VSVisualizeVertexMap;
        CShaderPtr m_FSVisualizeVertexMap;
        CShaderPtr m_VSVisualizeVolume;
        CShaderPtr m_FSVisualizeVolume;

        CShaderPtr m_CSMirrorDepth;
        CShaderPtr m_CSBilateralFilter;
        CShaderPtr m_CSVertexMap;
        CShaderPtr m_CSNormalMap;
        CShaderPtr m_CSDownSampleDepth;
        CShaderPtr m_CSVolumeIntegration;
        CShaderPtr m_CSRaycast;
        CShaderPtr m_CSRaycastPyramid;

        CShaderPtr m_CSSphere;

        GLuint m_KinectRawDepthBuffer;
        GLuint m_KinectSmoothDepthBuffer[g_PyramidLevels];
        GLuint m_KinectVertexMap[g_PyramidLevels];
        GLuint m_KinectNormalMap[g_PyramidLevels];

        GLuint m_RaycastVertexMap[g_PyramidLevels];
        GLuint m_RaycastNormalMap[g_PyramidLevels];

        GLuint m_Volume;

        std::unique_ptr<MR::CDepthSensorControl> m_pDepthSensorControl;

        Base::Float4x4 m_VertexMapWorldMatrix;
        Base::Float4x4 m_VolumeWorldMatrix;

        std::vector<unsigned short> m_DepthPixels;

        bool m_NewDepthDataAvailable;
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
        
        try
        {
            m_pDepthSensorControl.reset(new MR::CRealSenseControl());
            m_pDepthSensorControl->Start();
        }
        catch (...)
        {
            m_pDepthSensorControl.reset(new MR::CKinectControl());
            m_pDepthSensorControl->Start();
        }

        m_DepthPixels = std::vector<unsigned short>(m_pDepthSensorControl->GetPixelCount());

        m_NewDepthDataAvailable = false;
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

        m_CSMirrorDepth = 0;
        m_CSBilateralFilter = 0;
        m_CSVertexMap = 0;
        m_CSNormalMap = 0;
        m_CSDownSampleDepth = 0;
        m_CSVolumeIntegration = 0;
        m_CSRaycast = 0;
        m_CSRaycastPyramid = 0;
        m_CSSphere = 0;

        glDeleteTextures(1, &m_KinectRawDepthBuffer);
        glDeleteTextures(g_PyramidLevels, m_KinectSmoothDepthBuffer);
        glDeleteTextures(g_PyramidLevels, m_KinectVertexMap);
        glDeleteTextures(g_PyramidLevels, m_KinectNormalMap);
        glDeleteTextures(g_PyramidLevels, m_RaycastVertexMap);
        glDeleteTextures(g_PyramidLevels, m_RaycastNormalMap);
        glDeleteTextures(1, &m_Volume);
        glDeleteTextures(1, &m_DebugBuffer);

        glDeleteBuffers(1, &m_DrawCallConstantBuffer);
        glDeleteBuffers(1, &m_IntrinsicsConstantBuffer);
        glDeleteBuffers(1, &m_TrackingDataConstantBuffer);
        glDeleteBuffers(1, &m_RaycastPyramidConstantBuffer);
    }

    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupShader()
    {
        int NumberOfDefines = 11;

        std::vector<std::stringstream> DefineStreams(NumberOfDefines);

        DefineStreams[0] << "VOLUME_RESOLUTION " << g_VolumeResolution;
        DefineStreams[1] << "VOXEL_SIZE " << g_VoxelSize;
        DefineStreams[2] << "VOLUME_SIZE " << g_VolumeSize;
        DefineStreams[3] << "DEPTH_IMAGE_WIDTH " << m_pDepthSensorControl->GetWidth();
        DefineStreams[4] << "DEPTH_IMAGE_HEIGHT " << m_pDepthSensorControl->GetHeight();
        DefineStreams[5] << "TILE_SIZE2D " << g_TileSize2D;
        DefineStreams[6] << "TILE_SIZE3D " << g_TileSize3D;
        DefineStreams[7] << "INT16_MAX " << 32767;
        DefineStreams[8] << "TRUNCATED_DISTANCE " << g_TruncatedDistance;
        DefineStreams[9] << "TRUNCATED_DISTANCE_INVERSE " << g_TruncatedDistanceInverse;
        DefineStreams[10] << "MAX_INTEGRATION_WEIGHT " << g_MaxIntegrationWeight;

        std::vector<std::string> DefineStrings(NumberOfDefines);
        std::vector<const char*> Defines(NumberOfDefines);

        for (int i = 0; i < NumberOfDefines; ++ i)
        {
            DefineStrings[i] = DefineStreams[i].str();

            Defines[i] = DefineStrings[i].c_str();
        }

        m_VSVisualizeDepth = ShaderManager::CompileVS("kinect_fusion\\vs_visualize_depth.glsl", "main");
        m_FSVisualizeDepth = ShaderManager::CompilePS("kinect_fusion\\fs_visualize_depth.glsl", "main");
        m_VSVisualizeVertexMap = ShaderManager::CompileVS("kinect_fusion\\vs_visualize_vertex_map.glsl", "main");
        m_FSVisualizeVertexMap = ShaderManager::CompilePS("kinect_fusion\\fs_visualize_vertex_map.glsl", "main");
        m_VSVisualizeVolume = ShaderManager::CompileVS("kinect_fusion\\vs_visualize_volume.glsl", "main", NumberOfDefines, Defines.data());
        m_FSVisualizeVolume = ShaderManager::CompilePS("kinect_fusion\\fs_visualize_volume.glsl", "main");

        m_CSMirrorDepth = ShaderManager::CompileCS("kinect_fusion\\cs_mirror_depth.glsl", "main", NumberOfDefines, Defines.data());
        m_CSBilateralFilter = ShaderManager::CompileCS("kinect_fusion\\cs_bilateral_filter.glsl", "main", NumberOfDefines, Defines.data());
        m_CSVertexMap = ShaderManager::CompileCS("kinect_fusion\\cs_vertex_map.glsl", "main", NumberOfDefines, Defines.data());
        m_CSNormalMap = ShaderManager::CompileCS("kinect_fusion\\cs_normal_map.glsl", "main", NumberOfDefines, Defines.data());
        m_CSDownSampleDepth = ShaderManager::CompileCS("kinect_fusion\\cs_downsample_depth.glsl", "main", NumberOfDefines, Defines.data());
        m_CSVolumeIntegration = ShaderManager::CompileCS("kinect_fusion\\cs_integrate_volume.glsl", "main", NumberOfDefines, Defines.data());
        m_CSRaycast = ShaderManager::CompileCS("kinect_fusion\\cs_raycast.glsl", "main", NumberOfDefines, Defines.data());
        m_CSRaycastPyramid = ShaderManager::CompileCS("kinect_fusion\\cs_raycast_pyramid.glsl", "main", NumberOfDefines, Defines.data());

        m_CSSphere = ShaderManager::CompileCS("kinect_fusion\\cs_sphere.glsl", "main", NumberOfDefines, Defines.data());
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
        glCreateTextures(GL_TEXTURE_2D, 1, &m_KinectRawDepthBuffer);
        glCreateTextures(GL_TEXTURE_2D, g_PyramidLevels, m_KinectSmoothDepthBuffer);
        glCreateTextures(GL_TEXTURE_2D, g_PyramidLevels, m_KinectVertexMap);
        glCreateTextures(GL_TEXTURE_2D, g_PyramidLevels, m_KinectNormalMap);
        glCreateTextures(GL_TEXTURE_2D, g_PyramidLevels, m_RaycastVertexMap);
        glCreateTextures(GL_TEXTURE_2D, g_PyramidLevels, m_RaycastNormalMap);

        glCreateTextures(GL_TEXTURE_3D, 1, &m_Volume);

        glTextureStorage2D(m_KinectRawDepthBuffer, 1, GL_R16UI, m_pDepthSensorControl->GetWidth(), m_pDepthSensorControl->GetHeight());
        
        for (int i = 0; i < g_PyramidLevels; ++i)
        {
            const int Width = m_pDepthSensorControl->GetWidth() >> i;
            const int Height = m_pDepthSensorControl->GetHeight() >> i;

            glTextureStorage2D(m_KinectSmoothDepthBuffer[i], 1, GL_R16UI, Width, Height);
            glTextureStorage2D(m_KinectVertexMap[i], 1, GL_RGBA32F, Width, Height);
            glTextureStorage2D(m_KinectNormalMap[i], 1, GL_RGBA32F, Width, Height);
            glTextureStorage2D(m_RaycastVertexMap[i], 1, GL_RGBA32F, Width, Height);
            glTextureStorage2D(m_RaycastNormalMap[i], 1, GL_RGBA32F, Width, Height);
        }

        glTextureStorage3D(m_Volume, 1, GL_RG16I, g_VolumeResolution, g_VolumeResolution, g_VolumeResolution);

        glCreateTextures(GL_TEXTURE_3D, 1, &m_DebugBuffer);
        glTextureStorage3D(m_DebugBuffer, 1, GL_RGBA32F, g_VolumeResolution, g_VolumeResolution, g_VolumeResolution);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupBuffers()
    {
        const float FocalLengthX = m_pDepthSensorControl->GetFocalLengthX();
        const float FocalLengthY = m_pDepthSensorControl->GetFocalLengthY();
        const float FocalPointX = m_pDepthSensorControl->GetFocalPointX();
        const float FocalPointY = m_pDepthSensorControl->GetFocalPointY();

        glCreateBuffers(1, &m_DrawCallConstantBuffer);
        glNamedBufferData(m_DrawCallConstantBuffer, sizeof(Base::Float4x4), nullptr, GL_DYNAMIC_DRAW);

        Base::Float4x4 KMatrix(
            FocalLengthX,	      0.0f, FocalPointX, 0.0f,
                    0.0f, FocalLengthY, FocalPointY, 0.0f,
                    0.0f,		  0.0f,        1.0f, 0.0f,
                    0.0f,		  0.0f,        0.0f, 1.0f
        );

        SIntrinsics Intrinsics;

        Intrinsics.m_FocalPoint = Base::Float2(FocalPointX, FocalPointY);
        Intrinsics.m_FocalLength = Base::Float2(FocalLengthX, FocalLengthY);
        Intrinsics.m_InvFocalLength = Base::Float2(1.0f / FocalLengthX, 1.0f / FocalLengthY);
        Intrinsics.m_KMatrix = Intrinsics.m_InvKMatrix = KMatrix;
        Intrinsics.m_InvKMatrix.Invert();

        glCreateBuffers(1, &m_IntrinsicsConstantBuffer);
        glNamedBufferData(m_IntrinsicsConstantBuffer, sizeof(SIntrinsics), &Intrinsics, GL_STATIC_DRAW);

        STrackingData TrackingData;

        TrackingData.m_PoseRotationMatrix.SetRotation(g_InitialCameraRotation[0], g_InitialCameraRotation[1], g_InitialCameraRotation[2]);
        TrackingData.m_PoseTranslationMatrix.SetTranslation(g_InitialCameraPosition[0], g_InitialCameraPosition[1], g_InitialCameraPosition[2]);
        TrackingData.m_PoseMatrix = TrackingData.m_PoseTranslationMatrix * TrackingData.m_PoseRotationMatrix;

        TrackingData.m_InvPoseRotationMatrix = TrackingData.m_PoseRotationMatrix.GetInverted();
        TrackingData.m_InvPoseTranslationMatrix = TrackingData.m_PoseTranslationMatrix.GetInverted();
        TrackingData.m_InvPoseMatrix = TrackingData.m_PoseMatrix.GetInverted();

        glCreateBuffers(1, &m_TrackingDataConstantBuffer);
        glNamedBufferData(m_TrackingDataConstantBuffer, sizeof(STrackingData), &TrackingData, GL_DYNAMIC_COPY);

        glCreateBuffers(1, &m_RaycastPyramidConstantBuffer);
        glNamedBufferData(m_RaycastPyramidConstantBuffer, 16, nullptr, GL_DYNAMIC_DRAW);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupModels()
    {
        
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
        // Get Kinect Data
        //////////////////////////////////////////////////////////////////////////////////////

        Base::Float4x4 TranslationMatrix;
        Base::Float4x4 ScalingMatrix;
        Base::Float4x4 RotationMatrix;

        ScalingMatrix.SetScale(0.005f);
        RotationMatrix.SetRotationX(3.14f);
        
        m_VertexMapWorldMatrix = RotationMatrix * ScalingMatrix;

        ScalingMatrix.SetScale(5.0f / static_cast<float>(g_VolumeResolution));
        TranslationMatrix.SetTranslation(0.0f, 0.0f, 0.0f);

        m_VolumeWorldMatrix = TranslationMatrix * RotationMatrix * ScalingMatrix;

        if (m_pDepthSensorControl->GetDepthBuffer(m_DepthPixels.data()))
        {
            glTextureSubImage2D(m_KinectRawDepthBuffer, 0, 0, 0,
                m_pDepthSensorControl->GetWidth(), m_pDepthSensorControl->GetHeight(),
                GL_RED_INTEGER, GL_UNSIGNED_SHORT, m_DepthPixels.data());

            //////////////////////////////////////////////////////////////////////////////////////
            // Mirror depth data
            //////////////////////////////////////////////////////////////////////////////////////

            const int WorkGroupsX = m_pDepthSensorControl->GetWidth() / g_TileSize2D + 1;
            const int WorkGroupsY = m_pDepthSensorControl->GetHeight() / g_TileSize2D + 1;

            Gfx::ContextManager::SetShaderCS(m_CSMirrorDepth);
            glBindImageTexture(0, m_KinectRawDepthBuffer, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R16UI);
            glDispatchCompute(WorkGroupsX / 2 + 1, WorkGroupsY + 1, 1);

            m_NewDepthDataAvailable = true;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::ReadKinectData()
    {
        const int WorkGroupsX = m_pDepthSensorControl->GetWidth() / g_TileSize2D;
        const int WorkGroupsY = m_pDepthSensorControl->GetHeight() / g_TileSize2D;

        //////////////////////////////////////////////////////////////////////////////////////
        // Bilateral Filter
        //////////////////////////////////////////////////////////////////////////////////////

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        Gfx::ContextManager::SetShaderCS(m_CSBilateralFilter);
        glBindImageTexture(0, m_KinectRawDepthBuffer, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
        glBindImageTexture(1, m_KinectSmoothDepthBuffer[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16UI);
        glDispatchCompute(WorkGroupsX, WorkGroupsY, 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        for (int PyramidLevel = 0; PyramidLevel < g_PyramidLevels - 1; ++ PyramidLevel)
        {
            //////////////////////////////////////////////////////////////////////////////////////
            // Downsample depth buffer
            //////////////////////////////////////////////////////////////////////////////////////

            Gfx::ContextManager::SetShaderCS(m_CSDownSampleDepth);
            glBindImageTexture(0, m_KinectSmoothDepthBuffer[PyramidLevel], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
            glBindImageTexture(1, m_KinectSmoothDepthBuffer[PyramidLevel + 1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16UI);
            glDispatchCompute(WorkGroupsX >> PyramidLevel, WorkGroupsY >> PyramidLevel, 1);
        }

        /////////////////////////////////////////////////////////////////////////////////////
        // Generate vertex and normal map
        /////////////////////////////////////////////////////////////////////////////////////

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_IntrinsicsConstantBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_TrackingDataConstantBuffer);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        for (int PyramidLevel = 0; PyramidLevel < g_PyramidLevels; ++ PyramidLevel)
        {
            Gfx::ContextManager::SetShaderCS(m_CSVertexMap);
            glBindImageTexture(0, m_KinectSmoothDepthBuffer[PyramidLevel], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
            glBindImageTexture(1, m_KinectVertexMap[PyramidLevel], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glDispatchCompute(WorkGroupsX >> PyramidLevel, WorkGroupsY >> PyramidLevel, 1);			
        }

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        for (int PyramidLevel = 0; PyramidLevel < g_PyramidLevels; ++ PyramidLevel)
        {
            Gfx::ContextManager::SetShaderCS(m_CSNormalMap);
            glBindImageTexture(0, m_KinectVertexMap[PyramidLevel], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, m_KinectNormalMap[PyramidLevel], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glDispatchCompute(WorkGroupsX >> PyramidLevel, WorkGroupsY >> PyramidLevel, 1);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::PerformTracking()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::Integrate()
    {
        Gfx::ContextManager::SetShaderCS(m_CSVolumeIntegration);

        glBindImageTexture(0, m_Volume, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RG16I);
        glBindImageTexture(1, m_KinectRawDepthBuffer, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
        glBindImageTexture(2, m_DebugBuffer, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_IntrinsicsConstantBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_TrackingDataConstantBuffer);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glDispatchCompute(g_VolumeResolution / g_TileSize3D, g_VolumeResolution / g_TileSize3D, 1);
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::DownSample()
    {
        const int WorkGroupsX = m_pDepthSensorControl->GetWidth() / g_TileSize2D;
        const int WorkGroupsY = m_pDepthSensorControl->GetHeight() / g_TileSize2D;

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_RaycastPyramidConstantBuffer);

        for (int PyramidLevel = 1; PyramidLevel < g_PyramidLevels; ++PyramidLevel)
        {
            bool* pData = static_cast<bool*>(glMapNamedBuffer(m_RaycastPyramidConstantBuffer, GL_WRITE_ONLY));
            *pData = false;
            glUnmapNamedBuffer(m_RaycastPyramidConstantBuffer);

            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            Gfx::ContextManager::SetShaderCS(m_CSRaycastPyramid);
            glBindImageTexture(0, m_RaycastVertexMap[PyramidLevel - 1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, m_RaycastVertexMap[PyramidLevel], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glDispatchCompute(WorkGroupsX >> PyramidLevel, WorkGroupsY >> PyramidLevel, 1);

            pData = static_cast<bool*>(glMapNamedBuffer(m_RaycastPyramidConstantBuffer, GL_WRITE_ONLY));
            *pData = true;
            glUnmapNamedBuffer(m_RaycastPyramidConstantBuffer);

            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            Gfx::ContextManager::SetShaderCS(m_CSRaycastPyramid);
            glBindImageTexture(0, m_RaycastNormalMap[PyramidLevel - 1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, m_RaycastNormalMap[PyramidLevel], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glDispatchCompute(WorkGroupsX >> PyramidLevel, WorkGroupsY >> PyramidLevel, 1);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::Raycast()
    {
        Gfx::ContextManager::SetShaderCS(m_CSRaycast);

        glBindImageTexture(0, m_Volume, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG16I);
        glBindImageTexture(1, m_RaycastVertexMap[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_IntrinsicsConstantBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_TrackingDataConstantBuffer);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glDispatchCompute(m_pDepthSensorControl->GetWidth() / g_TileSize2D, m_pDepthSensorControl->GetHeight() / g_TileSize2D, 1);
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::UpdateReconstruction()
    {        
        Performance::BeginEvent("Kinect Tracking");

        ReadKinectData();
        PerformTracking();
        Integrate();

        //SetSphere(); // debugging

        Raycast();
        DownSample();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::Render()
    {
        //if (m_NewDepthDataAvailable)
        {
            UpdateReconstruction();
            m_NewDepthDataAvailable = false;
        }
        RenderReconstructionData();
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
        //glViewport(0, 0, 640, 720);
        //RenderVertexMap(m_KinectVertexMap[0]);
        //RenderVolume();
        //glViewport(640, 0, 640, 720);
        RenderVertexMap(m_RaycastVertexMap[0]);

        glViewport(0, 0, 1280, 720);
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
        glBindImageTexture(0, m_KinectRawDepthBuffer, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
        glViewport(ViewPort[2] / 2, 0, ViewPort[2] / 2, ViewPort[3]);
        glBindImageTexture(0, m_KinectSmoothDepthBuffer[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

        glViewport(ViewPort[0], ViewPort[1], ViewPort[2], ViewPort[3]);

        glBindVertexArray(0);
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::RenderVertexMap(GLuint VertexMap)
    {
        Base::Float4x4* pData = static_cast<Base::Float4x4*>(glMapNamedBuffer(m_DrawCallConstantBuffer, GL_WRITE_ONLY));
        *pData = m_VertexMapWorldMatrix;
        glUnmapNamedBuffer(m_DrawCallConstantBuffer);

        glBindVertexArray(1); // dummy vao because opengl needs vertex data even when it does not use it

        glBindImageTexture(0, VertexMap, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

        CBufferPtr FrameConstantBufferPtr = Gfx::Main::GetPerFrameConstantBufferVS();
        CNativeBuffer NativeBufer = *static_cast<CNativeBuffer*>(FrameConstantBufferPtr.GetPtr());
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, NativeBufer.m_NativeBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_DrawCallConstantBuffer);

        Gfx::ContextManager::SetShaderVS(m_VSVisualizeVertexMap);
        Gfx::ContextManager::SetShaderPS(m_FSVisualizeVertexMap);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glDrawArrays(GL_POINTS, 0, m_pDepthSensorControl->GetPixelCount());

        glBindVertexArray(0);
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::RenderVolume()
    {
        Base::Float4x4* pData = static_cast<Base::Float4x4*>(glMapNamedBuffer(m_DrawCallConstantBuffer, GL_WRITE_ONLY));
        *pData = m_VolumeWorldMatrix;
        glUnmapNamedBuffer(m_DrawCallConstantBuffer);

        glBindVertexArray(1); // dummy vao because opengl needs vertex data even when it does not use it

        Gfx::ContextManager::SetShaderVS(m_VSVisualizeVolume);
        Gfx::ContextManager::SetShaderPS(m_FSVisualizeVolume);

        CBufferPtr FrameConstantBufferPtr = Gfx::Main::GetPerFrameConstantBufferVS();
        CNativeBuffer NativeBufer = *static_cast<CNativeBuffer*>(FrameConstantBufferPtr.GetPtr());
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, NativeBufer.m_NativeBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_DrawCallConstantBuffer);

        glBindImageTexture(0, m_Volume, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG16I);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glDrawArrays(GL_POINTS, 0, g_VolumeResolution * g_VolumeResolution * g_VolumeResolution);

        glBindVertexArray(0);
    }

    void CGfxVoxelRenderer::SetSphere()
    {
        Gfx::ContextManager::SetShaderCS(m_CSSphere);

        glBindImageTexture(0, m_Volume, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG16I);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        const int WorkGroupSize = g_VolumeResolution / g_TileSize3D;

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

