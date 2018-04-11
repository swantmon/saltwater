
#include "plugin/light_estimation_lut/le_precompiled.h"

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

#include "plugin/light_estimation_lut/le_plugin_interface.h"

CORE_PLUGIN_INFO(LE::CPluginInterface, "Light Estimation LUT", "1.0", "This plugin generates a cubemap based on a precomputed look-up texture.")

namespace LE
{
    void CPluginInterface::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Shader
        // -----------------------------------------------------------------------------
        m_CubemapVSPtr = Gfx::ShaderManager::CompileVS("vs_spherical_env_cubemap_generation.glsl", "main");;
        m_CubemapGSPtr = Gfx::ShaderManager::CompileGS("gs_spherical_rotate_env_cubemap_generation.glsl", "main");
        m_CubemapPSPtr = Gfx::ShaderManager::CompilePS("fs_lut_env_cubemap_generation.glsl", "main");

        // -----------------------------------------------------------------------------
        // Buffer
        // -----------------------------------------------------------------------------
        Gfx::SBufferDescriptor ConstanteBufferDesc;

        glm::vec3 EyePosition = glm::vec3(0.0f);
        glm::vec3 UpDirection;
        glm::vec3 LookDirection;
        
        SCubemapBufferGS DefaultGSValues;
        
        DefaultGSValues.m_CubeProjectionMatrix = glm::perspective(glm::half_pi<float>(), 1.0f, 0.1f, 20000.0f);
        
        // -----------------------------------------------------------------------------
        // By creating a cube map in OpenGL, several facts should be considered:
        //  1. OpenGL cubemaps has an left handed coord system inside the cube and
        //    right handed coord system outside the cube
        //  2. Texcoords starts in the upper left corner (normally in the lower left 
        //     corner)
        //
        // RHS:
        //          +--------+
        //          |        |
        //          |   Y+   |
        //          |        |
        // +--------+--------+--------+--------+
        // |        |        |        |        |
        // |   X-   |   Z+   |   X+   |   Z-   |
        // |        |        |        |        |
        // +--------+--------+--------+--------+
        //          |        |
        //          |   Y-   |
        //          |        |
        //          +--------+
        //
        // LHS:
        //          +--------+
        //          |        |
        //          |   Y+   |
        //          |        |
        // +--------+--------+--------+--------+
        // |        |        |        |        |
        // |   X-   |   Z-   |   X+   |   Z+   |
        // |        |        |        |        |
        // +--------+--------+--------+--------+
        //          |        |
        //          |   Y-   |
        //          |        |
        //          +--------+
        // -----------------------------------------------------------------------------

        // -----------------------------------------------------------------------------
        // Creating VS matrix for spherical image to cube map:
        // -> Viewer is inside the cube > LHS
        // -----------------------------------------------------------------------------
        LookDirection = EyePosition + glm::vec3(1.0f, 0.0f, 0.0f);
        UpDirection   = glm::vec3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[0] = glm::lookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition - glm::vec3(1.0f, 0.0f, 0.0f);
        UpDirection   = glm::vec3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[1] = glm::lookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + glm::vec3(0.0f, 1.0f, 0.0f);
        UpDirection   = -glm::vec3(0.0f, 0.0f, -1.0f);;
        
        DefaultGSValues.m_CubeViewMatrix[2] = glm::lookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition - glm::vec3(0.0f, 1.0f, 0.0f);
        UpDirection   = glm::vec3(0.0f, 0.0f, -1.0f);;
        
        DefaultGSValues.m_CubeViewMatrix[3] = glm::lookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition + glm::vec3(0.0f, 0.0f, 1.0f);
        UpDirection   = glm::vec3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[4] = glm::lookAt(EyePosition, LookDirection, UpDirection);

        // -----------------------------------------------------------------------------
        
        LookDirection = EyePosition - glm::vec3(0.0f, 0.0f, 1.0f);
        UpDirection   = glm::vec3(0.0f, -1.0f, 0.0f);
        
        DefaultGSValues.m_CubeViewMatrix[5] = glm::lookAt(EyePosition, LookDirection, UpDirection);
        
        // -----------------------------------------------------------------------------
        
        for (unsigned int IndexOfCubeface = 0; IndexOfCubeface < 6; ++ IndexOfCubeface)
        {
            DefaultGSValues.m_CubeViewMatrix[IndexOfCubeface] *= glm::eulerAngleX(glm::radians(-90.0f));
        }
        
        // -----------------------------------------------------------------------------
        
        ConstanteBufferDesc.m_Stride        = 0;
        ConstanteBufferDesc.m_Usage         = Gfx::CBuffer::GPURead;
        ConstanteBufferDesc.m_Binding       = Gfx::CBuffer::ConstantBuffer;
        ConstanteBufferDesc.m_Access        = Gfx::CBuffer::CPUWrite;
        ConstanteBufferDesc.m_NumberOfBytes = sizeof(SCubemapBufferGS);
        ConstanteBufferDesc.m_pBytes        = &DefaultGSValues;
        ConstanteBufferDesc.m_pClassKey     = 0;
        
        m_CubemapGSWorldRotatedBuffer = Gfx::BufferManager::CreateBuffer(ConstanteBufferDesc);

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
        m_MeshPtr = Gfx::MeshManager::CreateSphereIsometric(1.0f, 3);

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

        Gfx::TextureManager::SetTextureLabel(m_OutputCubemapPtr, "Pre-LUT Light");

        // -----------------------------------------------------------------------------

        TextureDescriptor.m_NumberOfPixelsU  = 512;
        TextureDescriptor.m_NumberOfPixelsV  = 512;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = Gfx::STextureDescriptor::s_GenerateAllMipMaps;
        TextureDescriptor.m_NumberOfTextures = 6;
        TextureDescriptor.m_Binding          = Gfx::CTexture::ShaderResource;
        TextureDescriptor.m_Access           = Gfx::CTexture::CPUWrite;
        TextureDescriptor.m_Format           = Gfx::CTexture::Unknown;
        TextureDescriptor.m_Usage            = Gfx::CTexture::GPURead;
        TextureDescriptor.m_Semantic         = Gfx::CTexture::Diffuse;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_Format           = Gfx::CTexture::R8G8B8A8_UBYTE;

        m_LookUpTexturePtr = Gfx::TextureManager::CreateCubeTexture(TextureDescriptor);

        Gfx::TextureManager::SetTextureLabel(m_LookUpTexturePtr, "LUT");

        TextureDescriptor.m_pFileName        = "face_x.png";
        TextureDescriptor.m_NumberOfTextures = 1;
        Gfx::CTexturePtr FaceXP = Gfx::TextureManager::CreateTexture2D(TextureDescriptor, true, Gfx::SDataBehavior::Copy);

        TextureDescriptor.m_pFileName = "face_xm.png";
        Gfx::CTexturePtr FaceXM = Gfx::TextureManager::CreateTexture2D(TextureDescriptor, true, Gfx::SDataBehavior::Copy);

        TextureDescriptor.m_pFileName = "face_y.png";
        Gfx::CTexturePtr FaceYP = Gfx::TextureManager::CreateTexture2D(TextureDescriptor, true, Gfx::SDataBehavior::Copy);

        TextureDescriptor.m_pFileName = "face_ym.png";
        Gfx::CTexturePtr FaceYM = Gfx::TextureManager::CreateTexture2D(TextureDescriptor, true, Gfx::SDataBehavior::Copy);

        TextureDescriptor.m_pFileName = "face_z.png";
        Gfx::CTexturePtr FaceZP = Gfx::TextureManager::CreateTexture2D(TextureDescriptor, true, Gfx::SDataBehavior::Copy);

        TextureDescriptor.m_pFileName = "face_zm.png";
        Gfx::CTexturePtr FaceZM = Gfx::TextureManager::CreateTexture2D(TextureDescriptor, true, Gfx::SDataBehavior::Copy);

        Gfx::TextureManager::CopyToTextureArray2D(m_LookUpTexturePtr, 0, FaceXP);
        Gfx::TextureManager::CopyToTextureArray2D(m_LookUpTexturePtr, 1, FaceXM);
        Gfx::TextureManager::CopyToTextureArray2D(m_LookUpTexturePtr, 2, FaceYP);
        Gfx::TextureManager::CopyToTextureArray2D(m_LookUpTexturePtr, 3, FaceYM);
        Gfx::TextureManager::CopyToTextureArray2D(m_LookUpTexturePtr, 4, FaceZP);
        Gfx::TextureManager::CopyToTextureArray2D(m_LookUpTexturePtr, 5, FaceZM);

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
        m_CubemapVSPtr = 0;
        m_CubemapGSPtr = 0;
        m_CubemapPSPtr = 0;
        m_CubemapGSWorldRotatedBuffer = 0;
        m_ModelMatrixBufferPtr = 0;
        m_MeshPtr = 0;
        m_InputTexturePtr = 0;
        m_LookUpTexturePtr = 0;
        m_OutputCubemapPtr = 0;
        m_TargetSetPtr = 0;
        m_ViewPortSetPtr = 0;
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
        //if (m_InputTexturePtr == nullptr) return;

        Gfx::Performance::BeginEvent("Light estimation from LUT");

        // -----------------------------------------------------------------------------
        // Setup constant buffer
        // -----------------------------------------------------------------------------
        SModelMatrixBuffer ViewBuffer;

        ViewBuffer.m_ModelMatrix  = glm::mat4(1.0f);
        ViewBuffer.m_ModelMatrix *= glm::mat4(Gfx::ViewManager::GetMainCamera()->GetView()->GetRotationMatrix());

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

        Gfx::ContextManager::SetShaderVS(m_CubemapVSPtr);

        Gfx::ContextManager::SetShaderGS(m_CubemapGSPtr);

        Gfx::ContextManager::SetShaderPS(m_CubemapPSPtr);

        Gfx::ContextManager::SetVertexBuffer(m_MeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());

        Gfx::ContextManager::SetIndexBuffer(m_MeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), 0);

        Gfx::ContextManager::SetInputLayout(m_MeshPtr->GetLOD(0)->GetSurface()->GetMVPShaderVS()->GetInputLayout());

        Gfx::ContextManager::SetConstantBuffer(0, m_CubemapGSWorldRotatedBuffer);
        Gfx::ContextManager::SetConstantBuffer(1, m_ModelMatrixBufferPtr);

        Gfx::ContextManager::SetSampler(0, Gfx::SamplerManager::GetSampler(Gfx::CSampler::MinMagMipLinearClamp));
        Gfx::ContextManager::SetSampler(1, Gfx::SamplerManager::GetSampler(Gfx::CSampler::MinMagMipLinearClamp));

        Gfx::ContextManager::SetTexture(0, m_InputTexturePtr);
        Gfx::ContextManager::SetTexture(1, m_LookUpTexturePtr);

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
    LE::CPluginInterface::GetInstance().SetInputTexture(_InputTexturePtr);
}

extern "C" CORE_PLUGIN_API_EXPORT Gfx::CTexturePtr GetOutputCubemap()
{
    return LE::CPluginInterface::GetInstance().GetOutputCubemap();
}