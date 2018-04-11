
#pragma once

#include "engine/core/core_plugin_manager.h"

#include "base/base_include_glm.h"

#include "engine/graphic/gfx_buffer.h"
#include "engine/graphic/gfx_input_layout.h"
#include "engine/graphic/gfx_mesh.h"
#include "engine/graphic/gfx_shader.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_texture.h"

namespace LE
{
    class CPluginInterface : public Core::IPlugin
    {
    public:

        void OnStart() override;
        void OnExit() override;
        void OnPause() override;
        void OnResume() override;
        void Update() override;

        void SetInputTexture(Gfx::CTexturePtr _InputTexturePtr);

        Gfx::CTexturePtr GetOutputCubemap();

    private:

        struct SModelMatrixBuffer
        {
            glm::mat4 m_ModelMatrix;
        };

        struct SCubemapBufferGS
        {
            glm::mat4 m_CubeProjectionMatrix;
            glm::mat4 m_CubeViewMatrix[6];
        };

    private:

        Gfx::CShaderPtr m_CubemapVSPtr;
        Gfx::CShaderPtr m_CubemapGSPtr;
        Gfx::CShaderPtr m_CubemapPSPtr;

        Gfx::CBufferPtr m_CubemapGSWorldRotatedBuffer;
        Gfx::CBufferPtr m_ModelMatrixBufferPtr;

        Gfx::CMeshPtr m_MeshPtr;

        Gfx::CTexturePtr m_InputTexturePtr;
        Gfx::CTexturePtr m_LookUpTexturePtr;
        Gfx::CTexturePtr m_OutputCubemapPtr;

        Gfx::CTargetSetPtr m_TargetSetPtr;

        Gfx::CViewPortSetPtr m_ViewPortSetPtr;
    };
} // namespace LE