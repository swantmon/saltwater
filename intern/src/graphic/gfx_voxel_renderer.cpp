
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
    };

    const float CubeWidth = 15.0f;

    const int CubeVoxelWidth = 256;
    const float VoxelsPerMeter = 256.0f;

    const int VoxelCount = CubeVoxelWidth * CubeVoxelWidth * CubeVoxelWidth;

    const unsigned int TileSize = 8;

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

        std::thread StartKinectThread();
        void UpdateKinectVoxelData();

        Base::Float4x4 m_WorldMatrix;

		short* m_pVolumeBlock;

        GLuint m_VertexArray;
        GLuint m_VoxelDataBuffer;
        GLuint m_DrawCallConstantBuffer;

        GLuint m_VoxelDataPBOs[2];
        int m_WritablePBOIndex;
        std::atomic_bool m_NewVolumeExported;

        CShaderPtr m_VertexShader;
        CShaderPtr m_FragmentShader;
        CShaderPtr m_ComputeShader;
        
		MR::CKinectControl m_KinectControl;

        double m_TimeSinceLastVoxelUpdate;

        short* m_pVolumeData;

        std::thread m_KinectThread;
        std::atomic_bool m_KinectThreadStopped;
        std::mutex m_VolumeBlockMutex;
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
        m_TimeSinceLastVoxelUpdate = 0.0;

        Main::RegisterResizeHandler(GFX_BIND_RESIZE_METHOD(&CGfxVoxelRenderer::OnResize));

        float Translation = - CubeVoxelWidth / 2.0f;

        std::vector<Float3> Vertices;

        for (int i = 0; i < CubeVoxelWidth; ++i)
        {
            for (int j = 0; j < CubeVoxelWidth; ++j)
            {
                for (int k = 0; k < CubeVoxelWidth; ++k)
                {
                    Vertices.push_back(Float3(static_cast<float>(i) + Translation - 50.0f,
                                              static_cast<float>(j) + Translation,
                                              static_cast<float>(k) + Translation));
                }
            }
        }

        GLuint VertexBuffer;

        glGenBuffers(1, &VertexBuffer);
        glGenVertexArrays(1, &m_VertexArray);

        glBindVertexArray(m_VertexArray);

        glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(Vertices[0]), Vertices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Float3), 0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

		m_pVolumeBlock = new short[VoxelCount];

        glGenBuffers(2, m_VoxelDataPBOs);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_VoxelDataPBOs[0]);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, VoxelCount * sizeof(short), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_VoxelDataPBOs[1]);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, VoxelCount * sizeof(short), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        m_WritablePBOIndex = 0;

        m_pVolumeData = static_cast<short*>(glMapNamedBuffer(m_VoxelDataPBOs[m_WritablePBOIndex], GL_WRITE_ONLY));

        m_NewVolumeExported = false;
        m_KinectThreadStopped = true;
        m_KinectThread = StartKinectThread();
    }
    
    // -----------------------------------------------------------------------------
    

    std::thread CGfxVoxelRenderer::StartKinectThread()
    {
        auto ThreadFunctor = [this]
        {
            m_KinectControl.Start(CubeVoxelWidth, CubeVoxelWidth, CubeVoxelWidth, VoxelsPerMeter);

            m_KinectThreadStopped = false;

            while (!m_KinectThreadStopped)
            {
                this->UpdateKinectVoxelData();
            }

            m_KinectControl.Stop();
        };

        std::thread WorkerThread(ThreadFunctor);

        return WorkerThread;
    }

    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnExit()
    {
        m_KinectThreadStopped = true;
        m_KinectThread.join();

		delete[] m_pVolumeBlock;

        m_VertexShader = 0;
        m_FragmentShader = 0;
        m_ComputeShader = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::OnSetupShader()
    {
        std::stringstream TileSizeDefineStream;
        std::stringstream CubeWidthDefineStream;

        TileSizeDefineStream << "TILE_SIZE " << TileSize << '\n';
        CubeWidthDefineStream << "CUBE_WIDTH " << CubeVoxelWidth << '\n';

        std::string TileSizeDefine = TileSizeDefineStream.str();
        std::string CubeWidthDefine = CubeWidthDefineStream.str();

        const char* pDefines[] = { TileSizeDefine.c_str(), CubeWidthDefine.c_str() };

        m_VertexShader = ShaderManager::CompileVS("vs_voxel.glsl", "main", 1, pDefines + 1);
        m_FragmentShader = ShaderManager::CompilePS("fs_voxel.glsl", "main");
        m_ComputeShader = ShaderManager::CompileCS("cs_voxel.glsl", "main", 2, pDefines);
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
        glGenTextures(1, &m_VoxelDataBuffer);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, m_VoxelDataBuffer);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexImage3D(GL_TEXTURE_3D, 0, GL_R16, CubeVoxelWidth, CubeVoxelWidth, CubeVoxelWidth, 0, GL_RED, GL_SHORT, nullptr);

		/*const unsigned int VoxelCount = CubeVoxelWidth * CubeVoxelWidth * CubeVoxelWidth;

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
        glBindBuffer(GL_UNIFORM_BUFFER, m_DrawCallConstantBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(SDrawCallBufferData), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
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
    
    void CGfxVoxelRenderer::UpdateKinectVoxelData()
    {
        m_KinectControl.Update();

        m_TimeSinceLastVoxelUpdate += Core::Time::GetDeltaTimeLastFrame();

        if (!m_NewVolumeExported)
        {
            m_KinectControl.ExportVolumeBlock(m_pVolumeData);
            
            m_NewVolumeExported = true;
            m_TimeSinceLastVoxelUpdate = 0.0;
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxVoxelRenderer::Update()
    {
        if (m_KinectThreadStopped)
        {
            return;
        }

        const float Scale = CubeWidth / CubeVoxelWidth;

        Base::Float4x4 ScalingMatrix;
        Base::Float4x4 RotationMatrix;

        ScalingMatrix.SetScale(Scale, Scale, -Scale);
        RotationMatrix.SetRotation(0.0f, 0.0f, 3.14f);

        m_WorldMatrix = RotationMatrix * ScalingMatrix;
    }
    
    // -----------------------------------------------------------------------------
    
    void CGfxVoxelRenderer::Render()
    {
        Performance::BeginEvent("Voxel Rendering");

        //////////////////////////////////////////////////////////////////////////////////////
        // Compute shader
        //////////////////////////////////////////////////////////////////////////////////////

        //Gfx::ContextManager::SetShaderCS(m_ComputeShader);
        //glBindImageTexture(0, m_VoxelDataBuffer, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R16);
        //glDispatchCompute(CubeVoxelWidth / TileSize, CubeVoxelWidth / TileSize, CubeVoxelWidth / TileSize);

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

        glBindVertexArray(m_VertexArray);

        CBufferPtr FrameConstantBufferPtr = Gfx::Main::GetPerFrameConstantBufferVS();
        CNativeBuffer FrameConstantBuffer = *static_cast<CNativeBuffer*>(FrameConstantBufferPtr.GetPtr());

        SDrawCallBufferData* pBuffer = static_cast<SDrawCallBufferData*>(glMapNamedBuffer(m_DrawCallConstantBuffer, GL_WRITE_ONLY));
        pBuffer->m_WorldMatrix = m_WorldMatrix;
        glUnmapNamedBuffer(m_DrawCallConstantBuffer);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, FrameConstantBuffer.m_NativeBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_DrawCallConstantBuffer);

        if (m_NewVolumeExported)
        {
            glUnmapNamedBuffer(m_VoxelDataPBOs[m_WritablePBOIndex]);

            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_VoxelDataPBOs[m_WritablePBOIndex]);

            m_WritablePBOIndex = (m_WritablePBOIndex + 1) % 2;

            m_pVolumeData = static_cast<short*>(glMapNamedBuffer(m_VoxelDataPBOs[m_WritablePBOIndex], GL_WRITE_ONLY));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_3D, m_VoxelDataBuffer);
            glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, CubeVoxelWidth, CubeVoxelWidth, CubeVoxelWidth, GL_RED, GL_SHORT, 0);
            glBindTexture(GL_TEXTURE_3D, 0);

            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

            m_NewVolumeExported = false;
        }

        glBindImageTexture(0, m_VoxelDataBuffer, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R16);

        glDrawArrays(GL_POINTS, 0, VoxelCount);

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

