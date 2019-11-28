
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
        glm::vec3 m_Normal;
        float Padding;
    };

} // namespace

namespace MR
{
    // -----------------------------------------------------------------------------

    void CPlaneColorizer::UpdatePlane(const std::string& _ID)
    {
        auto& rPlaneMap = m_pReconstructor->GetPlanes();

        auto Iter = rPlaneMap.find(_ID);

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
        const bool InpaintExtent = Core::CProgramParameters::GetInstance().Get("mr:diminished_reality:plane_mode:fill_extent", true);

        Performance::BeginEvent("Plane colorization");

        MR::CSLAMReconstructor::SSLAMVolume& rVolume = m_pReconstructor->GetVolume();

        ContextManager::SetResourceBuffer(0, rVolume.m_RootVolumePoolPtr);
        ContextManager::SetResourceBuffer(1, rVolume.m_RootGridPoolPtr);
        ContextManager::SetResourceBuffer(2, rVolume.m_Level1PoolPtr);
        ContextManager::SetResourceBuffer(3, rVolume.m_TSDFPoolPtr);
        ContextManager::SetResourceBuffer(6, rVolume.m_RootVolumePositionBufferPtr);
        ContextManager::SetConstantBuffer(2, rVolume.m_AABBBufferPtr);

        ContextManager::SetTargetSet(m_TargetSetPtr);
        ContextManager::SetViewPortSet(m_ViewPortSetPtr);

        ContextManager::SetRasterizerState(StateManager::GetRasterizerState(CRasterizerState::NoCull));
        ContextManager::SetDepthStencilState(StateManager::GetDepthStencilState(CDepthStencilState::Default));
        ContextManager::SetBlendState(StateManager::GetBlendState(CBlendState::Default));

        ContextManager::SetConstantBuffer(0, m_ConstantBufferPtr);

        ContextManager::SetInputLayout(m_PlaneMeshLayoutPtr);

        auto& rPlaneMap = m_pReconstructor->GetPlanes();

        for (auto& Iter : rPlaneMap)
        {
            auto& rPlane = Iter.second;

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
                TextureDescriptor.m_pPixels = nullptr;

                rPlane.m_TexturePtr = TextureManager::CreateTexture2D(TextureDescriptor);
            }

            ColorizePlane(rPlane, rPlane.m_MeshPtr == nullptr || InpaintExtent);
        }

        ContextManager::ResetResourceBuffer(0);
        ContextManager::ResetResourceBuffer(1);
        ContextManager::ResetResourceBuffer(2);
        ContextManager::ResetResourceBuffer(3);
        ContextManager::ResetResourceBuffer(6);
        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetConstantBuffer(2);

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CPlaneColorizer::ColorizePlane(CSLAMReconstructor::SPlane& _rPlane, bool _WholeExtent)
    {
        ContextManager::SetImageTexture(0, _rPlane.m_TexturePtr);

        SConstantBuffer BufferData;

        BufferData.m_WorldMatrix = _rPlane.m_Transform;
        BufferData.m_Color = _WholeExtent ? glm::vec4(_rPlane.m_Min, _rPlane.m_Extent) : glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        BufferData.m_Normal = _rPlane.m_Normal;

        BufferManager::UploadBufferData(m_ConstantBufferPtr, &BufferData);

        TextureManager::ClearTexture(_rPlane.m_TexturePtr);

        if (_WholeExtent)
        {
            ContextManager::SetShaderVS(m_ExtentColorizationVSPtr);
            ContextManager::SetShaderPS(m_ExtentColorizationFSPtr);

            const unsigned int Offset = 0;
            ContextManager::SetVertexBuffer(m_ExtentMeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
            ContextManager::SetIndexBuffer(m_ExtentMeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);

            ContextManager::SetTopology(STopology::TriangleStrip);

            ContextManager::Draw(m_ExtentMeshPtr->GetLOD(0)->GetSurface()->GetNumberOfVertices(), 0);
        }
        else
        {
            ContextManager::SetShaderVS(m_PlaneColorizationVSPtr);
            ContextManager::SetShaderPS(m_PlaneColorizationFSPtr);

            const unsigned int Offset = 0;
            ContextManager::SetVertexBuffer(_rPlane.m_MeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());
            ContextManager::SetIndexBuffer(_rPlane.m_MeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), Offset);

            ContextManager::SetTopology(STopology::TriangleList);

            ContextManager::DrawIndexed(_rPlane.m_MeshPtr->GetLOD(0)->GetSurface()->GetNumberOfIndices(), 0, 0);
        }

        if (const bool InpaintExtent = Core::CProgramParameters::GetInstance().Get("mr:diminished_reality:plane_mode:inpaint", true))
        {
            const auto Width = _rPlane.m_TexturePtr->GetNumberOfPixelsU();
            const auto Height = _rPlane.m_TexturePtr->GetNumberOfPixelsV();
            const auto PixelCount = Width * Height;

            std::vector<glm::u8vec4> RawData(PixelCount);

            Gfx::TextureManager::CopyTextureToCPU(_rPlane.m_TexturePtr, reinterpret_cast<char*>(RawData.data()));

            std::vector<glm::u8vec4> InpaintedImage(PixelCount);

            InpaintWithPixMix(glm::ivec2(Width, Height), RawData, InpaintedImage);

            auto TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(Width, Height));
            Gfx::TextureManager::CopyToTexture2D(_rPlane.m_TexturePtr, TargetRect, Width, reinterpret_cast<char*>(InpaintedImage.data()), true);
        }
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
            << "#define MAX_INTEGRATION_WEIGHT " << Settings.m_MaxIntegrationWeight << '\n'
            << "#define ROOT_RESOLUTION " << Settings.m_GridResolutions[0] << '\n'
            << "#define LEVEL1_RESOLUTION " << Settings.m_GridResolutions[1] << '\n'
            << "#define LEVEL2_RESOLUTION " << Settings.m_GridResolutions[2] << '\n'
            << "#define VOXELS_PER_ROOTGRID " << Settings.m_VoxelsPerGrid[0] << " \n"
            << "#define VOXELS_PER_LEVEL1GRID " << Settings.m_VoxelsPerGrid[1] << " \n"
            << "#define VOXELS_PER_LEVEL2GRID " << Settings.m_VoxelsPerGrid[2] << " \n"
            << "#define RAYCAST_NEAR " << 0.0f << " \n"
            << "#define RAYCAST_FAR " << m_MaxRaycastLength << " \n"
            << "#define CAPTURE_COLOR \n"
            << "#define MIN_TREE_WEIGHT " << Core::CProgramParameters::GetInstance().Get("mr:slam:rendering:min_weight", 30) << " \n"
            << "#define CAMERA_OFFSET " << m_CameraOffset << '\n';

        std::string DefineString = DefineStream.str();

        m_PlaneColorizationVSPtr = ShaderManager::CompileVS("../../plugins/slam/scalable/colorization/vs_colorize_mesh.glsl", "main", DefineString.c_str());
        m_PlaneColorizationFSPtr = ShaderManager::CompilePS("../../plugins/slam/scalable/colorization/fs_colorize_mesh.glsl", "main", DefineString.c_str());

        m_ExtentColorizationVSPtr = ShaderManager::CompileVS("../../plugins/slam/scalable/colorization/vs_colorize_extent.glsl", "main", DefineString.c_str());
        m_ExtentColorizationFSPtr = ShaderManager::CompilePS("../../plugins/slam/scalable/colorization/fs_colorize_extent.glsl", "main", DefineString.c_str());

        SInputElementDescriptor PlaneMeshLayout[] =
        {
            { "POSITION", 0, CInputLayout::Float3Format, 0,  0, 20, CInputLayout::PerVertex, 0 },
            { "TEXCOORD", 1, CInputLayout::Float2Format, 0, 12, 20, CInputLayout::PerVertex, 0 },
        };

        m_PlaneMeshLayoutPtr = ShaderManager::CreateInputLayout(PlaneMeshLayout, sizeof(PlaneMeshLayout) / sizeof(PlaneMeshLayout[0]), m_PlaneColorizationVSPtr);
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

    void CPlaneColorizer::SetupMeshes()
    {
        struct SQuadVertex
        {
            glm::vec3 m_Pos;
            glm::vec2 m_Tex;
        };

        SQuadVertex Quad[4] =
        {
            { glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f) },
            { glm::vec3( 1.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f) },
            { glm::vec3(-1.0f, 0.0f,  1.0f), glm::vec2(1.0f, 0.0f) },
            { glm::vec3( 1.0f, 0.0f,  1.0f), glm::vec2(0.0f, 0.0f) },
        };

        m_ExtentMeshPtr = Gfx::MeshManager::CreateMesh(Quad, sizeof(Quad) / sizeof(Quad[0]), sizeof(Quad[0]), nullptr, 0);
    }

    // -----------------------------------------------------------------------------

    CPlaneColorizer::CPlaneColorizer(MR::CSLAMReconstructor* _pReconstructor)
        : m_pReconstructor(_pReconstructor)
    {
        assert(_pReconstructor != nullptr);

        m_PlaneTextureSize = Core::CProgramParameters::GetInstance().Get("mr:plane_extraction:texture_size", 512);
        m_CameraOffset = Core::CProgramParameters::GetInstance().Get("mr:plane_extraction:camera_offset", 0.2f);
        m_MaxRaycastLength = Core::CProgramParameters::GetInstance().Get("mr:plane_extraction:max_raycast_length", 0.4f);

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
        SetupMeshes();

        if (!Core::PluginManager::LoadPlugin("PixMix"))
        {
            BASE_THROWM("PixMix plugin was not loaded");
        }

        InpaintWithPixMix = (InpaintWithPixMixFunc)(Core::PluginManager::GetPluginFunction("PixMix", "Inpaint"));
    }

    // -----------------------------------------------------------------------------

    CPlaneColorizer::~CPlaneColorizer()
    {
        m_DummyTexturePtr = nullptr;
        m_PlaneColorizationVSPtr = nullptr;
        m_PlaneColorizationFSPtr = nullptr;
        m_ConstantBufferPtr = nullptr;
        m_PlaneMeshLayoutPtr = nullptr;
        m_ViewPortSetPtr = nullptr;
    }

} // namespace MR
