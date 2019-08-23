
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

#include "plugin/slam/mr_image_registration.h"

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

    struct SRegistrationBuffer
    {
        glm::vec2 m_Offset;
        glm::vec2 Dummy;
    };

    const int g_TileSize2D = 16;
    
} // namespace

namespace MR
{
    using namespace Gfx;

    void CImageRegistrator::Register()
    {
        Performance::BeginEvent("Image registration");

        // Setup data

        const int WorkGroupsX = DivUp(m_Texture1->GetNumberOfPixelsU(), g_TileSize2D);
        const int WorkGroupsY = DivUp(m_Texture1->GetNumberOfPixelsV(), g_TileSize2D);
        
        ContextManager::SetConstantBuffer(0, m_ConstantBuffer);
        ContextManager::SetResourceBuffer(0, m_SumBufferPtr);

        ContextManager::SetImageTexture(0, m_Texture1);
        ContextManager::SetImageTexture(1, m_Texture2);
        ContextManager::SetImageTexture(2, m_SDTexture);
        ContextManager::SetImageTexture(3, m_GradientTexture);

        SRegistrationBuffer RegistrationBuffer;
        RegistrationBuffer.m_Offset = glm::vec2(0.0f);
        BufferManager::UploadBufferData(m_ConstantBuffer, &RegistrationBuffer);

        // Compute squared differences

        ContextManager::SetShaderCS(m_SDCSPtr);
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        // Compute gradient image

        ContextManager::SetShaderCS(m_GradientCSPtr);
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        // Sum tiles

        ContextManager::SetShaderCS(m_SumTilesCSPtr);
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        // Compute final sum

        ContextManager::SetShaderCS(m_SumFinalCSPtr);
        ContextManager::Dispatch(1, 1, 1);

        // Reset

        ContextManager::ResetResourceBuffer(0);

        ContextManager::ResetImageTexture(0);
        ContextManager::ResetImageTexture(1);
        ContextManager::ResetImageTexture(2);
        ContextManager::ResetImageTexture(3);
        ContextManager::ResetShaderCS();
        
        glm::vec4 Gradient = *static_cast<glm::vec4*>(BufferManager::MapBufferRange(m_SumBufferPtr, CBuffer::Read, 0, sizeof(glm::vec4)));
        BufferManager::UnmapBuffer(m_SumBufferPtr);

        Gradient /= m_Texture1->GetNumberOfPixelsU() * m_Texture1->GetNumberOfPixelsV();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CImageRegistrator::SetupShaders()
    {
        assert(m_Texture1->GetNumberOfPixelsU() > 0);
        assert(m_Texture1->GetNumberOfPixelsU() > 1);

        const int WorkGroupsX = DivUp(m_Texture1->GetNumberOfPixelsU(), g_TileSize2D);
        const int WorkGroupsY = DivUp(m_Texture1->GetNumberOfPixelsV(), g_TileSize2D);

        std::stringstream DefineStream;

        DefineStream
            << "#define TILE_SIZE2D " << g_TileSize2D << " \n"
            << "#define TILE_COUNT_X " << WorkGroupsX << " \n"
            << "#define TILE_COUNT_Y " << WorkGroupsY << " \n"
            << "#define REDUCTION_SHADER_COUNT " << WorkGroupsX * WorkGroupsY << " \n";

        std::string DefineString = DefineStream.str();

        m_SDCSPtr = ShaderManager::CompileCS("../../plugins/slam/scalable/registration/cs_sd.glsl", "main", DefineString.c_str());
        m_GradientCSPtr = ShaderManager::CompileCS("../../plugins/slam/scalable/registration/cs_gradient.glsl", "main", DefineString.c_str());
        m_SumTilesCSPtr = ShaderManager::CompileCS("../../plugins/slam/scalable/registration/cs_sum_tiles.glsl", "main", DefineString.c_str());
        m_SumFinalCSPtr = ShaderManager::CompileCS("../../plugins/slam/scalable/registration/cs_sum_final.glsl", "main", DefineString.c_str());
    }

    // -----------------------------------------------------------------------------

    void CImageRegistrator::SetupBuffers()
    {
        assert(m_Texture1->GetNumberOfPixelsU() > 0);
        assert(m_Texture1->GetNumberOfPixelsU() > 1);

        SBufferDescriptor BufferDesc = {};

        BufferDesc.m_Stride = 0;
        BufferDesc.m_Usage = CBuffer::GPURead;
        BufferDesc.m_Binding = CBuffer::ResourceBuffer;
        BufferDesc.m_Access = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = m_Texture1->GetNumberOfPixelsU() * m_Texture1->GetNumberOfPixelsV() * sizeof(glm::vec4);
        BufferDesc.m_pBytes = nullptr;
        BufferDesc.m_pClassKey = nullptr;

        m_SumBufferPtr = BufferManager::CreateBuffer(BufferDesc);

        BufferDesc.m_Binding = CBuffer::ConstantBuffer;
        BufferDesc.m_NumberOfBytes = sizeof(SRegistrationBuffer);

        m_ConstantBuffer = BufferManager::CreateBuffer(BufferDesc);
    }

    // -----------------------------------------------------------------------------

    void CImageRegistrator::SetupStates()
    {

    }

    // -----------------------------------------------------------------------------

    void CImageRegistrator::SetupTextures()
    {
        STextureDescriptor TextureDescriptor = {};

        TextureDescriptor.m_NumberOfPixelsU = STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsV = STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsW = STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfMipMaps = STextureDescriptor::s_NumberOfMipMapsFromSource;
        TextureDescriptor.m_NumberOfTextures = STextureDescriptor::s_NumberOfTexturesFromSource;
        TextureDescriptor.m_Binding = CTexture::ShaderResource;
        TextureDescriptor.m_Access = CTexture::EAccess::CPURead;
        TextureDescriptor.m_Usage = CTexture::EUsage::GPUReadWrite;
        TextureDescriptor.m_Semantic = CTexture::UndefinedSemantic;
        TextureDescriptor.m_Format = CTexture::R8G8B8A8_UBYTE;
        TextureDescriptor.m_pFileName = "textures/Lenna.png";

        m_Texture1 = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureDescriptor.m_NumberOfPixelsU = m_Texture1->GetNumberOfPixelsU();
        TextureDescriptor.m_NumberOfPixelsV = m_Texture1->GetNumberOfPixelsV();
        TextureDescriptor.m_NumberOfPixelsW = m_Texture1->GetNumberOfPixelsW();
        TextureDescriptor.m_NumberOfMipMaps = m_Texture1->GetNumberOfMipLevels();
        TextureDescriptor.m_NumberOfTextures = m_Texture1->GetNumberOfTextures();
        TextureDescriptor.m_pFileName = nullptr;

        m_Texture2 = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureManager::CopyTexture(m_Texture1, m_Texture2);

        TextureDescriptor.m_NumberOfPixelsU = m_Texture1->GetNumberOfPixelsU();
        TextureDescriptor.m_NumberOfPixelsV = m_Texture1->GetNumberOfPixelsV();
        TextureDescriptor.m_NumberOfPixelsW = m_Texture1->GetNumberOfPixelsW();
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Format = CTexture::R32G32B32A32_FLOAT;

        m_SDTexture = TextureManager::CreateTexture2D(TextureDescriptor);
        m_GradientTexture = TextureManager::CreateTexture2D(TextureDescriptor);
    }

    // -----------------------------------------------------------------------------

    CImageRegistrator::CImageRegistrator()
    {
        SetupStates();
        SetupTextures();
        SetupBuffers();
        SetupShaders();
    }

    // -----------------------------------------------------------------------------

    CImageRegistrator::~CImageRegistrator()
    {
        m_SDCSPtr = nullptr;
        m_GradientCSPtr = nullptr;
        m_SumTilesCSPtr = nullptr;
        m_SumFinalCSPtr = nullptr;

        m_ConstantBuffer = nullptr;
        m_SumBufferPtr = nullptr;

        m_Texture1 = nullptr;
        m_Texture2 = nullptr;

        m_SDTexture = nullptr;
    }

} // namespace MR
