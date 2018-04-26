
// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "plugin/light_estimation_stitching/le_precompiled.h"

#include "engine/core/core_console.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_mesh_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_state_manager.h"
#include "engine/graphic/gfx_target_set_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

#include "plugin/light_estimation_stitching/le_plugin_interface.h"

#include <array>

// -----------------------------------------------------------------------------
// Register plugin
// -----------------------------------------------------------------------------
CORE_PLUGIN_INFO(LE::CPluginInterface, "Light Estimation Stitching", "1.0", "This plugin uses a plane to stitch the current camera image onto a cube map.")

// -----------------------------------------------------------------------------
// Definitions
// -----------------------------------------------------------------------------
namespace 
{
    Gfx::CTexturePtr CreateLUTFromOpenCV();
} // namespace 

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------
namespace LE
{
    void CPluginInterface::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Shader
        // -----------------------------------------------------------------------------
        m_VSPtr = Gfx::ShaderManager::CompileVS("../../plugins/light_estimation_stitching/vs.glsl", "main");;
        m_GSPtr = Gfx::ShaderManager::CompileGS("../../plugins/light_estimation_stitching/gs.glsl", "main");
        m_PSPtr = Gfx::ShaderManager::CompilePS("../../plugins/light_estimation_stitching/fs.glsl", "main");

        // -----------------------------------------------------------------------------
        // Buffer
        // -----------------------------------------------------------------------------
        SCubemapBuffer DefaultGSValues;
        
        std::array<glm::vec3, 6> LookDirections = {
            glm::vec3(-1.0f,  0.0f,  0.0f),
            glm::vec3(+1.0f,  0.0f,  0.0f),
            glm::vec3( 0.0f, -1.0f,  0.0f),
            glm::vec3( 0.0f, +1.0f,  0.0f),
            glm::vec3( 0.0f,  0.0f, +1.0f),
            glm::vec3( 0.0f,  0.0f, -1.0f),
        };

        std::array<glm::vec3, 6> UpDirections = {
            glm::vec3(0.0f, 1.0f,  0.0f),
            glm::vec3(0.0f, 1.0f,  0.0f),
            glm::vec3(0.0f, 0.0f, +1.0f),
            glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3(0.0f, 1.0f,  0.0f),
            glm::vec3(0.0f, 1.0f,  0.0f),
        };

        DefaultGSValues.m_CubeProjectionMatrix = glm::perspective(glm::half_pi<float>(), 1.0f, 0.1f, 1.0f);

        for (int IndexOfCubeface = 0; IndexOfCubeface < 6; ++IndexOfCubeface)
        {
            DefaultGSValues.m_CubeViewMatrix[IndexOfCubeface]  = glm::lookAt(glm::vec3(0.0f), LookDirections[IndexOfCubeface], UpDirections[IndexOfCubeface]);
#ifdef PLATFORM_WINDOWS
            DefaultGSValues.m_CubeViewMatrix[IndexOfCubeface] *= glm::eulerAngleX(glm::radians(90.0f));
#else
            DefaultGSValues.m_CubeViewMatrix[IndexOfCubeface] *= glm::eulerAngleX(glm::radians(-90.0f));
#endif // PLATFORM_WINDOWS
        }
        
        // -----------------------------------------------------------------------------

        Gfx::SBufferDescriptor ConstanteBufferDesc;

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SCubemapBuffer);
        ConstanteBufferDesc.m_pBytes        = &DefaultGSValues;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_CubemapBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SModelMatrixBuffer);
        ConstanteBufferDesc.m_pBytes        = 0;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_ModelMatrixBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------
        // Mesh
        // -----------------------------------------------------------------------------
        m_MeshPtr = Gfx::MeshManager::CreateBox(1.0f, 1.0f, 1.0f);

        // -----------------------------------------------------------------------------
        // Texture
        // -----------------------------------------------------------------------------
        Gfx::STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = 512;
        TextureDescriptor.m_NumberOfPixelsV  = 512;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = Gfx::STextureDescriptor::s_GenerateAllMipMaps;
        TextureDescriptor.m_NumberOfTextures = 6;
        TextureDescriptor.m_Binding          = Gfx::CTexture::ShaderResource | Gfx::CTexture::RenderTarget;
        TextureDescriptor.m_Access           = Gfx::CTexture::CPUWrite;
        TextureDescriptor.m_Format           = Gfx::CTexture::Unknown;
        TextureDescriptor.m_Usage            = Gfx::CTexture::GPURead;
        TextureDescriptor.m_Semantic         = Gfx::CTexture::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_Format           = Gfx::CTexture::R8G8B8_BYTE;

        m_OutputCubemapPtr = Gfx::TextureManager::CreateCubeTexture(TextureDescriptor);

        Gfx::TextureManager::SetTextureLabel(m_OutputCubemapPtr, "Sky cubemap from image");

        // -----------------------------------------------------------------------------

        m_InputTexturePtr = 0;

        // -----------------------------------------------------------------------------
        // Target Set
        // -----------------------------------------------------------------------------
        Gfx::CTexturePtr FirstMipmapCubeTexture = Gfx::TextureManager::GetMipmapFromTexture2D(m_OutputCubemapPtr, 0);

        m_TargetSetPtr = Gfx::TargetSetManager::CreateTargetSet(FirstMipmapCubeTexture);

        // -----------------------------------------------------------------------------
        // Viewport
        // -----------------------------------------------------------------------------
        Gfx::SViewPortDescriptor ViewPortDesc;

        ViewPortDesc.m_TopLeftX = 0.0f;
        ViewPortDesc.m_TopLeftY = 0.0f;
        ViewPortDesc.m_MinDepth = 0.0f;
        ViewPortDesc.m_MaxDepth = 1.0f;

        ViewPortDesc.m_Width  = static_cast<float>(FirstMipmapCubeTexture->GetNumberOfPixelsU());
        ViewPortDesc.m_Height = static_cast<float>(FirstMipmapCubeTexture->GetNumberOfPixelsV());

        Gfx::CViewPortPtr MipMapViewPort = Gfx::ViewManager::CreateViewPort(ViewPortDesc);

        m_ViewPortSetPtr = Gfx::ViewManager::CreateViewPortSet(MipMapViewPort);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        m_VSPtr = 0;
        m_GSPtr = 0;
        m_PSPtr = 0;
        m_CubemapBufferPtr = 0;
        m_ModelMatrixBufferPtr = 0;
        m_MeshPtr = 0;
        m_InputTexturePtr = 0;
        m_OutputCubemapPtr = 0;
        m_TargetSetPtr = 0;
        m_ViewPortSetPtr = 0;
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
        if (m_InputTexturePtr == nullptr) return;

        Gfx::Performance::BeginEvent("Light estimation from LUT");

        // -----------------------------------------------------------------------------
        // Setup constant buffer
        // -----------------------------------------------------------------------------
        SModelMatrixBuffer ViewBuffer;

        ViewBuffer.m_ModelMatrix = glm::mat4(Gfx::ViewManager::GetMainCamera()->GetView()->GetRotationMatrix());

        Gfx::BufferManager::UploadBufferData(m_ModelMatrixBufferPtr, &ViewBuffer);

        // -----------------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------------
        Gfx::ContextManager::SetTargetSet(m_TargetSetPtr);

        Gfx::ContextManager::SetViewPortSet(m_ViewPortSetPtr);

        Gfx::ContextManager::SetDepthStencilState(Gfx::StateManager::GetDepthStencilState(Gfx::CDepthStencilState::NoDepth));

        Gfx::ContextManager::SetBlendState(Gfx::StateManager::GetBlendState(Gfx::CBlendState::Default));

        Gfx::ContextManager::SetRasterizerState(Gfx::StateManager::GetRasterizerState(Gfx::CRasterizerState::NoCull));

        Gfx::ContextManager::SetTopology(Gfx::STopology::TriangleList);

        Gfx::ContextManager::SetShaderVS(m_VSPtr);

        Gfx::ContextManager::SetShaderGS(m_GSPtr);

        Gfx::ContextManager::SetShaderPS(m_PSPtr);

        Gfx::ContextManager::SetVertexBuffer(m_MeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());

        Gfx::ContextManager::SetIndexBuffer(m_MeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), 0);

        Gfx::ContextManager::SetInputLayout(m_MeshPtr->GetLOD(0)->GetSurface()->GetMVPShaderVS()->GetInputLayout());

        Gfx::ContextManager::SetConstantBuffer(0, m_ModelMatrixBufferPtr);
        Gfx::ContextManager::SetConstantBuffer(1, m_CubemapBufferPtr);

        Gfx::ContextManager::SetSampler(0, Gfx::SamplerManager::GetSampler(Gfx::CSampler::MinMagMipLinearClamp));
        Gfx::ContextManager::SetSampler(1, Gfx::SamplerManager::GetSampler(Gfx::CSampler::MinMagMipLinearClamp));

        Gfx::ContextManager::SetTexture(0, m_InputTexturePtr);

        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        Gfx::ContextManager::DrawIndexed(m_MeshPtr->GetLOD(0)->GetSurface()->GetNumberOfIndices(), 0, 0);

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        Gfx::ContextManager::ResetTexture(0);
        Gfx::ContextManager::ResetTexture(1);

        Gfx::ContextManager::ResetSampler(0);
        Gfx::ContextManager::ResetSampler(1);

        Gfx::ContextManager::ResetConstantBuffer(0);
        Gfx::ContextManager::ResetConstantBuffer(1);

        Gfx::ContextManager::ResetInputLayout();

        Gfx::ContextManager::ResetIndexBuffer();

        Gfx::ContextManager::ResetVertexBuffer();

        Gfx::ContextManager::ResetShaderVS();

        Gfx::ContextManager::ResetShaderGS();

        Gfx::ContextManager::ResetShaderPS();

        Gfx::ContextManager::ResetTopology();

        Gfx::ContextManager::ResetRenderContext();

        // -----------------------------------------------------------------------------
        // Update mip maps
        // -----------------------------------------------------------------------------
        Gfx::TextureManager::UpdateMipmap(m_OutputCubemapPtr);

        Gfx::Performance::EndEvent();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnPause()
    {
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::SetInputTexture(Gfx::CTexturePtr _InputTexturePtr)
    {
        m_InputTexturePtr = _InputTexturePtr;
    }

    // -----------------------------------------------------------------------------

    Gfx::CTexturePtr CPluginInterface::GetOutputCubemap()
    {
        return m_OutputCubemapPtr;
    }
} // namespace LE

extern "C" CORE_PLUGIN_API_EXPORT void SetInputTexture(Gfx::CTexturePtr _InputTexturePtr)
{
    static_cast<LE::CPluginInterface&>(GetInstance()).SetInputTexture(_InputTexturePtr);
}

extern "C" CORE_PLUGIN_API_EXPORT Gfx::CTexturePtr GetOutputCubemap()
{
    return static_cast<LE::CPluginInterface&>(GetInstance()).GetOutputCubemap();
}