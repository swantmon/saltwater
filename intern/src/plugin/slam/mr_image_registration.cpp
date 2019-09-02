
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
        glm::vec4 m_Theta;    // (Angle, Scale, Translation)
        glm::ivec2 m_FixedImageSize;
        glm::ivec2 m_MovingImageSize;
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

		float Angle = glm::radians(0.0f);
		float Scale = 1.0f;
        auto Translation = glm::vec2(0.0f);

        const int WorkGroupsX = DivUp(m_FixedTexture->GetNumberOfPixelsU(), g_TileSize2D);
        const int WorkGroupsY = DivUp(m_FixedTexture->GetNumberOfPixelsV(), g_TileSize2D);
        
        ContextManager::SetConstantBuffer(0, m_ConstantBuffer);
        ContextManager::SetResourceBuffer(0, m_SumBufferPtr);

        ContextManager::SetImageTexture(0, m_FixedTexture);
        ContextManager::SetImageTexture(1, m_MovingTexture);
		ContextManager::SetImageTexture(2, m_GradientTexture);
		ContextManager::SetImageTexture(3, m_DebugTexture);

        ContextManager::SetTexture(0, m_FixedTexture);
        ContextManager::SetTexture(1, m_MovingTexture);
        ContextManager::SetTexture(2, m_GradientTexture);

        ContextManager::SetSampler(0, SamplerManager::GetSampler(Gfx::CSampler::ESampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(1, SamplerManager::GetSampler(Gfx::CSampler::ESampler::MinMagMipLinearClamp));
        ContextManager::SetSampler(2, SamplerManager::GetSampler(Gfx::CSampler::ESampler::MinMagMipLinearClamp));

        // Precompute derivates in moving image (Sobel operator)

        ContextManager::SetShaderCS(m_GradientCSPtr);
        ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        float a = Scale * glm::cos(Angle);
        float b = Scale * glm::sin(Angle);

        glm::vec4 Gradient;
        const int MaxIterations = 50000;
        const float MinGradientLength = 0.0001f;

        float gamma = 1.0f;

        int Iteration = 0;
        for (Iteration = 0; Iteration < MaxIterations; ++ Iteration)
        {
			//std::cout << Translation.x << '\t' << Translation.y << '\n';

            SRegistrationBuffer RegistrationBuffer;
            RegistrationBuffer.m_Theta = glm::vec4(a, b, Translation);
            RegistrationBuffer.m_FixedImageSize.x = m_FixedTexture->GetNumberOfPixelsU();
            RegistrationBuffer.m_FixedImageSize.y = m_FixedTexture->GetNumberOfPixelsV();
            RegistrationBuffer.m_MovingImageSize.x = m_MovingTexture->GetNumberOfPixelsU();
            RegistrationBuffer.m_MovingImageSize.y = m_MovingTexture->GetNumberOfPixelsV();
            BufferManager::UploadBufferData(m_ConstantBuffer, &RegistrationBuffer);

            // Sum tiles

            ContextManager::SetShaderCS(m_SumTilesCSPtr);
            ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

            // Compute final sum

            ContextManager::SetShaderCS(m_SumFinalCSPtr);
            ContextManager::Dispatch(1, 1, 1);

            // Compute new registration parameter

            Gradient = *static_cast<glm::vec4*>(BufferManager::MapBufferRange(m_SumBufferPtr, CBuffer::Read, 0, sizeof(glm::vec4)));
            BufferManager::UnmapBuffer(m_SumBufferPtr);

            Gradient /= static_cast<float>(m_FixedTexture->GetNumberOfPixelsU() * m_FixedTexture->GetNumberOfPixelsV());

            if (glm::length(Gradient) > MinGradientLength)
            {
                Gradient *= gamma;

                a += Gradient.x;
                b += Gradient.y;
                Translation += glm::vec2(Gradient.z, Gradient.w);
            }
            else
            {
                break;
            }
        }

        std::cout << "Iterations:      " << Iteration << '\n';
        std::cout << "Gradient:        " << Gradient.x << "\t" << Gradient.y << "\t" << Gradient.z << "\t" << Gradient.w << '\n';
        std::cout << "Gradient length: " << glm::length(Gradient) << '\n';
        std::cout << "Theta:           " << a << "\t" << b << "\t" << Translation.x << "\t" << Translation.y << '\n';

        // Reset

        ContextManager::ResetConstantBuffer(0);
        ContextManager::ResetResourceBuffer(0);

        for (int i = 0; i < 4; ++ i)
        {
            ContextManager::ResetImageTexture(i);
            ContextManager::ResetTexture(i);
            ContextManager::ResetSampler(i);
        }
        
        ContextManager::ResetShaderCS();
        
        Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CImageRegistrator::SetupShaders()
    {
        const int WorkGroupsX = DivUp(m_FixedTexture->GetNumberOfPixelsU(), g_TileSize2D);
        const int WorkGroupsY = DivUp(m_FixedTexture->GetNumberOfPixelsV(), g_TileSize2D);

        std::stringstream DefineStream;

        DefineStream
            << "#define TILE_SIZE2D " << g_TileSize2D << " \n"
            << "#define TILE_COUNT_X " << WorkGroupsX << " \n"
            << "#define TILE_COUNT_Y " << WorkGroupsY << " \n"
            << "#define REDUCTION_SHADER_COUNT " << WorkGroupsX * WorkGroupsY << " \n";

        std::string DefineString = DefineStream.str();

        m_GradientCSPtr = ShaderManager::CompileCS("../../plugins/slam/scalable/registration/cs_gradient.glsl", "main", DefineString.c_str());
        m_SumTilesCSPtr = ShaderManager::CompileCS("../../plugins/slam/scalable/registration/cs_sum_tiles.glsl", "main", DefineString.c_str());
        m_SumFinalCSPtr = ShaderManager::CompileCS("../../plugins/slam/scalable/registration/cs_sum_final.glsl", "main", DefineString.c_str());
    }

    // -----------------------------------------------------------------------------

    void CImageRegistrator::SetupBuffers()
    {
        SBufferDescriptor BufferDesc = {};

        BufferDesc.m_Stride = 0;
        BufferDesc.m_Usage = CBuffer::GPURead;
        BufferDesc.m_Binding = CBuffer::ResourceBuffer;
        BufferDesc.m_Access = CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = m_FixedTexture->GetNumberOfPixelsU() * m_FixedTexture->GetNumberOfPixelsV() * sizeof(glm::vec4);
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

        m_FixedTexture = TextureManager::CreateTexture2D(TextureDescriptor);

        TextureDescriptor.m_pFileName = "textures/Lenna_moving.png";

        m_MovingTexture = TextureManager::CreateTexture2D(TextureDescriptor);
        
        TextureDescriptor.m_pFileName = nullptr;

        TextureDescriptor.m_NumberOfPixelsU = m_FixedTexture->GetNumberOfPixelsU();
        TextureDescriptor.m_NumberOfPixelsV = m_FixedTexture->GetNumberOfPixelsV();
        TextureDescriptor.m_NumberOfPixelsW = m_FixedTexture->GetNumberOfPixelsW();
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Format = CTexture::R32G32_FLOAT;

        m_GradientTexture = TextureManager::CreateTexture2D(TextureDescriptor);

		TextureDescriptor.m_Format = CTexture::R32G32B32A32_FLOAT;

		m_DebugTexture = TextureManager::CreateTexture2D(TextureDescriptor);
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
        m_GradientCSPtr = nullptr;
        m_SumTilesCSPtr = nullptr;
        m_SumFinalCSPtr = nullptr;

        m_ConstantBuffer = nullptr;
        m_SumBufferPtr = nullptr;

        m_FixedTexture = nullptr;
        m_MovingTexture = nullptr;

        m_GradientTexture = nullptr;

		m_DebugTexture = nullptr;
    }

} // namespace MR
