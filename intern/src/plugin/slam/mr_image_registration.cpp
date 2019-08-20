
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

    struct SConstantBuffer
    {
        glm::vec4 Dummy;
    };

    const int g_TileSize2D = 16;
    
} // namespace

namespace MR
{
    using namespace Gfx;

    void CImageRegistrator::Register()
    {
        Performance::BeginEvent("Image registration");

        ContextManager::SetImageTexture(0, m_Texture1);
        ContextManager::SetImageTexture(1, m_Texture2);
        ContextManager::SetImageTexture(2, m_DebugTexture);
        ContextManager::SetShaderCS(m_RegistrationCSPtr);

        ContextManager::Dispatch(DivUp(m_Texture1->GetNumberOfPixelsU(), g_TileSize2D), DivUp(m_Texture1->GetNumberOfPixelsV(), g_TileSize2D), 1);

        ContextManager::ResetImageTexture(0);
        ContextManager::ResetImageTexture(1);
        ContextManager::ResetImageTexture(2);
        ContextManager::ResetShaderCS();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CImageRegistrator::SetupShaders()
    {
        std::stringstream DefineStream;

        DefineStream << "#define TILE_SIZE2D " << g_TileSize2D << " \n";

        std::string DefineString = DefineStream.str();

        m_RegistrationCSPtr = ShaderManager::CompileCS("../../plugins/slam/scalable/registration/cs_register_image.glsl", "main", DefineString.c_str());
    }

    // -----------------------------------------------------------------------------

    void CImageRegistrator::SetupBuffers()
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

        const auto Offset = glm::ivec2(10);
        const auto ImageSize = glm::ivec2(m_Texture1->GetNumberOfPixelsU(), m_Texture1->GetNumberOfPixelsV());

        TextureManager::CopyTexture(m_Texture1, m_Texture2, glm::ivec2(0), Offset, glm::ivec2(ImageSize - Offset));

        TextureDescriptor.m_NumberOfPixelsU = m_Texture1->GetNumberOfPixelsU();
        TextureDescriptor.m_NumberOfPixelsV = m_Texture1->GetNumberOfPixelsV();
        TextureDescriptor.m_NumberOfPixelsW = m_Texture1->GetNumberOfPixelsW();
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Format = CTexture::R32G32B32A32_FLOAT;

        m_DebugTexture = TextureManager::CreateTexture2D(TextureDescriptor);
    }

    // -----------------------------------------------------------------------------

    CImageRegistrator::CImageRegistrator()
    {
        SetupBuffers();
        SetupShaders();
        SetupStates();
        SetupTextures();
    }

    // -----------------------------------------------------------------------------

    CImageRegistrator::~CImageRegistrator()
    {
        m_RegistrationCSPtr = nullptr;
        m_ConstantBufferPtr = nullptr;

        m_Texture1 = nullptr;
        m_Texture2 = nullptr;

        m_DebugTexture = nullptr;
    }

} // namespace MR
