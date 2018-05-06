
// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "plugin/light_estimation_stitching/le_precompiled.h"

#include "engine/core/core_console.h"

#include "engine/graphic/gfx_buffer_manager.h"
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_main.h"
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
        m_VSPtr = Gfx::ShaderManager::CompileVS("../../plugins/light_estimation_stitching/vs.glsl", "main");
        m_GSPtr = Gfx::ShaderManager::CompileGS("../../plugins/light_estimation_stitching/gs.glsl", "main");
        m_PSPtr = Gfx::ShaderManager::CompilePS("../../plugins/light_estimation_stitching/fs.glsl", "main");

        // -----------------------------------------------------------------------------
        // Input layout
        // -----------------------------------------------------------------------------
        const Gfx::SInputElementDescriptor P3T2InputLayout[] =
        {
            { "POSITION", 0, Gfx::CInputLayout::Float3Format, 0,  0, 20, Gfx::CInputLayout::PerVertex, 0 },
            { "TEXCOORD", 0, Gfx::CInputLayout::Float2Format, 0, 12, 20, Gfx::CInputLayout::PerVertex, 0 },
        };

        Gfx::ShaderManager::CreateInputLayout(P3T2InputLayout, 2, m_VSPtr);

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

        DefaultGSValues.m_CubeProjectionMatrix = glm::perspective(glm::half_pi<float>(), 1.0f, 0.01f, 8192.0f);

        for (int IndexOfCubeface = 0; IndexOfCubeface < 6; ++IndexOfCubeface)
        {
            DefaultGSValues.m_CubeViewMatrix[IndexOfCubeface]  = glm::lookAt(glm::vec3(0.0f), LookDirections[IndexOfCubeface], UpDirections[IndexOfCubeface]);
            DefaultGSValues.m_CubeViewMatrix[IndexOfCubeface] *= glm::eulerAngleX(glm::radians(-90.0f));
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
        // Mesh
        // -----------------------------------------------------------------------------
        static float PlaneVertexBufferData[] =
        {
          //x   , y   , z   , tx  , ty
            0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        };
        
        static unsigned int PlaneIndexBufferData[] =
        {
            0, 1, 2, 0, 2, 3,
        };
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::VertexBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(PlaneVertexBufferData);
        ConstanteBufferDesc.m_pBytes        = &PlaneVertexBufferData[0];
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_VertexBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::IndexBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(PlaneIndexBufferData);
        ConstanteBufferDesc.m_pBytes        = &PlaneIndexBufferData[0];
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_IndexBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

        // -----------------------------------------------------------------------------

        glm::vec2 DefaultProperties(0.0f, 0.0f);

        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(glm::vec2);
        ConstanteBufferDesc.m_pBytes        = &DefaultProperties;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_PropertiesBufferPtr = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

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
        TextureDescriptor.m_Format           = Gfx::CTexture::R8G8B8A8_BYTE;

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
        m_VertexBufferPtr = 0;
        m_IndexBufferPtr = 0;
        m_InputTexturePtr = 0;
        m_OutputCubemapPtr = 0;
        m_TargetSetPtr = 0;
        m_ViewPortSetPtr = 0;
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
        if (m_InputTexturePtr == nullptr) return;

        Gfx::Performance::BeginEvent("Light estimation from far plane");

        Gfx::CCameraPtr MainCameraPtr = Gfx::ViewManager::GetMainCamera();
        Gfx::CViewPtr   MainViewPtr = MainCameraPtr->GetView();

        const glm::vec3* pWorldSpaceCameraFrustum = MainCameraPtr->GetWorldSpaceFrustum();

        glm::vec3 FarBottomLeft = pWorldSpaceCameraFrustum[4];
        glm::vec3 FarTopLeft = pWorldSpaceCameraFrustum[5];
        glm::vec3 FarBottomRight = pWorldSpaceCameraFrustum[6];
        glm::vec3 FarTopRight = pWorldSpaceCameraFrustum[7];

        // -----------------------------------------------------------------------------
        // Calculate far plane and setup plane
        // -----------------------------------------------------------------------------
        float PlaneGeometryBuffer[20];

        PlaneGeometryBuffer[0] = FarTopLeft[0];
        PlaneGeometryBuffer[1] = FarTopLeft[1];
        PlaneGeometryBuffer[2] = FarTopLeft[2];
        PlaneGeometryBuffer[3] = 0.0f;
        PlaneGeometryBuffer[4] = 1.0f;

        PlaneGeometryBuffer[5] = FarTopRight[0];
        PlaneGeometryBuffer[6] = FarTopRight[1];
        PlaneGeometryBuffer[7] = FarTopRight[2];
        PlaneGeometryBuffer[8] = 1.0f;
        PlaneGeometryBuffer[9] = 1.0f;

        PlaneGeometryBuffer[10] = FarBottomRight[0];
        PlaneGeometryBuffer[11] = FarBottomRight[1];
        PlaneGeometryBuffer[12] = FarBottomRight[2];
        PlaneGeometryBuffer[13] = 1.0f;
        PlaneGeometryBuffer[14] = 0.0f;

        PlaneGeometryBuffer[15] = FarBottomLeft[0];
        PlaneGeometryBuffer[16] = FarBottomLeft[1];
        PlaneGeometryBuffer[17] = FarBottomLeft[2];
        PlaneGeometryBuffer[18] = 0.0f;
        PlaneGeometryBuffer[19] = 0.0f;

        Gfx::BufferManager::UploadBufferData(m_VertexBufferPtr, &PlaneGeometryBuffer);

        // -----------------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------------
        Gfx::ContextManager::SetTargetSet(m_TargetSetPtr);

        Gfx::ContextManager::SetViewPortSet(m_ViewPortSetPtr);

        Gfx::ContextManager::SetDepthStencilState(Gfx::StateManager::GetDepthStencilState(Gfx::CDepthStencilState::NoDepth));

        Gfx::ContextManager::SetBlendState(Gfx::StateManager::GetBlendState(Gfx::CBlendState::AlphaBlend));

        Gfx::ContextManager::SetRasterizerState(Gfx::StateManager::GetRasterizerState(Gfx::CRasterizerState::NoCull));

        Gfx::ContextManager::SetTopology(Gfx::STopology::TriangleList);

        Gfx::ContextManager::SetShaderVS(m_VSPtr);

        Gfx::ContextManager::SetShaderGS(m_GSPtr);

        Gfx::ContextManager::SetShaderPS(m_PSPtr);

        Gfx::ContextManager::SetVertexBuffer(m_VertexBufferPtr);

        Gfx::ContextManager::SetIndexBuffer(m_IndexBufferPtr, 0);

        Gfx::ContextManager::SetInputLayout(m_VSPtr->GetInputLayout());

        Gfx::ContextManager::SetConstantBuffer(0, Gfx::Main::GetPerFrameConstantBuffer());
        Gfx::ContextManager::SetConstantBuffer(1, m_CubemapBufferPtr);
        Gfx::ContextManager::SetConstantBuffer(2, m_PropertiesBufferPtr);

        Gfx::ContextManager::SetSampler(0, Gfx::SamplerManager::GetSampler(Gfx::CSampler::MinMagMipLinearClamp));

        Gfx::ContextManager::SetTexture(0, m_InputTexturePtr);

        // -----------------------------------------------------------------------------
        // Draw
        // -----------------------------------------------------------------------------
        Gfx::ContextManager::DrawIndexed(6, 0, 0);

        // -----------------------------------------------------------------------------
        // Reset
        // -----------------------------------------------------------------------------
        Gfx::ContextManager::ResetTexture(0);

        Gfx::ContextManager::ResetSampler(0);

        Gfx::ContextManager::ResetConstantBuffer(0);
        Gfx::ContextManager::ResetConstantBuffer(1);
        Gfx::ContextManager::ResetConstantBuffer(2);

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

    // -----------------------------------------------------------------------------

    void CPluginInterface::SetFlipVertical(bool _Value)
    {
        glm::vec2 Properties = _Value ? glm::vec2(0.0f, 1.0f) : glm::vec2(0.0f, 0.0f);

        Gfx::BufferManager::UploadBufferData(m_PropertiesBufferPtr, &Properties);
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

extern "C" CORE_PLUGIN_API_EXPORT void SetFlipVertical(bool _Value)
{
    static_cast<LE::CPluginInterface&>(GetInstance()).SetFlipVertical(_Value);
}