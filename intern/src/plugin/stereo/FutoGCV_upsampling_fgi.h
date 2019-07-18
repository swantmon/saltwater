#pragma once

//---GLSL for GPU Parallel Programming---
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_buffer_manager.h"

//---Basic Calculation---
#include "base/base_include_glm.h" // Some warnings appears when directly #include "glm" in Engine

namespace FutoGCV
{
    struct SFGSParameter 
    {
        float m_Lamda;
        float m_Sigma = 0.05f;
        glm::ivec2 m_Direction;
    };

    class CFGI
    {
    //---Constructors & Destructor---
    public:
        CFGI(const glm::ivec2& OutputSize);
        ~CFGI();

    //---Execute Functions---
    public:
        void FGS(Gfx::CTexturePtr Output, const Gfx::CTexturePtr Input, const Gfx::CTexturePtr Guide);
        void FGI(Gfx::CTexturePtr Output_HR, const Gfx::CTexturePtr Input_SparseHR, const Gfx::CTexturePtr Guide_HR);

    //---Assist Functions---
    private:
        void WLS(Gfx::CTexturePtr OutputData, const Gfx::CTexturePtr InputData, const Gfx::CTexturePtr GuideData);
    
    //---Members---
    private:
        const float m_Lamda = 900.0f;
        const int m_Iteration = 3;
        const float m_Attenuation = 4.0f;
        SFGSParameter m_Param_WLS;

        //---GLSL---
        Gfx::CShaderPtr m_FGS_CSPtr;

        Gfx::CTexturePtr m_Intermediate_TexturePtr, m_Temp_TexturePtr;

        Gfx::CBufferPtr m_WLSParameter_BufferPtr;
    };
} // namespace FutoGCV


