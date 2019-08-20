
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
    
} // namespace

namespace MR
{
    using namespace Gfx;

    void CImageRegistrator::Register()
    {
        Performance::BeginEvent("Image registration");

        ContextManager::SetImageTexture(0, m_Texture1);
        ContextManager::SetImageTexture(1, m_Texture2);
        ContextManager::SetShaderCS(m_RegistrationCSPtr);

        ContextManager::Dispatch(1024 / 16, 1024 / 16, 1);

        ContextManager::ResetImageTexture(0);
        ContextManager::ResetImageTexture(1);
        ContextManager::ResetShaderCS();

        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CImageRegistrator::SetupShaders()
    {
        std::string DefineString = "";

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
        const int TextureWidth = 1024;

        std::vector<glm::u8vec4> Pixels(TextureWidth * TextureWidth);

        for (auto& Pixel : Pixels)
        {
            Pixel = glm::u8vec4(127);
        }

        STextureDescriptor TextureDescriptor = {};

        TextureDescriptor.m_NumberOfPixelsU = TextureWidth;
        TextureDescriptor.m_NumberOfPixelsV = TextureWidth;
        TextureDescriptor.m_NumberOfPixelsW = 1;
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding = CTexture::ShaderResource;
        TextureDescriptor.m_Access = CTexture::EAccess::CPURead;
        TextureDescriptor.m_Usage = CTexture::EUsage::GPUReadWrite;
        TextureDescriptor.m_Semantic = CTexture::UndefinedSemantic;
        TextureDescriptor.m_Format = CTexture::R8G8B8A8_UBYTE;
        TextureDescriptor.m_pPixels = Pixels.data();

        m_Texture1 = TextureManager::CreateTexture2D(TextureDescriptor);
        m_Texture2 = TextureManager::CreateTexture2D(TextureDescriptor);
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
    }

} // namespace MR
