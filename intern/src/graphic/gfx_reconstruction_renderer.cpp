
#include "graphic/gfx_precompiled.h"

#include "base/base_vector3.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "camera/cam_control_manager.h"
#include "camera/cam_editor_control.h"

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
#include "mr/mr_scalable_slam_reconstructor.h"

#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

using namespace Gfx;

namespace
{
	struct SDrawCallConstantBuffer
	{
		Base::Float4x4 m_WorldMatrix;
		Base::Float4 m_Color;
	};

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
        void PauseIntegration(bool _Paused);
        void PauseTracking(bool _Paused);
        void ChangeCamera(bool _IsTrackingCamera);

        void OnReconstructionUpdate(const MR::SReconstructionSettings& _Settings);

    private:

		void RenderVolume();

		void RenderScalableVolume();
		void RenderGrid();

        void RenderCamera();
        
    private:

		std::unique_ptr<MR::CSLAMReconstructor> m_pReconstructor;
		std::unique_ptr<MR::CScalableSLAMReconstructor> m_pScalableReconstructor;
        
        CShaderPtr m_OutlineVSPtr;
        CShaderPtr m_OutlineFSPtr;
        CShaderPtr m_RaycastVSPtr;
        CShaderPtr m_RaycastFSPtr;

        CBufferPtr m_RaycastConstantBufferPtr;
        CBufferPtr m_DrawCallConstantBufferPtr;
        
        CMeshPtr m_CameraMeshPtr;
		CInputLayoutPtr m_CameraInputLayoutPtr;

		CMeshPtr m_CubeOutlineMeshPtr;
		CInputLayoutPtr m_CubeOutlineInputLayoutPtr;

        CMeshPtr m_VolumeMeshPtr;        
        CInputLayoutPtr m_VolumeInputLayoutPtr;

        CRenderContextPtr m_OutlineRenderContextPtr;

        bool m_UseTrackingCamera;
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
        
		MR::SReconstructionSettings DefaultSettings;

		if (DefaultSettings.m_IsScalable)
		{			
			m_pScalableReconstructor.reset(new MR::CScalableSLAMReconstructor);
			m_pReconstructor = nullptr;
		}
		else
		{
			m_pReconstructor.reset(new MR::CSLAMReconstructor);
			m_pScalableReconstructor = nullptr;
		}

        m_UseTrackingCamera = true;
    }

    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnExit()
    {
        m_OutlineVSPtr = 0;
        m_OutlineFSPtr = 0;
        m_RaycastVSPtr = 0;
        m_RaycastFSPtr = 0;
        
        m_RaycastConstantBufferPtr = 0;
        m_DrawCallConstantBufferPtr = 0;
        
        m_CameraMeshPtr = 0;
        m_VolumeMeshPtr = 0;
		m_CubeOutlineMeshPtr = 0;
        m_CameraInputLayoutPtr = 0;
        m_VolumeInputLayoutPtr = 0;
		m_CubeOutlineInputLayoutPtr = 0;

        m_OutlineRenderContextPtr = 0;

		m_pReconstructor = nullptr;
		m_pScalableReconstructor = nullptr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupShader()
    {
		MR::SReconstructionSettings Settings;

		if (m_pScalableReconstructor != nullptr)
		{
			m_pScalableReconstructor->GetReconstructionSettings(&Settings);

			std::stringstream DefineStream;

			DefineStream
				<< "#define VOLUME_RESOLUTION "  << Settings.m_VolumeResolution << " \n"
				<< "#define TRUNCATED_DISTANCE " << Settings.m_TruncatedDistance << " \n"
				<< "#define VOLUME_SIZE "        << Settings.m_VolumeSize << " \n"
				<< "#define VOXEL_SIZE "         << Settings.m_VolumeSize / Settings.m_VolumeResolution << " \n";

			if (Settings.m_CaptureColor)
			{
				DefineStream << "#define CAPTURE_COLOR\n";
			}

			std::string DefineString = DefineStream.str();

			m_OutlineVSPtr = ShaderManager::CompileVS("scalable_kinect_fusion\\vs_outline.glsl", "main", DefineString.c_str());
			m_OutlineFSPtr = ShaderManager::CompilePS("scalable_kinect_fusion\\fs_outline.glsl", "main", DefineString.c_str());
			m_RaycastVSPtr = ShaderManager::CompileVS("scalable_kinect_fusion\\vs_raycast.glsl"  , "main", DefineString.c_str());
			m_RaycastFSPtr = ShaderManager::CompilePS("scalable_kinect_fusion\\fs_raycast.glsl"  , "main", DefineString.c_str());
        }
		else
		{
			m_pReconstructor->GetReconstructionSettings(&Settings);

			std::stringstream DefineStream;

			DefineStream
				<< "#define VOLUME_RESOLUTION "  << Settings.m_VolumeResolution << " \n"
				<< "#define TRUNCATED_DISTANCE " << Settings.m_TruncatedDistance << " \n"
				<< "#define VOLUME_SIZE "        << Settings.m_VolumeSize << " \n"
				<< "#define VOXEL_SIZE "         << Settings.m_VolumeSize / Settings.m_VolumeResolution << " \n";

			if (Settings.m_CaptureColor)
			{
				DefineStream << "#define CAPTURE_COLOR\n";
			}

			std::string DefineString = DefineStream.str();

			m_OutlineVSPtr = ShaderManager::CompileVS("kinect_fusion\\vs_outline.glsl", "main", DefineString.c_str());
			m_OutlineFSPtr = ShaderManager::CompilePS("kinect_fusion\\fs_outline.glsl", "main", DefineString.c_str());
			m_RaycastVSPtr = ShaderManager::CompileVS("kinect_fusion\\vs_raycast.glsl"  , "main", DefineString.c_str());
			m_RaycastFSPtr = ShaderManager::CompilePS("kinect_fusion\\fs_raycast.glsl"  , "main", DefineString.c_str());
		}
        
        SInputElementDescriptor InputLayoutDesc = {};

        InputLayoutDesc.m_pSemanticName        = "POSITION";
        InputLayoutDesc.m_SemanticIndex        = 0;
        InputLayoutDesc.m_Format               = CInputLayout::Float3Format;
        InputLayoutDesc.m_InputSlot            = 0;
        InputLayoutDesc.m_AlignedByteOffset    = 0;
        InputLayoutDesc.m_Stride               = 12;
        InputLayoutDesc.m_InputSlotClass       = CInputLayout::PerVertex;
        InputLayoutDesc.m_InstanceDataStepRate = 0;

        m_CameraInputLayoutPtr = ShaderManager::CreateInputLayout(&InputLayoutDesc, 1, m_OutlineVSPtr);
        m_VolumeInputLayoutPtr = ShaderManager::CreateInputLayout(&InputLayoutDesc, 1, m_RaycastVSPtr);
		m_CubeOutlineInputLayoutPtr = ShaderManager::CreateInputLayout(&InputLayoutDesc, 1, m_OutlineVSPtr);
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
        m_OutlineRenderContextPtr = ContextManager::CreateRenderContext();

        m_OutlineRenderContextPtr->SetCamera(ViewManager::GetMainCamera());
        m_OutlineRenderContextPtr->SetViewPortSet(ViewManager::GetViewPortSet());
        m_OutlineRenderContextPtr->SetTargetSet(TargetSetManager::GetDeferredTargetSet());
        m_OutlineRenderContextPtr->SetRenderState(StateManager::GetRenderState(CRenderState::NoCull | CRenderState::Wireframe));
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
        
        ConstantBufferDesc.m_NumberOfBytes = sizeof(SDrawCallConstantBuffer);

        m_DrawCallConstantBufferPtr = BufferManager::CreateBuffer(ConstantBufferDesc);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupResources()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupModels()
    {
		//Todo: remove magic numbers (focal length/point, max/min depth)

		float x = (-0.50602675f) / 0.72113f;
		float y = (-0.499133f) / 0.870799f;
		
        Float3 CameraVertices[] =
        {
            Float3(-x * 8.0f, -y * 8.0f, 8.0f),
            Float3( x * 8.0f, -y * 8.0f, 8.0f),
            Float3( x * 8.0f,  y * 8.0f, 8.0f),
            Float3(-x * 8.0f,  y * 8.0f, 8.0f),
            Float3( 0.0f,  0.0f, 0.0f),
			Float3(-x * 0.5f, -y * 0.5f, 0.5f),
			Float3( x * 0.5f, -y * 0.5f, 0.5f),
			Float3( x * 0.5f,  y * 0.5f, 0.5f),
			Float3(-x * 0.5f,  y * 0.5f, 0.5f),
        };

		Float3 CameraLines[24] =
        {
			CameraVertices[4], CameraVertices[0],
			CameraVertices[4], CameraVertices[1],
			CameraVertices[4], CameraVertices[2],
			CameraVertices[4], CameraVertices[3],

			CameraVertices[0], CameraVertices[1],
			CameraVertices[1], CameraVertices[2],
			CameraVertices[2], CameraVertices[3],
			CameraVertices[3], CameraVertices[0],

			CameraVertices[5], CameraVertices[6],
			CameraVertices[6], CameraVertices[7],
			CameraVertices[7], CameraVertices[8],
			CameraVertices[8], CameraVertices[5],
        };
                
        Dt::CSurface* pSurface = new Dt::CSurface;
        Dt::CLOD* pLOD = new Dt::CLOD;
        Dt::CMesh* pMesh = new Dt::CMesh;

        pSurface->SetPositions(CameraLines);
        pSurface->SetNumberOfVertices(sizeof(CameraLines) / sizeof(CameraLines[0]));
        pSurface->SetIndices(nullptr);
        pSurface->SetNumberOfIndices(0);
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

		Float3 CubeLines[24] =
		{
			CubeVertices[0], CubeVertices[1],
			CubeVertices[1], CubeVertices[2],
			CubeVertices[2], CubeVertices[3],
			CubeVertices[3], CubeVertices[0],

			CubeVertices[0], CubeVertices[4],
			CubeVertices[1], CubeVertices[5],
			CubeVertices[2], CubeVertices[6],
			CubeVertices[3], CubeVertices[7],

			CubeVertices[4], CubeVertices[5],
			CubeVertices[5], CubeVertices[6],
			CubeVertices[6], CubeVertices[7],
			CubeVertices[7], CubeVertices[4],
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

        MeshDesc.m_pMesh = pMesh;

        m_VolumeMeshPtr = MeshManager::CreateMesh(MeshDesc);

		pSurface = new Dt::CSurface;
		pLOD = new Dt::CLOD;
		pMesh = new Dt::CMesh;

		pSurface->SetPositions(CubeLines);
		pSurface->SetNumberOfVertices(sizeof(CubeLines) / sizeof(CubeLines[0]));
		pSurface->SetIndices(nullptr);
		pSurface->SetNumberOfIndices(0);
		pSurface->SetElements(0);

		pLOD->SetSurface(0, pSurface);
		pLOD->SetNumberOfSurfaces(1);

		pMesh->SetLOD(0, pLOD);
		pMesh->SetNumberOfLODs(1);

		MeshDesc.m_pMesh = pMesh;

		m_CubeOutlineMeshPtr = MeshManager::CreateMesh(MeshDesc);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnSetupEnd()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::OnReload()
    {
        if (m_pScalableReconstructor != nullptr)
        {
			m_pScalableReconstructor->ResetReconstruction();
        }
		else
		{
			m_pReconstructor->ResetReconstruction();
		}

        OnSetupShader();
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::OnReconstructionUpdate(const MR::SReconstructionSettings& _Settings)
    {
		if (_Settings.m_IsScalable && m_pScalableReconstructor == nullptr)
		{
			m_pReconstructor = nullptr;
			m_pScalableReconstructor.reset(new MR::CScalableSLAMReconstructor);			
		}
		else if (!_Settings.m_IsScalable && m_pReconstructor == nullptr)
		{
			m_pScalableReconstructor = nullptr;
			m_pReconstructor.reset(new MR::CSLAMReconstructor);
		}

		if (m_pScalableReconstructor != nullptr)
		{
			m_pScalableReconstructor->ResetReconstruction(&_Settings);
		}
		else
		{
			m_pReconstructor->ResetReconstruction(&_Settings);
		}

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
        if (m_UseTrackingCamera)
        {
            Cam::CControl& rControl = static_cast<Cam::CEditorControl&>(Cam::ControlManager::GetActiveControl());
            
            Float4x4 PoseMatrix = (m_pScalableReconstructor != nullptr) ? m_pScalableReconstructor->GetPoseMatrix() : m_pReconstructor->GetPoseMatrix();

            Base::Float3 Position;
            Base::Float3 Rotation;

            PoseMatrix.GetTranslation(Position);
            PoseMatrix.GetRotation(Rotation);

            Base::Float3x3 RotationMatrix;
            RotationMatrix.SetRotation(Rotation[0] + 3.14f, Rotation[1], Rotation[2]);            

            rControl.SetPosition(Position);
            rControl.SetRotation(RotationMatrix);
        }        
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxReconstructionRenderer::RenderVolume()
    {
        MR::SReconstructionSettings Settings;
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
        ContextManager::SetVertexBufferSet(m_VolumeMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), &Offset);
        ContextManager::SetIndexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);
        ContextManager::SetInputLayout(m_VolumeInputLayoutPtr);
        
        ContextManager::SetTopology(STopology::TriangleList);

        ContextManager::DrawIndexed(36, 0, 0);

		// Render volume box

		ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Wireframe));

		ContextManager::SetRenderContext(m_OutlineRenderContextPtr);
		ContextManager::SetShaderVS(m_OutlineVSPtr);
		ContextManager::SetShaderPS(m_OutlineFSPtr);

		SDrawCallConstantBuffer BufferData;

		BufferData.m_WorldMatrix.SetScale(Settings.m_VolumeSize);
		BufferData.m_Color = Float4(0.0f, 0.0f, 1.0f, 1.0f);

		BufferManager::UploadConstantBufferData(m_DrawCallConstantBufferPtr, &BufferData);

		ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
		ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);
		
		ContextManager::SetVertexBufferSet(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), &Offset);
		ContextManager::SetInputLayout(m_CubeOutlineInputLayoutPtr);

		ContextManager::SetTopology(STopology::LineList);

		ContextManager::Draw(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfVertices(), 0);
    }

	// -----------------------------------------------------------------------------

	void CGfxReconstructionRenderer::RenderScalableVolume()
	{
		MR::SReconstructionSettings Settings;
		m_pScalableReconstructor->GetReconstructionSettings(&Settings);

		Float4x4 PoseMatrix = m_pScalableReconstructor->GetPoseMatrix();

		Float4 RaycastData[2];
		if (Settings.m_CaptureColor)
		{
			RaycastData[1] = m_pScalableReconstructor->IsTrackingLost() ? Float4(1.0f, 0.0f, 0.0f, 1.0f) : Float4(0.0f, 0.0f, 0.0f, 1.0f);
		}
		else
		{
			RaycastData[1] = m_pScalableReconstructor->IsTrackingLost() ? Float4(1.0f, 0.0f, 0.0f, 1.0f) : Float4(0.0f, 1.0f, 0.0f, 1.0f);
		}

		ContextManager::SetShaderVS(m_RaycastVSPtr);
		ContextManager::SetShaderPS(m_RaycastFSPtr);

		ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
		ContextManager::SetConstantBuffer(1, m_RaycastConstantBufferPtr);

		ContextManager::Barrier();

		ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::Default));
		ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

		const unsigned int Offset = 0;
		ContextManager::SetVertexBufferSet(m_VolumeMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), &Offset);
		ContextManager::SetIndexBuffer(m_VolumeMeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);
		ContextManager::SetInputLayout(m_VolumeInputLayoutPtr);

		ContextManager::SetTopology(STopology::TriangleList);

		for (auto& rPair : m_pScalableReconstructor->GetRootGrids())
		{
			auto& rRootGrid = rPair.second;
						
			RaycastData[0][0] = rRootGrid.m_Offset[0] * Settings.m_VolumeSize;
			RaycastData[0][1] = rRootGrid.m_Offset[1] * Settings.m_VolumeSize;
			RaycastData[0][2] = rRootGrid.m_Offset[2] * Settings.m_VolumeSize;
			
			BufferManager::UploadConstantBufferData(m_RaycastConstantBufferPtr, RaycastData);

			ContextManager::SetTexture(0, static_cast<CTextureBasePtr>(rRootGrid.m_TSDFVolumePtr));
			ContextManager::SetSampler(0, SamplerManager::GetSampler(CSampler::ESampler::MinMagMipLinearClamp));

			if (Settings.m_CaptureColor)
			{
				ContextManager::SetTexture(1, static_cast<CTextureBasePtr>(rRootGrid.m_ColorVolumePtr));
				ContextManager::SetSampler(1, SamplerManager::GetSampler(CSampler::ESampler::MinMagMipLinearClamp));
			}

			ContextManager::DrawIndexed(36, 0, 0);
		}
	}

	// -----------------------------------------------------------------------------

	void CGfxReconstructionRenderer::RenderGrid()
	{
		MR::SReconstructionSettings Settings;
		m_pScalableReconstructor->GetReconstructionSettings(&Settings);

		ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

		ContextManager::SetRenderContext(m_OutlineRenderContextPtr);
		ContextManager::SetShaderVS(m_OutlineVSPtr);
		ContextManager::SetShaderPS(m_OutlineFSPtr);

		ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
		ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);

		SDrawCallConstantBuffer BufferData;

		const unsigned int Offset = 0;
		ContextManager::SetVertexBufferSet(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), &Offset);
		ContextManager::SetInputLayout(m_CubeOutlineInputLayoutPtr);

		ContextManager::SetTopology(STopology::LineList);

		Float3 Position;
		Float4x4 Scaling;
		Float4x4 Translation;

		for (auto& rPair : m_pScalableReconstructor->GetRootGrids())
		{
			auto& rRootGrid = rPair.second;

			Position[0] = static_cast<float>(rRootGrid.m_Offset[0]);
			Position[1] = static_cast<float>(rRootGrid.m_Offset[1]);
			Position[2] = static_cast<float>(rRootGrid.m_Offset[2]);

			Position = Position * Settings.m_VolumeSize;
			
			Scaling.SetScale(Settings.m_VolumeSize);
			Translation.SetTranslation(Position);

			BufferData.m_WorldMatrix = Translation * Scaling;
			BufferData.m_Color = Float4(0.0f, 0.0f, 1.0f, 1.0f);

			BufferManager::UploadConstantBufferData(m_DrawCallConstantBufferPtr, &BufferData);

			ContextManager::Draw(m_CubeOutlineMeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfVertices(), 0);
		}
	}

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::RenderCamera()
    {
        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));

        ContextManager::SetRenderContext(m_OutlineRenderContextPtr);
        ContextManager::SetShaderVS(m_OutlineVSPtr);
        ContextManager::SetShaderPS(m_OutlineFSPtr);

		SDrawCallConstantBuffer BufferData;

		BufferData.m_WorldMatrix = (m_pScalableReconstructor != nullptr) ? m_pScalableReconstructor->GetPoseMatrix() : m_pReconstructor->GetPoseMatrix();
		BufferData.m_Color = Float4(1.0f, 0.0f, 1.0f, 1.0f);

		BufferManager::UploadConstantBufferData(m_DrawCallConstantBufferPtr, &BufferData);

        ContextManager::SetConstantBuffer(0, Main::GetPerFrameConstantBuffer());
        ContextManager::SetConstantBuffer(1, m_DrawCallConstantBufferPtr);

        const unsigned int Offset = 0;
        ContextManager::SetVertexBufferSet(m_CameraMeshPtr->GetLOD(0)->GetSurface(0)->GetVertexBuffer(), &Offset);
        ContextManager::SetIndexBuffer(m_CameraMeshPtr->GetLOD(0)->GetSurface(0)->GetIndexBuffer(), Offset);

        ContextManager::SetInputLayout(m_CameraInputLayoutPtr);
        ContextManager::SetTopology(STopology::LineList);

        ContextManager::Draw(m_CameraMeshPtr->GetLOD(0)->GetSurface(0)->GetNumberOfVertices(), 0);
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::Render()
    {
		if (m_pScalableReconstructor != nullptr)
		{
			m_pScalableReconstructor->Update();
		}
		else
		{
			m_pReconstructor->Update();
		}        

        Performance::BeginEvent("SLAM Reconstruction Rendering");
        
        ContextManager::SetTargetSet(TargetSetManager::GetDeferredTargetSet());
        //Base::Float4 ClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        //TargetSetManager::ClearTargetSet(TargetSetManager::GetDeferredTargetSet(), ClearColor);

		if (m_pScalableReconstructor != nullptr)
		{
			RenderScalableVolume();
			RenderGrid();
		}
		else
		{
			RenderVolume();
		}

        if (!m_UseTrackingCamera)
        {
            RenderCamera();
        }
        
        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::PauseIntegration(bool _Paused)
    {
		if (m_pScalableReconstructor != nullptr)
		{
			m_pScalableReconstructor->PauseIntegration(_Paused);
		}
		else
		{
			m_pReconstructor->PauseIntegration(_Paused);
		}
    }
    
    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::PauseTracking(bool _Paused)
    {
		if (m_pScalableReconstructor != nullptr)
		{
			m_pScalableReconstructor->PauseTracking(_Paused);
		}
		else
		{
			m_pReconstructor->PauseTracking(_Paused);
		}        
    }

    // -----------------------------------------------------------------------------

    void CGfxReconstructionRenderer::ChangeCamera(bool _IsTrackingCamera)
    {
        m_UseTrackingCamera = _IsTrackingCamera;
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

    void OnReconstructionUpdate(const MR::SReconstructionSettings& _Settings)
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

    void PauseIntegration(bool _Paused)
    {
        CGfxReconstructionRenderer::GetInstance().PauseIntegration(_Paused);
    }

    // -----------------------------------------------------------------------------

    void PauseTracking(bool _Paused)
    {
        CGfxReconstructionRenderer::GetInstance().PauseTracking(_Paused);
    }

    // -----------------------------------------------------------------------------

    void ChangeCamera(bool _IsTrackingCamera)
    {
        CGfxReconstructionRenderer::GetInstance().ChangeCamera(_IsTrackingCamera);
    }

} // namespace Voxel
} // namespace Gfx

