
#include "graphic/gfx_precompiled.h"

#include "base/base_vector3.h"
#include "base/base_matrix4x4.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_time.h"

#include "data/data_map.h"

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

// only for testing
// http://www.busydevelopers.com/article/44073720/What+does+the+unsgned+short+value+in+INuiFusionColorReconstruction%3A%3AExportVolumeBlock+represent%3F

#include "base/base_console.h"
#include "gfx_native_buffer.h"
#include "gfx_native_shader.h"
#include "gfx_native_target_set.h"
#include "mr/mr_kinect_control.h"

#include <gl/glew.h>
#include <gl/wglew.h>

#include <atomic>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

using namespace Gfx;

namespace
{
    struct SDrawCallBufferData
    {
        Base::Float4x4 m_WorldMatrix;
        Base::Float4x4 m_WorldToCameraMatrix;
    };
    
    const unsigned int TileSize = 16;

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

		GLuint m_DrawCallConstantBuffer;

        CShaderPtr m_VertexShader;
        CShaderPtr m_FragmentShader;
        CShaderPtr m_ComputeShader;

        GLuint m_KinectDepthBuffer[2];
        
		MR::CKinectControl m_KinectControl;
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

        m_KinectControl.Start();
    }

    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnExit()
    {
        m_VertexShader = 0;
        m_FragmentShader = 0;
        m_ComputeShader = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupShader()
    {
        std::stringstream TileSizeDefineStream;
        TileSizeDefineStream << "TILE_SIZE " << TileSize << '\n';
        std::string TileSizeDefine = TileSizeDefineStream.str();
        const char* pDefines[] = { TileSizeDefine.c_str() };

        m_VertexShader = ShaderManager::CompileVS("vs_kinect.glsl", "main");
        m_FragmentShader = ShaderManager::CompilePS("fs_kinect.glsl", "main");
        m_ComputeShader = ShaderManager::CompileCS("cs_kinect.glsl", "main", 1, pDefines);
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
        glCreateTextures(GL_TEXTURE_2D, 2, m_KinectDepthBuffer);
		        
		glTextureStorage2D(m_KinectDepthBuffer[0], 1, GL_R16, MR::CKinectControl::DepthImageWidth, MR::CKinectControl::DepthImageHeight);
		glTextureParameteri(m_KinectDepthBuffer[0], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_KinectDepthBuffer[0], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_KinectDepthBuffer[0], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_KinectDepthBuffer[0], GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTextureStorage2D(m_KinectDepthBuffer[1], 1, GL_R16, MR::CKinectControl::DepthImageWidth, MR::CKinectControl::DepthImageHeight);
		glTextureParameteri(m_KinectDepthBuffer[1], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_KinectDepthBuffer[1], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_KinectDepthBuffer[1], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_KinectDepthBuffer[1], GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        /*glGenTextures(1, &m_VoxelDataBuffer);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, m_VoxelDataBuffer);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexImage3D(GL_TEXTURE_3D, 0, GL_R16, CubeVoxelWidth, CubeVoxelWidth, CubeVoxelWidth, 0, GL_RED, GL_SHORT, nullptr);

		const unsigned int VoxelCount = CubeVoxelWidth * CubeVoxelWidth * CubeVoxelWidth;

		std::vector<float> VoxelData(VoxelCount);

        std::fstream VoxelFile;
        VoxelFile.open("kinect_voxel_data.txt", std::ios::in);

        short Value;
		unsigned int Index = 0;
        while (VoxelFile >> Value)
        {
			VoxelData[Index++] = Value < 0 ? 0.0f : 1.0f;
        }

        VoxelFile.close();

		glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, CubeVoxelWidth, CubeVoxelWidth, CubeVoxelWidth, 0, GL_RED, GL_FLOAT, VoxelData.data());*/
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupBuffers()
    {
        glGenBuffers(1, &m_DrawCallConstantBuffer);
        glNamedBufferData(m_DrawCallConstantBuffer, sizeof(SDrawCallBufferData), nullptr, GL_DYNAMIC_DRAW);
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

        std::vector<unsigned short> DepthPixels(MR::CKinectControl::DepthImagePixelsCount);

        if (m_KinectControl.GetDepthBuffer(DepthPixels.data()))
        {
            glTextureSubImage2D(m_KinectDepthBuffer[0], 0, 0, 0,
                MR::CKinectControl::DepthImageWidth, MR::CKinectControl::DepthImageHeight,
                GL_RED, GL_UNSIGNED_SHORT, DepthPixels.data());
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::Render()
    {
        Performance::BeginEvent("Voxel Rendering");

        //////////////////////////////////////////////////////////////////////////////////////
        // Compute shader
        //////////////////////////////////////////////////////////////////////////////////////
        
        const int WorkGroupsX = (MR::CKinectControl::DepthImageWidth / TileSize) + 1;
        const int WorkGroupsY = (MR::CKinectControl::DepthImageHeight / TileSize) + 1;

        Gfx::ContextManager::SetShaderCS(m_ComputeShader);
        glBindImageTexture(0, m_KinectDepthBuffer[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
        glBindImageTexture(1, m_KinectDepthBuffer[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16UI);
        glDispatchCompute(WorkGroupsX, WorkGroupsY, 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        //////////////////////////////////////////////////////////////////////////////////////
        // Rendering
        //////////////////////////////////////////////////////////////////////////////////////

        CTargetSetPtr DefaultTargetSetPtr = TargetSetManager::GetDefaultTargetSet();
        CNativeTargetSet NativeTargetSet = *static_cast<CNativeTargetSet*>(DefaultTargetSetPtr.GetPtr());

        glBindFramebuffer(GL_FRAMEBUFFER, NativeTargetSet.m_NativeTargetSet);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        Gfx::ContextManager::SetShaderVS(m_VertexShader);
        Gfx::ContextManager::SetShaderPS(m_FragmentShader);

        glBindVertexArray(1);

        glViewport(0, 0, 640, 720);
        glBindImageTexture(0, m_KinectDepthBuffer[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
        glViewport(640, 0, 640, 720);
        glBindImageTexture(0, m_KinectDepthBuffer[1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
        
        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        Performance::EndEvent();
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

