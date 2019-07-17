//---Engine---
#include "plugin\stereo\stereo_precompiled.h"
#include "FutoGCV_upsample_fgi.h"

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
}

namespace FutoGCV
{
    //---Constructors & Destructor---
    CFGI::CFGI()
    {
        //---Initialize Shader Manager---
        std::stringstream DefineStream;
        DefineStream
            << "#define TILE_SIZE_1D " << TileSize_1D << " \n";
        std::string DefineString = DefineStream.str();
        m_FGS_CSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/FGI/cs_FGS.glsl", "main", DefineString.c_str());

        //---Initialize Buffer Manager---
        Gfx::SBufferDescriptor Param_BufferDesc = {};
        Param_BufferDesc.m_Stride = 0;
        Param_BufferDesc.m_Usage = Gfx::CBuffer::GPURead;
        Param_BufferDesc.m_Binding = Gfx::CBuffer::ConstantBuffer;
        Param_BufferDesc.m_Access = Gfx::CBuffer::CPUWrite;
        Param_BufferDesc.m_NumberOfBytes = sizeof(SFGSParameter);
        Param_BufferDesc.m_pBytes = nullptr;
        Param_BufferDesc.m_pClassKey = 0;
        m_FGSParameter_BufferPtr = Gfx::BufferManager::CreateBuffer(Param_BufferDesc);
    }


    CFGI::~CFGI()
    {
    }

    //---Assist Functions---
    void CFGI::FGS1(Gfx::CTexturePtr& Output_TexturePtr, const Gfx::CTexturePtr& Input_TexturePtr, const Gfx::CTexturePtr& Guide_TexturePtr)
    {

        const int WorkGroupsX = DivUp(Output_TexturePtr->GetNumberOfPixelsV, TileSize_1D);

        for (int iter = 0; iter < m_Iteration_FGS1; iter++)
        {
            //---Start Horizontal FGS1 in GLSL---
            Gfx::Performance::BeginEvent("Horizontal FGS1");

            m_Param_FGS1.m_Direction = glm::ivec2(1, 0); // Means horizontal direction

            float LamdaRatio = std::powf(m_Attenuation_FGS1, m_Iteration_FGS1 - iter) / (std::powf(m_Attenuation_FGS1, m_Iteration_FGS1) - 1.0f);
            m_Param_FGS1.m_Lamda = 1.5f * LamdaRatio * m_Lamda_FGS1;

            Gfx::BufferManager::UploadBufferData(m_FGSParameter_BufferPtr, &m_Param_FGS1);

            Gfx::ContextManager::SetShaderCS(m_FGS_CSPtr);
            Gfx::ContextManager::SetImageTexture(0, Output_TexturePtr);
            Gfx::ContextManager::SetImageTexture(1, Input_TexturePtr);
            Gfx::ContextManager::SetImageTexture(2, Guide_TexturePtr);
            Gfx::ContextManager::SetConstantBuffer(0, m_FGSParameter_BufferPtr);

            Gfx::ContextManager::Dispatch(WorkGroupsX, 1, 1);

            Gfx::ContextManager::ResetShaderCS();

            Gfx::Performance::EndEvent();
            //---Finish Horizontal FGS1 in GLSL---

            //---Start Vertical FGS1 in GLSL---
            Gfx::Performance::BeginEvent("Vertical FGS1");

            Gfx::ContextManager::SetShaderCS(m_FGS_CSPtr);
            Gfx::ContextManager::SetImageTexture(0, Output_TexturePtr);
            Gfx::ContextManager::SetImageTexture(1, Input_TexturePtr);
            Gfx::ContextManager::SetImageTexture(2, Guide_TexturePtr);
            Gfx::ContextManager::SetConstantBuffer(0, m_FGSParameter_BufferPtr);

            Gfx::ContextManager::Dispatch(WorkGroupsX, 1, 1);

            Gfx::ContextManager::ResetShaderCS();

            Gfx::Performance::EndEvent();
            //---Finish Vertical FGS1 in GLSL---
        }


        
    }

} // Namespace FutoGCV

