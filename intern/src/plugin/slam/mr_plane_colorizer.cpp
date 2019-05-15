
#include "plugin/slam/slam_precompiled.h"

#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"
#include "engine/core/core_time.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

#include "plugin/slam/mr_plane_colorizer.h"

#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

#include <gl/glew.h>

using namespace MR;
using namespace Gfx;

namespace
{
    int DivUp(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }

    struct SConstantBuffer
    {
        glm::mat4 m_WorldMatrix;
        glm::vec4 m_Color;
    };

} // namespace

namespace MR
{
    // -----------------------------------------------------------------------------

    void CPlaneColorizer::UpdatePlane(int _PlaneID)
    {
        auto& rPlaneMap = m_pReconstructor->GetPlanes();

        auto Iter = rPlaneMap.find(_PlaneID);

        if (Iter != rPlaneMap.end())
        {
            auto& rPlane = Iter->second;

            UpdatePlane(rPlane);
        }
    }

    // -----------------------------------------------------------------------------

    void CPlaneColorizer::UpdatePlane(CSLAMReconstructor::SPlane& _rPlane)
    {
        if (_rPlane.m_TexturePtr == nullptr)
        {
            _rPlane.m_TexturePtr = m_DummyTexturePtr;
        }
    }
    
    // -----------------------------------------------------------------------------

    void CPlaneColorizer::ColorizeAllPlanes()
    {
        auto& rPlaneMap = m_pReconstructor->GetPlanes();

        std::vector<unsigned char> Pixels;

        for (int i = 0; i < m_PlaneTextureSize; ++ i)
        {
            for (int j = 0; j < m_PlaneTextureSize; ++ j)
            {
                Pixels.push_back(255);
                Pixels.push_back(20);
                Pixels.push_back(147);
                Pixels.push_back(255);
            }
        }

        for (auto& Iter : rPlaneMap)
        {
            auto& rPlane = Iter.second;

            if (rPlane.m_MeshPtr != nullptr)
            {
                if (rPlane.m_TexturePtr == m_DummyTexturePtr)
                {
                    STextureDescriptor TextureDescriptor = {};

                    TextureDescriptor.m_NumberOfPixelsU = m_PlaneTextureSize;
                    TextureDescriptor.m_NumberOfPixelsV = m_PlaneTextureSize;
                    TextureDescriptor.m_NumberOfPixelsW = 1;
                    TextureDescriptor.m_NumberOfMipMaps = 1;
                    TextureDescriptor.m_NumberOfTextures = 1;
                    TextureDescriptor.m_Binding = CTexture::ShaderResource;
                    TextureDescriptor.m_Access = CTexture::EAccess::CPURead;
                    TextureDescriptor.m_Usage = CTexture::EUsage::GPUReadWrite;
                    TextureDescriptor.m_Semantic = CTexture::UndefinedSemantic;
                    TextureDescriptor.m_Format = CTexture::R8G8B8A8_UBYTE;
                    TextureDescriptor.m_pPixels = Pixels.data();

                    rPlane.m_TexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);
                }

                ColorizePlane(rPlane);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CPlaneColorizer::ColorizePlane(CSLAMReconstructor::SPlane& _rPlane)
    {
        Performance::BeginEvent("Plane colorization");
        
        ContextManager::SetConstantBuffer(0, m_ConstantBufferPtr);

        SConstantBuffer BufferData;

        ContextManager::SetTargetSet(m_TargetSetPtr);
        ContextManager::SetViewPortSet(m_ViewPortSetPtr);

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::Default));
        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::Default));
        ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::Default));

        ContextManager::SetShaderVS(m_ColorizationVSPtr);
        ContextManager::SetShaderPS(m_ColorizationFSPtr);
        
        const unsigned int Offset = 0;
        ContextManager::SetVertexBuffer(_rPlane.m_MeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
        ContextManager::SetIndexBuffer(_rPlane.m_MeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);

        ContextManager::SetImageTexture(0, _rPlane.m_TexturePtr);

        ContextManager::SetInputLayout(m_PlaneMeshLayoutPtr);
        ContextManager::SetTopology(STopology::TriangleList);

        BufferData.m_WorldMatrix = _rPlane.m_Transform;
        BufferData.m_Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

        BufferManager::UploadBufferData(m_ConstantBufferPtr, &BufferData);

        ContextManager::DrawIndexed(_rPlane.m_MeshPtr->GetLOD(0)->GetSurface()->GetNumberOfIndices(), 0, 0);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CPlaneColorizer::SetupShaders()
    {
        MR::SReconstructionSettings Settings;
        m_pReconstructor->GetReconstructionSettings(&Settings);
        std::stringstream DefineStream;

        DefineStream
            << "#define TRUNCATED_DISTANCE " << Settings.m_TruncatedDistance / 1000.0f << " \n"
            << "#define VOLUME_SIZE " << Settings.m_VolumeSize << " \n"
            << "#define VOXEL_SIZE " << Settings.m_VoxelSize << " \n"
            << "#define ROOT_RESOLUTION " << Settings.m_GridResolutions[0] << '\n'
            << "#define LEVEL1_RESOLUTION " << Settings.m_GridResolutions[1] << '\n'
            << "#define LEVEL2_RESOLUTION " << Settings.m_GridResolutions[2] << '\n'
            << "#define VOXELS_PER_ROOTGRID " << Settings.m_VoxelsPerGrid[0] << " \n"
            << "#define VOXELS_PER_LEVEL1GRID " << Settings.m_VoxelsPerGrid[1] << " \n"
            << "#define VOXELS_PER_LEVEL2GRID " << Settings.m_VoxelsPerGrid[2] << " \n"
            << "#define RAYCAST_NEAR " << 0.0f << " \n"
            << "#define RAYCAST_FAR " << 1000.0f << " \n"
            << "#define MIN_TREE_WEIGHT " << Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:min_weight", 30) << " \n"
            << "#define CAPTURE_COLOR\n";

        std::string DefineString = DefineStream.str();

        m_ColorizationVSPtr = ShaderManager::CompileVS("../../plugins/slam/scalable/colorization/vs_colorize_plane.glsl", "main", DefineString.c_str());
        m_ColorizationFSPtr = ShaderManager::CompilePS("../../plugins/slam/scalable/colorization/fs_colorize_plane.glsl", "main", DefineString.c_str());

        SInputElementDescriptor PlaneMeshLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float3Format, 0,  0, 20, CInputLayout::PerVertex, 0 },
            { "TEXCOORD", 1, CInputLayout::Float2Format, 0, 12, 20, CInputLayout::PerVertex, 0 },
        };

        m_PlaneMeshLayoutPtr = ShaderManager::CreateInputLayout(PlaneMeshLayout, sizeof(PlaneMeshLayout) / sizeof(PlaneMeshLayout[0]), m_ColorizationVSPtr);
    }

    // -----------------------------------------------------------------------------

    void CPlaneColorizer::SetupBuffers()
    {
        SBufferDescriptor BufferDesc = {};

        BufferDesc.m_Stride = 0;
        BufferDesc.m_Usage = CBuffer::GPURead;
        BufferDesc.m_Binding = CBuffer::ConstantBuffer;
        BufferDesc.m_Access = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(SConstantBuffer);
        BufferDesc.m_pBytes = nullptr;
        BufferDesc.m_pClassKey = nullptr;

        m_ConstantBufferPtr = BufferManager::CreateBuffer(BufferDesc);
    }

    // -----------------------------------------------------------------------------

    void CPlaneColorizer::SetupStates()
    {
        Gfx::SViewPortDescriptor Desc;

        Desc.m_Width = static_cast<float>(m_PlaneTextureSize);
        Desc.m_Height = static_cast<float>(m_PlaneTextureSize);
        Desc.m_TopLeftX = 0;
        Desc.m_TopLeftY = 0;
        Desc.m_MinDepth = 0.0f;
        Desc.m_MaxDepth = 0.0f;

        CViewPortPtr ViewPort = ViewManager::CreateViewPort(Desc);

        m_ViewPortSetPtr = ViewManager::CreateViewPortSet(ViewPort);

        m_TargetSetPtr = TargetSetManager::CreateEmptyTargetSet(m_PlaneTextureSize, m_PlaneTextureSize);
    }

    // -----------------------------------------------------------------------------

    CPlaneColorizer::CPlaneColorizer(MR::CSLAMReconstructor* _pReconstructor)
        : m_pReconstructor(_pReconstructor)
    {
        assert(_pReconstructor != nullptr);

        m_PlaneTextureSize = Core::CProgramParameters::GetInstance().Get("mr:plane_texture_size", 512);

        STextureDescriptor TextureDescriptor = {};

        TextureDescriptor.m_NumberOfPixelsU = STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsV = STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsW = 1;
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = STextureDescriptor::s_NumberOfTexturesFromSource;
        TextureDescriptor.m_Binding = CTexture::ShaderResource;
        TextureDescriptor.m_Access = CTexture::EAccess::CPURead;
        TextureDescriptor.m_Usage = CTexture::EUsage::GPUReadWrite;
        TextureDescriptor.m_Semantic = CTexture::UndefinedSemantic;
        TextureDescriptor.m_Format = CTexture::R8G8B8A8_UBYTE;
        TextureDescriptor.m_pFileName = "textures/lines_d.png";

        m_DummyTexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);

        SetupShaders();
        SetupBuffers();
        SetupStates();
    }

    // -----------------------------------------------------------------------------

    CPlaneColorizer::~CPlaneColorizer()
    {
        m_DummyTexturePtr = nullptr;
        m_ColorizationVSPtr = nullptr;
        m_ColorizationFSPtr = nullptr;
        m_ConstantBufferPtr = nullptr;
        m_PlaneMeshLayoutPtr = nullptr;
        m_ViewPortSetPtr = nullptr;
    }

} // namespace MR
