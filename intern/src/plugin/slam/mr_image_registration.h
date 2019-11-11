
#pragma once

#include "base/base_exception.h"
#include "base/base_include_glm.h"

#include "engine/core/core_program_parameters.h"

#include "engine/engine.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_main.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_selection.h"
#include "engine/graphic/gfx_selection_renderer.h"
#include "engine/graphic/gfx_view_manager.h"

#include "plugin/slam/mr_slam_reconstructor.h"

namespace MR
{
    class CImageRegistrator
    {
    public:

        void Register();
        void Register(Gfx::CTexturePtr _FixedTexture, Gfx::CTexturePtr _MovingTexture);

    public:

        CImageRegistrator();
        CImageRegistrator(int _Width, int _Height);
        ~CImageRegistrator();

    private:

        void SetupBuffers();
        void SetupShaders();
        void SetupStates();
        void SetupTextures();

    private:

        int m_Width, m_Height;
        bool m_UseTestData;
        
        Gfx::CShaderPtr m_GradientCSPtr;
        Gfx::CShaderPtr m_SumTilesCSPtr;
        Gfx::CShaderPtr m_SumFinalCSPtr;
        Gfx::CShaderPtr m_OutputCSPtr;

        Gfx::CBufferPtr m_ConstantBuffer;
        Gfx::CBufferPtr m_SumBufferPtr;
        Gfx::CBufferPtr m_SumCountBufferPtr;

        Gfx::CTexturePtr m_FixedTexture;
        Gfx::CTexturePtr m_MovingTexture;

		Gfx::CTexturePtr m_GradientTexture;

        Gfx::CTexturePtr m_DebugTexture;
        Gfx::CTexturePtr m_OutputTexture;
    };
} // namespace Scpt