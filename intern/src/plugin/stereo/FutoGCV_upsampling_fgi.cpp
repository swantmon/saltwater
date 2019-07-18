//---Engine---
#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\FutoGCV_upsampling_fgi.h"

//---GLSL for GPU Parallel Programming---
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_texture.h"

namespace
{
    #define TileSize_1D 64 

    int DivUp(int TotalShaderCount, int WorkGroupSize) // Calculate number of work groups.
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }

    Gfx::CTexturePtr Temp1_TexturePtr, Temp2_TexturePtr;
}

namespace FutoGCV
{
    //---Constructors & Destructor---
    CFGI::CFGI()
    {
    }

    CFGI::CFGI(const glm::ivec2& OutputSize)
    {
        //---Initialize Shader Manager---
        std::stringstream DefineStream;
        DefineStream
            << "#define TILE_SIZE_1D " << TileSize_1D << " \n";
        std::string DefineString = DefineStream.str();
        m_FGS_CSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/Scaling/FGI/cs_WLS_1D.glsl", "main", DefineString.c_str());

        //---Initialize Texture Manager---
        Gfx::STextureDescriptor TextureDesc = {};
        TextureDesc.m_NumberOfPixelsU = OutputSize.x;
        TextureDesc.m_NumberOfPixelsV = OutputSize.y;
        TextureDesc.m_NumberOfPixelsW = 1;
        TextureDesc.m_NumberOfMipMaps = 1;
        TextureDesc.m_NumberOfTextures = 1;
        TextureDesc.m_Binding = Gfx::CTexture::ShaderResource;
        TextureDesc.m_Access = Gfx::CTexture::EAccess::CPUWrite;
        TextureDesc.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
        TextureDesc.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextureDesc.m_Format = Gfx::CTexture::R32_FLOAT; // 1 channels with 32-bit float.

        Temp1_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc);
        Temp2_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc);

        //---Initialize Buffer Manager---
        Gfx::SBufferDescriptor Param_BufferDesc = {};
        Param_BufferDesc.m_Stride = 0;
        Param_BufferDesc.m_Usage = Gfx::CBuffer::GPURead;
        Param_BufferDesc.m_Binding = Gfx::CBuffer::ConstantBuffer;
        Param_BufferDesc.m_Access = Gfx::CBuffer::CPUWrite;
        Param_BufferDesc.m_NumberOfBytes = sizeof(SFGSParameter);
        Param_BufferDesc.m_pBytes = nullptr;
        Param_BufferDesc.m_pClassKey = 0;
        m_WLSParameter_BufferPtr = Gfx::BufferManager::CreateBuffer(Param_BufferDesc);
    }

    CFGI::~CFGI()
    {
        m_FGS_CSPtr = nullptr;
        m_WLSParameter_BufferPtr = nullptr;
    }

    //---Execute Functions---
    void CFGI::FGS(Gfx::CTexturePtr Output_TexturePtr, const Gfx::CTexturePtr Input_TexturePtr, const Gfx::CTexturePtr Guide_TexturePtr)
    {
        WLS_1D(Output_TexturePtr, Input_TexturePtr, Guide_TexturePtr);
    }

    void CFGI::FGI(Gfx::CTexturePtr Output_HR, const Gfx::CTexturePtr Input_SparseHR, const Gfx::CTexturePtr Guide_HR)
    {

        /*
            * Input_SparseHR = Input_HR_Interpolation .* Mask <- Already masked before transmitted

            D = FGS(Input_SparseHR, Guide_Color_HR)
            M = FGS(Mask, Guide_Color_HR)
            Output_HR = D ./ M
        */

    }

    //---Assist Functions---
    void CFGI::WLS_1D(Gfx::CTexturePtr Output_TexturePtr, const Gfx::CTexturePtr Input_TexturePtr, const Gfx::CTexturePtr Guide_TexturePtr)
    {

        const int WorkGroupsX = DivUp(Output_TexturePtr->GetNumberOfPixelsV(), TileSize_1D);


        for (int iter = 1; iter <= m_Iteration; iter++)
        {
            float LamdaRatio = std::powf(m_Attenuation, static_cast<float>(m_Iteration - iter)) / (std::powf(m_Attenuation, static_cast<float>(m_Iteration)) - 1.0f);
            m_Param_WLS.m_Lamda = 1.5f * LamdaRatio * m_Lamda;
            
            //---Start Horizontal WLS in GLSL---
            Gfx::Performance::BeginEvent("Horizontal WLS");

            m_Param_WLS.m_Direction = glm::ivec2(1, 0); // Horizontal direction

            Gfx::BufferManager::UploadBufferData(m_WLSParameter_BufferPtr, &m_Param_WLS);

            Gfx::ContextManager::SetShaderCS(m_FGS_CSPtr);

            Gfx::ContextManager::SetConstantBuffer(0, m_WLSParameter_BufferPtr);

            Gfx::ContextManager::SetImageTexture(0, Temp1_TexturePtr);
            Gfx::ContextManager::SetImageTexture(2, Guide_TexturePtr);
            if (iter == 1)
            {
                Gfx::ContextManager::SetImageTexture(1, Input_TexturePtr);
            }
            else
            {
                Gfx::ContextManager::SetImageTexture(1, Temp2_TexturePtr);
            }

            Gfx::ContextManager::Dispatch(WorkGroupsX, 1, 1);

            Gfx::ContextManager::ResetShaderCS();

            Gfx::Performance::EndEvent();
            //---Finish Horizontal FGS1 in GLSL---

            //---Start Vertical WLS in GLSL---
            Gfx::Performance::BeginEvent("Vertical WLS");

            m_Param_WLS.m_Direction = glm::ivec2(0, 1); // Vertical direction

            Gfx::BufferManager::UploadBufferData(m_WLSParameter_BufferPtr, &m_Param_WLS);

            Gfx::ContextManager::SetShaderCS(m_FGS_CSPtr);

            Gfx::ContextManager::SetConstantBuffer(0, m_WLSParameter_BufferPtr);

            Gfx::ContextManager::SetImageTexture(1, Temp1_TexturePtr);
            Gfx::ContextManager::SetImageTexture(2, Guide_TexturePtr);
            if (iter == m_Iteration)
            {
                Gfx::ContextManager::SetImageTexture(0, Output_TexturePtr);
            } 
            else
            {
                Gfx::ContextManager::SetImageTexture(0, Temp2_TexturePtr);
            }

            Gfx::ContextManager::Dispatch(WorkGroupsX, 1, 1);

            Gfx::ContextManager::ResetShaderCS();

            Gfx::Performance::EndEvent();
            //---Finish Vertical FGS1 in GLSL---
        }

    }

} // Namespace FutoGCV

