
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

#include "mr/mr_slam_reconstructor.h"

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
        void PauseDepthIntegration(bool _Paused);
        void PauseColorIntegration(bool _Paused);
        void PauseTracking(bool _Paused);

        void OnReconstructionUpdate(const MR::CSLAMReconstructor::SReconstructionSettings& _Settings);

    private:

        void RenderVolume();
        void RenderCamera();
                
    private:

        std::unique_ptr<MR::CSLAMReconstructor> m_pReconstructor;
        
        CShaderPtr m_CameraVSPtr;
        CShaderPtr m_CameraFSPtr;
        CShaderPtr m_RaycastVSPtr;
        CShaderPtr m_RaycastFSPtr;

        CBufferPtr m_RaycastConstantBufferPtr;
        CBufferPtr m_DrawCallConstantBufferPtr;
        
        CMeshPtr m_CameraMeshPtr;
        CMeshPtr m_CubeMeshPtr;
        CInputLayoutPtr m_CameraInputLayoutPtr;
        CInputLayoutPtr m_CubeInputLayoutPtr;

        CRenderContextPtr m_WireframeRenderContextPtr;
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
        m_CameraVSPtr = 0;
        m_CameraFSPtr = 0;
        m_RaycastVSPtr = 0;
        m_RaycastFSPtr = 0;
        
        m_RaycastConstantBufferPtr = 0;
        m_DrawCallConstantBufferPtr = 0;
                
        m_CameraMeshPtr = 0;
        m_CubeMeshPtr = 0;
        m_CameraInputLayoutPtr = 0;
        m_CubeInputLayoutPtr = 0;

        m_WireframeRenderContextPtr = 0;

        m_pReconstructor = nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupShader()
    {
        MR::CSLAMReconstructor::SReconstructionSettings Settings;

        m_pReconstructor->GetReconstructionSettings(&Settings);

        std::stringstream DefineStream;

        DefineStream
            << "#define VOLUME_RESOLUTION "  << Settings.m_VolumeResolution                         << " \n"
            << "#define INT16_MAX "          << std::numeric_limits<int16_t>::max()                 << " \n"
            << "#define TRUNCATED_DISTANCE " << Settings.m_TruncatedDistance                        << " \n"
            << "#define VOLUME_SIZE "        << Settings.m_VolumeSize                               << " \n"
            << "#define VOXEL_SIZE "         << Settings.m_VolumeSize / Settings.m_VolumeResolution << " \n";

        if (Settings.m_CaptureColor)
        {
            DefineStream << "#define CAPTURE_COLOR\n";
        }

        std::string DefineString = DefineStream.str();

        m_CameraVSPtr = ShaderManager::CompileVS("kinect_fusion\\vs_camera.glsl", "main", DefineString.c_str());
        m_CameraFSPtr = ShaderManager::CompilePS("kinect_fusion\\fs_camera.glsl", "main", DefineString.c_str());
        m_RaycastVSPtr = ShaderManager::CompileVS("kinect_fusion\\vs_raycast.glsl", "main", DefineString.c_str());
        m_RaycastFSPtr = ShaderManager::CompilePS("kinect_fusion\\fs_raycast.glsl", "main", DefineString.c_str());
        
        SInputElementDescriptor InputLayoutDesc = {};

        InputLayoutDesc.m_pSemanticName        = "POSITION";
        InputLayoutDesc.m_SemanticIndex        = 0;
        InputLayoutDesc.m_Format               = CInputLayout::Float3Format;
        InputLayoutDesc.m_InputSlot            = 0;
        InputLayoutDesc.m_AlignedByteOffset    = 0;
        InputLayoutDesc.m_Stride               = 12;
        InputLayoutDesc.m_InputSlotClass       = CInputLayout::PerVertex;
        InputLayoutDesc.m_InstanceDataStepRate = 0;

        m_CameraInputLayoutPtr = ShaderManager::CreateInputLayout(&InputLayoutDesc, 1, m_CameraVSPtr);
        m_CubeInputLayoutPtr = ShaderManager::CreateInputLayout(&InputLayoutDesc, 1, m_RaycastVSPtr);
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
        m_WireframeRenderContextPtr = ContextManager::CreateRenderContext();

        m_WireframeRenderContextPtr->SetCamera(ViewManager::GetMainCamera());
        m_WireframeRenderContextPtr->SetViewPortSet(ViewManager::GetViewPortSet());
        m_WireframeRenderContextPtr->SetTargetSet(TargetSetManager::GetDeferredTargetSet());
        m_WireframeRenderContextPtr->SetRenderState(StateManager::GetRenderState(CRenderState::NoCull | CRenderState::Wireframe));
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
        
        ConstantBufferDesc.m_NumberOfBytes = sizeof(Float4x4);

        m_DrawCallConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupModels()
    {
        Float3 CameraVertices[] =
        {
            Float3(-1.0f, -0.5f, 2.0f),
            Float3( 1.0f, -0.5f, 2.0f),
            Float3( 1.0f,  0.5f, 2.0f),
            Float3(-1.0f,  0.5f, 2.0f),
            Float3(-0.0f,  0.0f, 0.0f),
        };

        unsigned int CameraIndices[] =
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

        pSurface->SetPositions(CameraVertices);
        pSurface->SetNumberOfVertices(sizeof(CameraVertices) / sizeof(CameraVertices[0]));
        pSurface->SetIndices(CameraIndices);
        pSurface->SetNumberOfIndices(sizeof(CameraIndices) / sizeof(CameraIndices[0]));
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

        pSurface = new Dt::CSurface;
        pLOD = new Dt::CLOD;
        pMesh = new Dt::CMesh;

        pSurface->SetPositions(CubeVertices);
        pSurface->SetNumberOfVertices(sizeof(CubeVertices) / sizeof(CubeVertices[0]));
        pSurface->SetIndices(CubeIndices);
        pSurface->SetNumberOfIndices(sizeof(CubeIndices) / sizeof(CubeIndices[0]));
        pSurface->SetElements(0);

        pLOD->SetSurface(0, pSurface);
        pLOD->SetNumberOfSurfaces(1);

        pMesh->SetLOD(0, pLOD);
        pMesh->SetNumberOfLODs(1);

        MeshDesc.m_pModel = pMesh;

        m_CubeMeshPtr = MeshManager::CreateMesh(MeshDesc);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnReload()
    {
        m_pReconstructor->ResetReconstruction();

        OnSetupShader();
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::OnReconstructionUpdate(const MR::CSLAMReconstructor::SReconstructionSettings& _Settings)
    {
        m_pReconstructor->ResetReconstruction(&_Settings);

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
        MR::CSLAMReconstructor::SReconstructionSettings Settings;
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

        BufferManager::UploadConstantBufferData(m_RaycastConstantBufferPtr, RaycastData);
        
        ContextManager::SetShaderVS(m_RaycastVSPtr);
        ContextManager::SetShaderPS(m_RaycastFSPtr);

        ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(m_pReconstructor->GetTSDFVolume()));
        ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::ESampler::MinMagMipLinearClamp));

        if (Settings.m_CaptureColor)
        {
            ContextManager::SetTexture(1, static_cast<CTextureBasePtr>(m_pReconstructor->GetColorVolume()));
            ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::ESampler::MinMagMipLinearClamp));
        }

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_RaycastConstantBufferPtr);

        ContextManager::Barrier();

        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::Default));
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));
        
        const unsigned int Offset = 0;
        ContextManager::SetVertexBufferSet(m_CubeMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), &Offset);
        ContextManager::SetIndexBuffer(m_CubeMeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);
        ContextManager::SetInputLayout(m_CubeInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::DrawIndexed(36, 0, 0);
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderCamera()
    {
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Wireframe));

        ContextManager::SetRenderContext(m_WireframeRenderContextPtr);
        ContextManager::SetShaderVS(m_CameraVSPtr);
        ContextManager::SetShaderPS(m_CameraFSPtr);

        Float4x4 WorldMatrix;
        WorldMatrix.SetScale(0.1f);
        WorldMatrix = m_pReconstructor->GetPoseMatrix() * WorldMatrix;

        BufferManager::UploadConstantBufferData(m_DrawCallConstantBufferPtr, &WorldMatrix);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);

        const unsigned int Offset = 0;
        ContextManager::SetVertexBufferSet(m_CameraMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), &Offset);
        ContextManager::SetIndexBuffer(m_CameraMeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);

        ContextManager::SetInputLayout(m_CameraInputLayoutPtr);
        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::DrawIndexed(m_CameraMeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfIndices(), 0, 0);
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::Render()
    {
        m_pReconstructor->Update();

        Performance::BeginEvent("SLAM Reconstruction Rendering");
        
        ContextManager::SetTargetSet(TargetSetManager::GetDeferredTargetSet());
        //Base::Float4 ClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        //TargetSetManager::ClearTargetSet(TargetSetManager::GetDeferredTargetSet(), ClearColor);

        RenderVolume();
        RenderCamera();
        
        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::PauseDepthIntegration(bool _Paused)
    {
        m_pReconstructor->PauseDepthIntegration(_Paused);
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::PauseColorIntegration(bool _Paused)
    {
        m_pReconstructor->PauseColorIntegration(_Paused);
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::PauseTracking(bool _Paused)
    {
        m_pReconstructor->PauseTracking(_Paused);
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

    void OnReconstructionUpdate(const MR::CSLAMReconstructor::SReconstructionSettings& _Settings)
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

    void PauseDepthIntegration(bool _Paused)
    {
        CGfxReconstructionRenderer::GetInstance().PauseDepthIntegration(_Paused);
    }

    // -----------------------------------------------------------------------------

    void PauseColorIntegration(bool _Paused)
    {
        CGfxReconstructionRenderer::GetInstance().PauseColorIntegration(_Paused);
    }

    // -----------------------------------------------------------------------------

    void PauseTracking(bool _Paused)
    {
        CGfxReconstructionRenderer::GetInstance().PauseTracking(_Paused);
    }
} // namespace Voxel
} // namespace Gfx

