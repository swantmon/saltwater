
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
#include "gfx_native_texture_3d.h"

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

        CBufferPtr m_RaycastBuffer;
        CBufferPtr m_DrawCallConstantBuffer;

        GLuint m_CameraVAO;

        CMeshPtr m_CameraMesh;
        CInputLayoutPtr m_InputLayout;

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

        m_RaycastBuffer = 0;
        m_DrawCallConstantBuffer = 0;
        
        glDeleteVertexArrays(1, &m_CameraVAO);
        
        m_CameraMesh = 0;
        m_InputLayout = 0;

        m_pReconstructor = nullptr;
    }
        
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupShader()
    {
        MR::CSLAMReconstructor::ReconstructionSettings Settings;

        m_pReconstructor->GetReconstructionData(Settings);

        std::stringstream DefineStream;

        DefineStream
            << "#define VOLUME_RESOLUTION "  << Settings.m_VolumeResolution                         << " \n"
            << "#define INT16_MAX "          << std::numeric_limits<int16_t>::max()                 << " \n"
            << "#define TRUNCATED_DISTANCE " << Settings.m_TruncatedDistance                        << " \n"
            << "#define VOLUME_SIZE "        << Settings.m_VolumeSize                               << " \n"
            << "#define VOXEL_SIZE "         << Settings.m_VolumeSize / Settings.m_VolumeResolution << " \n";

        std::string DefineString = DefineStream.str();

        m_VSCamera = ShaderManager::CompileVS("kinect_fusion\\vs_camera.glsl", "main", DefineString.c_str());
        m_FSCamera = ShaderManager::CompilePS("kinect_fusion\\fs_camera.glsl", "main", DefineString.c_str());
        m_VSRaycast = ShaderManager::CompileVS("kinect_fusion\\vs_raycast.glsl", "main", DefineString.c_str());
        m_FSRaycast = ShaderManager::CompilePS("kinect_fusion\\fs_raycast.glsl", "main", DefineString.c_str());

        //m_VSVisualizeDepth = ShaderManager::CompileVS("kinect_fusion\\vs_visualize_depth.glsl", "main", DefineString.c_str());
        //m_FSVisualizeDepth = ShaderManager::CompilePS("kinect_fusion\\fs_visualize_depth.glsl", "main", DefineString.c_str());

        SInputElementDescriptor InputLayout = {};

        InputLayout.m_pSemanticName        = "POSITION";
        InputLayout.m_SemanticIndex        = 0;
        InputLayout.m_Format               = CInputLayout::Float3Format;
        InputLayout.m_InputSlot            = 0;
        InputLayout.m_AlignedByteOffset    = 0;
        InputLayout.m_Stride               = 12;
        InputLayout.m_InputSlotClass       = CInputLayout::PerVertex;
        InputLayout.m_InstanceDataStepRate = 0;

        m_InputLayout = ShaderManager::CreateInputLayout(&InputLayout, 1, m_VSCamera);
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
        SBufferDescriptor ConstantBufferDesc;

        ConstantBufferDesc.m_Stride = 0;
        ConstantBufferDesc.m_Usage = CBuffer::EUsage::GPURead;
        ConstantBufferDesc.m_Binding = CBuffer::ConstantBuffer;
        ConstantBufferDesc.m_Access = CBuffer::CPUWrite;
        ConstantBufferDesc.m_NumberOfBytes = sizeof(Float4) * 2;
        ConstantBufferDesc.m_pBytes = nullptr;
        ConstantBufferDesc.m_pClassKey = 0;

        m_RaycastBuffer = BufferManager::CreateBuffer(ConstantBufferDesc);
        
        ConstantBufferDesc.m_NumberOfBytes = sizeof(Float4x4) * 2;

        m_DrawCallConstantBuffer = BufferManager::CreateBuffer(ConstantBufferDesc);
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
                
        Dt::CSurface* pSurface = new Dt::CSurface;
        Dt::CLOD* pLOD = new Dt::CLOD;
        Dt::CMesh* pMesh = new Dt::CMesh;

        pSurface->SetPositions(Vertices);
        pSurface->SetNumberOfVertices(sizeof(Vertices) / sizeof(Vertices[0]));
        pSurface->SetIndices(Indices);
        pSurface->SetNumberOfIndices(sizeof(Indices) / sizeof(Indices[0]));
        pSurface->SetElements(0);

        pLOD->SetSurface(0, pSurface);
        pLOD->SetNumberOfSurfaces(1);
        
        pMesh->SetLOD(0, pLOD);
        pMesh->SetNumberOfLODs(1);

        SMeshDescriptor MeshDesc =
        {
            pMesh
        };

        m_CameraMesh = MeshManager::CreateMesh(MeshDesc);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnReload()
    {
        //MR::CSLAMReconstructor::ReconstructionSettings Settings;
        //m_pReconstructor->GetReconstructionData(Settings);
        //Settings.m_VolumeResolution = 512;
        //m_pReconstructor->ResetReconstruction(&Settings);

        m_pReconstructor->ResetReconstruction();

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
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::RenderVolume()
    {
        Performance::BeginEvent("Rendering");
        
        glBindVertexArray(1); // dummy vao because opengl needs vertex data even when it does not use it

        Float4x4 PoseMatrix = m_pReconstructor->GetPoseMatrix();

        Float4 RaycastData[2];
        PoseMatrix.GetTranslation(RaycastData[0][0], RaycastData[0][1], RaycastData[0][2]);
        RaycastData[0][3] = 1.0f;
        RaycastData[1] = m_pReconstructor->IsTrackingLost() ? Float4(1.0f, 0.0f, 0.0f, 1.0f) : Float4(0.0f, 1.0f, 0.0f, 1.0f);

        BufferManager::UploadConstantBufferData(m_RaycastBuffer, RaycastData);
                
        Gfx::ContextManager::SetShaderVS(m_VSRaycast);
        Gfx::ContextManager::SetShaderPS(m_FSRaycast);

        ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(m_pReconstructor->GetVolume()));
        ContextManager::SetSampler(0, SamplerManager::GetSampler(Gfx::CSampler::ESampler::MinMagMipLinearClamp));

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_RaycastBuffer);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetTopology(STopology::TriangleFan);
        ContextManager::Draw(4, 0);
                
        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::Render()
    {
        m_pReconstructor->Update();

        Performance::BeginEvent("SLAM Reconstruction Rendering");
        
        //RenderReconstructionData();

        Base::Float4 ClearColor(0.2f, 0.2f, 0.2f, 1.0f);

        ContextManager::SetTargetSet(TargetSetManager::GetDefaultTargetSet());
        //TargetSetManager::ClearTargetSet(TargetSetManager::GetDefaultTargetSet(), ClearColor);

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
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Wireframe));

        Gfx::ContextManager::SetShaderVS(m_VSCamera);
        Gfx::ContextManager::SetShaderPS(m_FSCamera);

        Float4x4 WorldMatrix;
        WorldMatrix.SetScale(0.1f);
        WorldMatrix = m_pReconstructor->GetPoseMatrix() * WorldMatrix;

        BufferManager::UploadConstantBufferData(m_DrawCallConstantBuffer, &WorldMatrix);
        
        ContextManager::SetConstantBuffer(0, Gfx::Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_DrawCallConstantBuffer);
        
        const unsigned int Offset = 0;
        ContextManager::SetVertexBufferSet(m_CameraMesh->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), &Offset);
        ContextManager::SetIndexBuffer(m_CameraMesh->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);

        ContextManager::SetInputLayout(m_InputLayout);
                
        ContextManager::SetTopology(STopology::TriangleList);
        ContextManager::DrawIndexed(m_CameraMesh->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
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

