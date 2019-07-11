#pragma once

//---GLSL for GPU Parallel Programming---
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_buffer_manager.h"

namespace disparity
{
    class CFGI
    {
    //---Constructors / Destructure---
    public:
        CFGI();
        ~CFGI();

    //---Execute Function---
    public:
        void WLS(Gfx::CTexturePtr& Disp_HR, const Gfx::CTexturePtr& Disp_LR, const Gfx::CTexturePtr& Img_HR); // Directly up-sampling by WLS*1
        void CWLS(); // Directly up-sampling by Cascaded WLS (WLS*2)
        void HCWLS(); // Hierarchical up-sampling by WLS*2
        void CHCWLS(); // Consensus-based Hierarchical up-sampling by WLS*2

    //---Assistant Function---
    private:
        

    //---Members---
    private:


        Gfx::CShaderPtr m_WLSCSPtr;
    };
} // namespace depth



