//---Engine---
#include "plugin\stereo\stereo_precompiled.h"
#include "FutoGCV_upsample_fgi.h"

//---GLSL for GPU Parallel Programming---
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_texture.h"

namespace
{
    #define TileSize_2D 16 // Define size of work group for GPU parallel processing. <= 16 suggested for 2D image (based on experience).

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
            << "#define TILE_SIZE_2D " << TileSize_2D << " \n";
        std::string DefineString = DefineStream.str();
        m_FGS_CSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/FGI/cs_FGS.glsl", "main", DefineString.c_str());

        //---Initialize Buffer Manager---
        Gfx::SBufferDescriptor Param_BufferDesc = {};
        Param_BufferDesc.m_Stride = 0;
        Param_BufferDesc.m_Usage = Gfx::CBuffer::GPURead;
        Param_BufferDesc.m_Binding = Gfx::CBuffer::ConstantBuffer;
        Param_BufferDesc.m_Access = Gfx::CBuffer::CPUWrite;
        Param_BufferDesc.m_NumberOfBytes = sizeof(SFGIParam);
        Param_BufferDesc.m_pBytes = nullptr;
        Param_BufferDesc.m_pClassKey = 0;
        m_Parameter_BufferPtr = Gfx::BufferManager::CreateBuffer(Param_BufferDesc);
    }


    CFGI::~CFGI()
    {
    }

    //---Assist Functions---
    void CFGI::FGS(Gfx::CTexturePtr& Output_TexturePtr, const Gfx::CTexturePtr& Input_TexturePtr, const Gfx::CTexturePtr& Guide_TexturePtr)
    {
        //---Start WLS in GLSL---
        Gfx::Performance::BeginEvent("Planar Rectification");

        Gfx::ContextManager::SetShaderCS(m_FGS_CSPtr);
        Gfx::ContextManager::SetImageTexture(0, Output_TexturePtr);
        Gfx::ContextManager::SetImageTexture(1, Input_TexturePtr);
        Gfx::ContextManager::SetImageTexture(2, Guide_TexturePtr);
        Gfx::ContextManager::SetConstantBuffer(0, m_Parameter_BufferPtr);

        const int WorkGroupsX = DivUp(m_ImgSize_Rect.x, TileSize_2D);
        const int WorkGroupsY = DivUp(m_ImgSize_Rect.y, TileSize_2D);

        Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        Gfx::ContextManager::ResetShaderCS();

        Gfx::Performance::EndEvent();
        //---Finish WLS in GLSL---
    }

} // Namespace FutoGCV

