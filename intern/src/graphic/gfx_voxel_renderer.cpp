
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
    const float g_KinectFocalLengthX = 0.72113f;
    const float g_KinectFocalLengthY = 0.870799f;
    const float g_KinectFocalPointX = 0.50602675f;
    const float g_KinectFocalPointY = 0.499133f;

	const int g_PyramidLevels = 3;

	const int g_VolumeSize = 256;

    struct SIntrinsics
    {
        Base::Float2 m_FocalPoint;
        Base::Float2 m_FocalLength;
        Base::Float2 m_InvFocalLength;
        Base::Float2 Padding;
    };

    struct SDrawCallBufferData
    {
        Base::Float4x4 m_WorldMatrix;
    };
    
    const unsigned int TileSize2D = 16;
	const unsigned int TileSize3D = 8;

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

		void ReadKinectData();
		void PerformTracking();

		void RenderDepth();
		void RenderVertexMap();
		void RenderVolume();

	private:

		int m_CurrentKinectFrameIndex = 0;

        GLuint m_DrawCallConstantBuffer;
        GLuint m_IntrinsicsConstantBuffer;

        CShaderPtr m_VSVisualizeDepth;
        CShaderPtr m_FSVisualizeDepth;
        CShaderPtr m_VSVisualizeVertexMap;
        CShaderPtr m_FSVisualizeVertexMap;
		CShaderPtr m_VSVisualizeVolume;
		CShaderPtr m_FSVisualizeVolume;

        CShaderPtr m_CSBilateralFilter;
        CShaderPtr m_CSVertexMap;
        CShaderPtr m_CSNormalMap;
		CShaderPtr m_CSDownSample;
		CShaderPtr m_CSVolume;

		GLuint m_KinectRawDepthBuffer;
        GLuint m_KinectSmoothDepthBuffer[g_PyramidLevels][2];
        GLuint m_KinectVertexMap[g_PyramidLevels][2];
        GLuint m_KinectNormalMap[g_PyramidLevels][2];

		GLuint m_Volume;

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

		m_CurrentKinectFrameIndex = 0;

        m_KinectControl.Start();
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

        m_CSBilateralFilter = 0;
        m_CSVertexMap = 0;
        m_CSNormalMap = 0;
		m_CSDownSample = 0;
		m_CSVolume = 0;

        glDeleteTextures(1, &m_KinectRawDepthBuffer);
        glDeleteTextures(g_PyramidLevels * 2, m_KinectSmoothDepthBuffer[0]);
        glDeleteTextures(g_PyramidLevels * 2, m_KinectVertexMap[0]);
        glDeleteTextures(g_PyramidLevels * 2, m_KinectNormalMap[0]);
		glDeleteTextures(1, &m_Volume);

        glDeleteBuffers(1, &m_DrawCallConstantBuffer);
        glDeleteBuffers(1, &m_IntrinsicsConstantBuffer);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupShader()
    {
        std::stringstream TileSizeDefineStream;
        TileSizeDefineStream << "TILE_SIZE " << TileSize2D << '\n';
        std::string TileSizeDefine = TileSizeDefineStream.str();
        const char* pDefines[] = { TileSizeDefine.c_str() };

        m_VSVisualizeDepth = ShaderManager::CompileVS("vs_kinect_visualize_depth.glsl", "main");
        m_FSVisualizeDepth = ShaderManager::CompilePS("fs_kinect_visualize_depth.glsl", "main");
        m_VSVisualizeVertexMap = ShaderManager::CompileVS("vs_kinect_visualize_vertex_map.glsl", "main");
        m_FSVisualizeVertexMap = ShaderManager::CompilePS("fs_kinect_visualize_vertex_map.glsl", "main");
		m_VSVisualizeVolume = ShaderManager::CompileVS("vs_kinect_visualize_volume.glsl", "main");
		m_FSVisualizeVolume = ShaderManager::CompilePS("fs_kinect_visualize_volume.glsl", "main");

        m_CSBilateralFilter = ShaderManager::CompileCS("cs_kinect_bilateral_filter.glsl", "main", 1, pDefines);
        m_CSVertexMap = ShaderManager::CompileCS("cs_kinect_vertex_map.glsl", "main", 1, pDefines);
        m_CSNormalMap = ShaderManager::CompileCS("cs_kinect_normal_map.glsl", "main", 1, pDefines);
		m_CSDownSample = ShaderManager::CompileCS("cs_kinect_downsample.glsl", "main", 1, pDefines);

		TileSizeDefineStream.str("");
		TileSizeDefineStream << "TILE_SIZE " << TileSize3D << '\n';
		TileSizeDefine = TileSizeDefineStream.str();
		pDefines[0] = { TileSizeDefine.c_str() };

		m_CSVolume = ShaderManager::CompileCS("cs_kinect_volume.glsl", "main", 1, pDefines);
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
        glCreateTextures(GL_TEXTURE_2D, g_PyramidLevels * 2, m_KinectSmoothDepthBuffer[0]);
        glCreateTextures(GL_TEXTURE_2D, g_PyramidLevels * 2, m_KinectVertexMap[0]);
        glCreateTextures(GL_TEXTURE_2D, g_PyramidLevels * 2, m_KinectNormalMap[0]);
		glCreateTextures(GL_TEXTURE_3D, 1, &m_Volume);

        glTextureStorage2D(m_KinectRawDepthBuffer, 1, GL_R16UI, MR::CKinectControl::DepthImageWidth, MR::CKinectControl::DepthImageHeight);
        
		for (int i = 0; i < g_PyramidLevels; ++ i)
		{
			const int Width = MR::CKinectControl::DepthImageWidth >> i;
			const int Height = MR::CKinectControl::DepthImageHeight >> i;

			for (int j = 0; j < 2; ++ j)
			{
				glTextureStorage2D(m_KinectSmoothDepthBuffer[i][j], 1, GL_R16UI, Width, Height);
				glTextureStorage2D(m_KinectVertexMap[i][j], 1, GL_RGBA32F, Width, Height);
				glTextureStorage2D(m_KinectNormalMap[i][j], 1, GL_RGBA32F, Width, Height);
			}
		}

		glTextureStorage3D(m_Volume, 1, GL_RG16UI, g_VolumeSize, g_VolumeSize, g_VolumeSize);
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupBuffers()
    {
        Base::Float4x4 WorldMatrix;
        Base::Float4x4 RotationMatrix;
        Base::Float4x4 ScalingMatrix;

        RotationMatrix.SetRotationX(3.14f);
        ScalingMatrix.SetScale(0.005f);

        WorldMatrix = RotationMatrix * ScalingMatrix;

        glCreateBuffers(1, &m_DrawCallConstantBuffer);
        glNamedBufferData(m_DrawCallConstantBuffer, sizeof(SDrawCallBufferData), &WorldMatrix, GL_DYNAMIC_DRAW);

        SIntrinsics Intrinsics;

        Intrinsics.m_FocalPoint = Base::Float2(g_KinectFocalPointX, g_KinectFocalPointY);
        Intrinsics.m_FocalLength = Base::Float2(g_KinectFocalLengthX, g_KinectFocalLengthY);
        Intrinsics.m_InvFocalLength = Base::Float2(1.0f / g_KinectFocalLengthX, 1.0f / g_KinectFocalLengthY);

        glCreateBuffers(1, &m_IntrinsicsConstantBuffer);
        glNamedBufferData(m_IntrinsicsConstantBuffer, sizeof(SIntrinsics), &Intrinsics, GL_STATIC_DRAW);
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
            glTextureSubImage2D(m_KinectRawDepthBuffer, 0, 0, 0,
                MR::CKinectControl::DepthImageWidth, MR::CKinectControl::DepthImageHeight,
                GL_RED_INTEGER, GL_UNSIGNED_SHORT, DepthPixels.data());
        }
    }
    
    // -----------------------------------------------------------------------------
    
	void CGfxVoxelRenderer::ReadKinectData()
	{
		m_CurrentKinectFrameIndex = (m_CurrentKinectFrameIndex + 1) % 2;

		const int WorkGroupsX = (MR::CKinectControl::DepthImageWidth / TileSize2D);
		const int WorkGroupsY = (MR::CKinectControl::DepthImageHeight / TileSize2D);

		//////////////////////////////////////////////////////////////////////////////////////
		// Bilateral Filter
		//////////////////////////////////////////////////////////////////////////////////////

		Gfx::ContextManager::SetShaderCS(m_CSBilateralFilter);
		glBindImageTexture(0, m_KinectRawDepthBuffer, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
		glBindImageTexture(1, m_KinectSmoothDepthBuffer[0][m_CurrentKinectFrameIndex], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16UI);
		glDispatchCompute(WorkGroupsX, WorkGroupsY, 1);

		for (int PyramidLevel = 0; PyramidLevel < g_PyramidLevels - 1; ++PyramidLevel)
		{
			//////////////////////////////////////////////////////////////////////////////////////
			// Downsample depth buffer
			//////////////////////////////////////////////////////////////////////////////////////

			Gfx::ContextManager::SetShaderCS(m_CSDownSample);
			glBindImageTexture(0, m_KinectSmoothDepthBuffer[PyramidLevel][m_CurrentKinectFrameIndex], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
			glBindImageTexture(1, m_KinectSmoothDepthBuffer[PyramidLevel + 1][m_CurrentKinectFrameIndex], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16UI);
			glDispatchCompute(WorkGroupsX >> PyramidLevel, WorkGroupsY >> PyramidLevel, 1);
		}

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_IntrinsicsConstantBuffer);

		for (int PyramidLevel = 0; PyramidLevel < g_PyramidLevels; ++PyramidLevel)
		{
			//////////////////////////////////////////////////////////////////////////////////////
			// Generate vertex and normal map
			//////////////////////////////////////////////////////////////////////////////////////

			Gfx::ContextManager::SetShaderCS(m_CSVertexMap);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glBindImageTexture(0, m_KinectSmoothDepthBuffer[PyramidLevel][m_CurrentKinectFrameIndex], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
			glBindImageTexture(1, m_KinectVertexMap[PyramidLevel][m_CurrentKinectFrameIndex], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glDispatchCompute(WorkGroupsX >> PyramidLevel, WorkGroupsY >> PyramidLevel, 1);			
		}

		for (int PyramidLevel = 0; PyramidLevel < g_PyramidLevels; ++PyramidLevel)
		{
			Gfx::ContextManager::SetShaderCS(m_CSNormalMap);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glBindImageTexture(0, m_KinectVertexMap[PyramidLevel][m_CurrentKinectFrameIndex], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			glBindImageTexture(1, m_KinectNormalMap[PyramidLevel][m_CurrentKinectFrameIndex], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glDispatchCompute(WorkGroupsX >> PyramidLevel, WorkGroupsY >> PyramidLevel, 1);
		}
	}

	// -----------------------------------------------------------------------------

	void CGfxVoxelRenderer::PerformTracking()
	{
		
	}

	// -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::Render()
    {
        Performance::BeginEvent("Kinect Tracking");

		ReadKinectData();
		PerformTracking();

		Gfx::ContextManager::SetShaderCS(m_CSVolume);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glBindImageTexture(0, m_Volume, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG16UI);
		glDispatchCompute(g_VolumeSize / TileSize3D, g_VolumeSize / TileSize3D, g_VolumeSize / TileSize3D);

        //////////////////////////////////////////////////////////////////////////////////////
        // Rendering
        //////////////////////////////////////////////////////////////////////////////////////

        CTargetSetPtr DefaultTargetSetPtr = TargetSetManager::GetDefaultTargetSet();
        CNativeTargetSet NativeTargetSet = *static_cast<CNativeTargetSet*>(DefaultTargetSetPtr.GetPtr());

        glBindFramebuffer(GL_FRAMEBUFFER, NativeTargetSet.m_NativeTargetSet);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
		
		RenderDepth();
		RenderVertexMap();
		RenderVolume();

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
		glBindImageTexture(0, m_KinectSmoothDepthBuffer[0][m_CurrentKinectFrameIndex], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

		glViewport(ViewPort[0], ViewPort[1], ViewPort[2], ViewPort[3]);

		glBindVertexArray(0);
	}

	// -----------------------------------------------------------------------------

	void CGfxVoxelRenderer::RenderVertexMap()
	{
		glBindVertexArray(1); // dummy vao because opengl needs vertex data even when it does not use it

		glBindImageTexture(0, m_KinectVertexMap[0][m_CurrentKinectFrameIndex], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

		CBufferPtr FrameConstantBufferPtr = Gfx::Main::GetPerFrameConstantBufferVS();
		CNativeBuffer NativeBufer = *static_cast<CNativeBuffer*>(FrameConstantBufferPtr.GetPtr());
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, NativeBufer.m_NativeBuffer);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_DrawCallConstantBuffer);

		Gfx::ContextManager::SetShaderVS(m_VSVisualizeVertexMap);
		Gfx::ContextManager::SetShaderPS(m_FSVisualizeVertexMap);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glDrawArrays(GL_POINTS, 0, MR::CKinectControl::DepthImagePixelsCount);

		glBindVertexArray(0);
	}

	// -----------------------------------------------------------------------------

	void CGfxVoxelRenderer::RenderVolume()
	{
		Gfx::ContextManager::SetShaderVS(m_VSVisualizeVolume);
		Gfx::ContextManager::SetShaderPS(m_FSVisualizeVolume);

		CBufferPtr FrameConstantBufferPtr = Gfx::Main::GetPerFrameConstantBufferVS();
		CNativeBuffer NativeBufer = *static_cast<CNativeBuffer*>(FrameConstantBufferPtr.GetPtr());
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, NativeBufer.m_NativeBuffer);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_DrawCallConstantBuffer);

		glBindImageTexture(0, m_Volume, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RG16UI);
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

