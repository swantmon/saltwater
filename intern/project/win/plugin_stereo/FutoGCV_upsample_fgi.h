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
        CFGI();
        ~CFGI();

    //---Execute Functions---
    public:

    //---Assist Functions---
    private:
        void FGS1(Gfx::CTexturePtr& OutputData, const Gfx::CTexturePtr& InputData, const Gfx::CTexturePtr& GuideData);
    
    //---Members---
    private:
        const float m_Lamda_FGS1 = 900.f;
        const int m_Iteration_FGS1 = 3;
        const int m_Attenuation_FGS1 = 4;
        SFGSParameter m_Param_FGS1;

        //---GLSL---
        Gfx::CShaderPtr m_FGS_CSPtr;
        Gfx::CBufferPtr m_FGSParameter_BufferPtr;
    };
} // namespace FutoGCV


