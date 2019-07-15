#include "plugin\stereo\stereo_precompiled.h"
#include "disp_upsampling_FGI.h"

//---GLSL for GPU Parallel Programming---
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_texture.h"

#include <sstream>

namespace // No specific namespace => Only allowed to use in this file.
{
    //---Definition for GPU Parallel Processing---
    #define TileSize_2D 16

    int DivUp(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }
}

namespace disparity
{
    //---Constructors & Destructor---
    CFGI::CFGI()
    {
        std::stringstream DefineStream;
        DefineStream
            << "#define TILE_SIZE_2D " << TileSize_2D << " \n"; // 16 for work group size is suggested for 2D image (based on experience).
        std::string DefineString = DefineStream.str();

        m_WLSCSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/FGI/cs_WLS.glsl", "main", DefineString.c_str());
    }


    CFGI::~CFGI()
    {
    }

    //---Execute Function---
    void CFGI::WLS(Gfx::CTexturePtr& Disp_HR, const Gfx::CTexturePtr& Disp_LR, const Gfx::CTexturePtr& Img_HR) // Directly up-sampling by WLS*1
    {
        // * GPU Start
        Gfx::Performance::BeginEvent("WLS");

        Gfx::ContextManager::SetShaderCS(m_WLSCSPtr);
        Gfx::ContextManager::SetImageTexture(0, Disp_LR);
        Gfx::ContextManager::SetImageTexture(1, Img_HR);
        Gfx::ContextManager::SetImageTexture(2, Disp_HR);
        Gfx::ContextManager::SetConstantBuffer();
        // * GPU End
    }

    void CFGI::CWLS()
    {

    }
    void CFGI::HCWLS()
    {

    }
    void CFGI::CHCWLS()
    {

    }

} // namespace depth


