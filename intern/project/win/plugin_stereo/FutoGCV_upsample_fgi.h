#pragma once

//---GLSL for GPU Parallel Programming---
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_buffer_manager.h"

//---Basic Calculation---
#include "base/base_include_glm.h" // Some warnings appears when directly #include "glm" in Engine

namespace FutoGCV
{
    struct SFGIParam 
    {
        glm::vec2 m_Lamda = glm::vec2(900.0f, 100.0f);
        glm::vec2 m_Sigma = glm::vec2(0.005f, 0.005f);
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
        void FGS(Gfx::CTexturePtr& OutputData, const Gfx::CTexturePtr& InputData, const Gfx::CTexturePtr& GuideData);
    
    //---Members---
    private:
        SFGIParam m_Parameter;

        //---GLSL---
        Gfx::CShaderPtr m_FGS_CSPtr;
        Gfx::CBufferPtr m_Parameter_BufferPtr;
    };
} // namespace FutoGCV


