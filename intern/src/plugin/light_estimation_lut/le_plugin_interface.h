
#pragma once

#include "engine/core/core_plugin_manager.h"

#include "base/base_include_glm.h"
#include "base/base_uncopyable.h"

#include "engine/graphic/gfx_buffer.h"
#include "engine/graphic/gfx_input_layout.h"
#include "engine/graphic/gfx_mesh.h"
#include "engine/graphic/gfx_shader.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_texture.h"

namespace LE
{
    class CPluginInterface : private Base::CUncopyable, public Core::IPlugin
    {
    public:

        void OnStart() override;
        void OnExit() override;
        void OnPause() override;
        void OnResume() override;
        void Update() override;

        void SetInputTexture(Gfx::CTexturePtr _InputTexturePtr);

        void SetOutputCubemap(Gfx::CTexturePtr _OutputCubemapPtr);
        Gfx::CTexturePtr GetOutputCubemap();

    private:

        struct SCubemapBuffer
        {
            glm::mat4 m_CubeProjectionMatrix;
            glm::mat4 m_CubeViewMatrix[6];
        };

        struct SModelMatrixBuffer
        {
            glm::mat4 m_ModelMatrix;
        };

    private:

        Gfx::CShaderPtr m_VSPtr;
        Gfx::CShaderPtr m_GSPtr;
        Gfx::CShaderPtr m_PSPtr;

        Gfx::CBufferPtr m_CubemapBufferPtr;
        Gfx::CBufferPtr m_ModelMatrixBufferPtr;

        Gfx::CMeshPtr m_MeshPtr;

        Gfx::CTexturePtr m_InputTexturePtr;
        Gfx::CTexturePtr m_LookUpTexturePtr;
        Gfx::CTexturePtr m_OutputCubemapPtr;

        Gfx::CTargetSetPtr m_TargetSetPtr;

        Gfx::CViewPortSetPtr m_ViewPortSetPtr;

        bool m_IsActive;
    };
} // namespace LE