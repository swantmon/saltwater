
#pragma once

#include "engine/core/core_plugin_manager.h"

#include "base/base_include_glm.h"
#include "base/base_uncopyable.h"

#include "engine/data/data_component_manager.h"

#include "engine/graphic/gfx_buffer.h"
#include "engine/graphic/gfx_input_layout.h"
#include "engine/graphic/gfx_mesh.h"
#include "engine/graphic/gfx_shader.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_texture.h"

#include "engine/network/core_network_manager.h"

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

        Gfx::CTexturePtr GetOutputCubemap();

        void SetActive(bool _Flag);

		void FillEnvironmentWithNN();

	private:

		static const unsigned int s_PanoramaWidth = 256;
		static const unsigned int s_PanoramaHeight = 128;
		static const unsigned int s_CubemapSize = 512;

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

		Gfx::CShaderPtr m_C2PShaderPtr;
		Gfx::CShaderPtr m_P2CShaderPtr;
		Gfx::CShaderPtr m_FusePanoramaShaderPtr;

        Gfx::CBufferPtr m_CubemapBufferPtr;

        Gfx::CBufferPtr m_VertexBufferPtr;

        Gfx::CTexturePtr m_InputTexturePtr;
        Gfx::CTexturePtr m_StitchingCubemapPtr;
        Gfx::CTexturePtr m_EstimationCubemapPtr;
		Gfx::CTexturePtr m_PanoramaTexturePtr;
        Gfx::CTexturePtr m_EstimationPanoramaTexturePtr;
		Gfx::CTexturePtr m_NNPanoramaTexturePtr;

        Gfx::CTargetSetPtr m_TargetSetPtr;

        Gfx::CViewPortSetPtr m_ViewPortSetPtr;

        bool m_IsActive;

		Engine::CEventDelegates::HandleType m_GfxOnUpdateDelegate;

		Net::CNetworkManager::CMessageDelegate::HandleType m_NetworkDelegate;
        Net::SocketHandle m_SocketHandle;

        Dt::CComponentManager::CComponentDelegate::HandleType m_OnDirtyComponentDelegate;

        bool m_UseNeuralNetwork;

    private:

		void OnNewMessage(const Net::CMessage& _rMessage, Net::SocketHandle _SocketHandle);

        void Gfx_OnUpdate();

        void OnDirtyComponent(Dt::IComponent* _pComponent);
    };
} // namespace LE