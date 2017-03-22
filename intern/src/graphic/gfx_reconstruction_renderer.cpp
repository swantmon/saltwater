
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
#include "graphic/gfx_reconstruction_renderer.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader_manager.h"
#include "graphic/gfx_state_manager.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_target_set_manager.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_view_manager.h"

#include "base/base_console.h"
#include "gfx_native_buffer.h"
#include "gfx_native_sampler.h"
#include "gfx_native_shader.h"
#include "gfx_native_target_set.h"

#include "mr/mr_slam_reconstructor.h"

#include <gl/glew.h>

#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

using namespace Gfx;

namespace
{
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

    private:

        void RenderVolume();
        void RenderCamera();
        
        // for debugging

        void RenderDepth();
        
    private:

        std::unique_ptr<MR::CSLAMReconstructor> m_pReconstructor;
        
        CShaderPtr m_VSCamera;
        CShaderPtr m_FSCamera;
        CShaderPtr m_VSRaycast;
        CShaderPtr m_FSRaycast;

        GLuint m_RaycastBuffer;
        GLuint m_DrawCallConstantBuffer;

        GLuint m_CameraVAO;

        CShaderPtr m_VSDepth;
        CShaderPtr m_FSDepth;
    };
} // namespace

namespace
{
    using namespace Base;

    CGfxReconstructionRenderer::CGfxReconstructionRenderer()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CGfxReconstructionRenderer::~CGfxReconstructionRenderer()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnStart()
    {
        Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxReconstructionRenderer::OnResize));

        m_pReconstructor.reset(new MR::CSLAMReconstructor);
    }

    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnExit()
    {
        m_VSCamera = 0;
        m_FSCamera = 0;        
        m_VSRaycast = 0;
        m_FSRaycast = 0;

        m_VSDepth = 0;
        m_FSDepth = 0;

        glDeleteBuffers(1, &m_RaycastBuffer);
        glDeleteBuffers(1, &m_DrawCallConstantBuffer);

        glDeleteVertexArrays(1, &m_CameraVAO);

        m_pReconstructor = nullptr;
    }
        
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupShader()
    {
        MR::CSLAMReconstructor::ReconstructionData Data;

        m_pReconstructor->GetReconstructionData(Data);

        std::stringstream DefineStream;

        DefineStream
            << "#define VOLUME_RESOLUTION "  << Data.m_VolumeResolution                     << " \n"
            << "#define INT16_MAX "          << std::numeric_limits<int16_t>::max()         << " \n"
            << "#define TRUNCATED_DISTANCE " << Data.m_TruncatedDistance                    << " \n"
            << "#define VOLUME_SIZE "        << Data.m_VolumeSize                           << " \n"
            << "#define VOXEL_SIZE "         << Data.m_VolumeSize / Data.m_VolumeResolution << " \n";

        std::string DefineString = DefineStream.str();

        m_VSCamera = ShaderManager::CompileVS("kinect_fusion\\vs_camera.glsl", "main", DefineString.c_str());
        m_FSCamera = ShaderManager::CompilePS("kinect_fusion\\fs_camera.glsl", "main", DefineString.c_str());
        m_VSRaycast = ShaderManager::CompileVS("kinect_fusion\\vs_raycast.glsl", "main", DefineString.c_str());
        m_FSRaycast = ShaderManager::CompilePS("kinect_fusion\\fs_raycast.glsl", "main", DefineString.c_str());

        //m_VSVisualizeDepth = ShaderManager::CompileVS("kinect_fusion\\vs_visualize_depth.glsl", "main", DefineString.c_str());
        //m_FSVisualizeDepth = ShaderManager::CompilePS("kinect_fusion\\fs_visualize_depth.glsl", "main", DefineString.c_str());
        //m_VSVisualizeVertexMap = ShaderManager::CompileVS("kinect_fusion\\vs_visualize_vertex_map.glsl", "main", DefineString.c_str());
        //m_FSVisualizeVertexMap = ShaderManager::CompilePS("kinect_fusion\\fs_visualize_vertex_map.glsl", "main", DefineString.c_str());
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
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupTextures()
    {

    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupBuffers()
    {
        glCreateBuffers(1, &m_RaycastBuffer);
        glNamedBufferData(m_RaycastBuffer, sizeof(Float4) * 2, nullptr, GL_DYNAMIC_DRAW);

        glCreateBuffers(1, &m_DrawCallConstantBuffer);
        glNamedBufferData(m_DrawCallConstantBuffer, sizeof(Float4x4) * 2, nullptr, GL_DYNAMIC_DRAW);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupModels()
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
    
    void CGfxReconstructionRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnReload()
    {

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
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::RenderVolume()
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

        Float4x4 PoseMatrix = m_pReconstructor->GetPoseMatrix();
        bool TrackingLost = m_pReconstructor->IsTrackingLost();

        Float4 RaycastData[2];
        PoseMatrix.GetTranslation(RaycastData[0][0], RaycastData[0][1], RaycastData[0][2]);
        RaycastData[0][3] = 1.0f;
        RaycastData[1] = TrackingLost ? Float4(1.0f, 0.0f, 0.0f, 1.0f) : Float4(0.0f, 1.0f, 0.0f, 1.0f);

        Float4* pData = static_cast<Float4*>(glMapNamedBuffer(m_RaycastBuffer, GL_WRITE_ONLY));
        memcpy(pData, &RaycastData, sizeof(RaycastData));
        glUnmapNamedBuffer(m_RaycastBuffer);
        
        Gfx::ContextManager::SetShaderVS(m_VSRaycast);
        Gfx::ContextManager::SetShaderPS(m_FSRaycast);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, m_pReconstructor->GetVolume());
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

    void CGfxReconstructionRenderer::Render()
    {
        m_pReconstructor->Update();

        Performance::BeginEvent("SLAM Reconstruction Rendering");
        
        //RenderReconstructionData();

        RenderVolume();
        RenderCamera();
        
        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderDepth()
    {
        glBindVertexArray(1); // dummy vao because opengl needs vertex data even when it does not use it

        Gfx::ContextManager::SetShaderVS(m_VSDepth);
        Gfx::ContextManager::SetShaderPS(m_FSDepth);

        GLint ViewPort[4];
        glGetIntegerv(GL_VIEWPORT, ViewPort);

        glViewport(0, 0, ViewPort[2] / 2, ViewPort[3]);
        //glBindImageTexture(0, m_RawDepthBuffer, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
        glViewport(ViewPort[2] / 2, 0, ViewPort[2] / 2, ViewPort[3]);
        //glBindImageTexture(0, m_SmoothDepthBuffer[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

        glViewport(ViewPort[0], ViewPort[1], ViewPort[2], ViewPort[3]);

        glBindVertexArray(0);
    }

    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::RenderCamera()
    {
        CTargetSetPtr DefaultTargetSetPtr = TargetSetManager::GetDefaultTargetSet();
        CNativeTargetSet NativeTargetSet = *static_cast<CNativeTargetSet*>(DefaultTargetSetPtr.GetPtr());

        glBindFramebuffer(GL_FRAMEBUFFER, NativeTargetSet.m_NativeTargetSet);

        Gfx::ContextManager::SetShaderVS(m_VSCamera);
        Gfx::ContextManager::SetShaderPS(m_FSCamera);

        Float4x4 WorldMatrix;
        WorldMatrix.SetScale(0.1f);
        WorldMatrix = m_pReconstructor->GetPoseMatrix() * WorldMatrix;

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
} // namespace Voxel
} // namespace Gfx

