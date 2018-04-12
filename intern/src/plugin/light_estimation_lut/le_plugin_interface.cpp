
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
    const char c_VS[] = R"(
        // -----------------------------------------------------------------------------
        // Built-In variables
        // -----------------------------------------------------------------------------
        out gl_PerVertex
        {
            vec4 gl_Position;
        };

        // -----------------------------------------------------------------------------
        // Input from buffer
        // -----------------------------------------------------------------------------
        layout(location = 0) in vec3 VertexPosition;

        // -----------------------------------------------------------------------------
        // Output to next stage
        // -----------------------------------------------------------------------------
        layout(location = 0) out vec3 out_Normal;

        // -----------------------------------------------------------------------------
        // Main
        // -----------------------------------------------------------------------------
        void main(void)
        {
            vec4 WSPosition = vec4(VertexPosition.xyz, 1.0f);
    
            out_Normal = normalize(WSPosition.xyz);
    
            gl_Position = WSPosition;
        }
    )";

    const char c_GS[] = R"(
        // -----------------------------------------------------------------------------
        // Built-In variables
        // -----------------------------------------------------------------------------
        in gl_PerVertex
        {
            vec4  gl_Position;
        } gl_in[];

        out gl_PerVertex
        {
            vec4 gl_Position;
        };

        // -----------------------------------------------------------------------------
        // Geometry definition
        // -----------------------------------------------------------------------------
        layout(triangles) in;
        layout(triangle_strip, max_vertices = 18) out;

        // -----------------------------------------------------------------------------
        // Input from engine
        // -----------------------------------------------------------------------------
        layout(std140, binding = 0) uniform UB0
        {
            mat4 m_CubeProjectionMatrix;
            mat4 m_CubeViewMatrix[6];
        };

        layout(std140, binding = 1) uniform UB1
        {
            mat4 m_ModelMatrix;
        };

        // -----------------------------------------------------------------------------
        // Input from previous stage
        // -----------------------------------------------------------------------------
        layout(location = 0) in vec3 in_Normal[];
        layout(location = 1) in vec2 in_UV[];

        // -----------------------------------------------------------------------------
        // Output to next stage
        // -----------------------------------------------------------------------------
        layout(location = 0) out vec3 out_Normal;
        layout(location = 1) out vec2 out_UV;

        // -----------------------------------------------------------------------------
        // Main
        // -----------------------------------------------------------------------------
        void main() 
        {
            for( int FaceIndex = 0; FaceIndex < 6; ++FaceIndex )
            {
                for( int IndexOfVertex = 0; IndexOfVertex < 3; IndexOfVertex++ )
                {
                    gl_Layer = FaceIndex;
            
                    out_Normal  = -in_Normal[IndexOfVertex];
                    out_UV      = in_UV[IndexOfVertex];
                    gl_Position = m_CubeProjectionMatrix * m_CubeViewMatrix[FaceIndex] * m_ModelMatrix * gl_in[IndexOfVertex].gl_Position;

                    EmitVertex();
                }

                EndPrimitive();
            }
        }
    )";

    const char c_FS[] = R"(
        // -----------------------------------------------------------------------------
        // Input from engine
        // -----------------------------------------------------------------------------
        layout(binding = 0) uniform sampler2D in_InputTexture;
        layout(binding = 1) uniform samplerCube in_LookUpTexture;

        // -----------------------------------------------------------------------------
        // Input to fragment from VS
        // -----------------------------------------------------------------------------
        layout(location = 0) in vec3 in_Normal;
        layout(location = 1) in vec2 in_UV;

        // -----------------------------------------------------------------------------
        // Output to fragment
        // -----------------------------------------------------------------------------
        layout(location = 0) out vec4 out_Output;

        // -----------------------------------------------------------------------------
        // Main
        // -----------------------------------------------------------------------------
        void main(void)
        {
            vec4 LookUp = textureLod(in_LookUpTexture, in_Normal, 0.0f);

            vec4 FinalColor = texture(in_InputTexture, LookUp.xy);
        
            out_Output = vec4(LookUp.xyz, 1.0f);
        }
    )";
} // namespace LE

namespace LE
{
    void CPluginInterface::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Shader
        // -----------------------------------------------------------------------------
        m_VSPtr = Gfx::ShaderManager::CompileVS(c_VS, "main", nullptr, nullptr, 0, false, false, true);
        m_GSPtr = Gfx::ShaderManager::CompileGS(c_GS, "main", nullptr, nullptr, 0, false, false, true);
        m_PSPtr = Gfx::ShaderManager::CompilePS(c_FS, "main", nullptr, nullptr, 0, false, false, true);

        // -----------------------------------------------------------------------------
        // Buffer
        // -----------------------------------------------------------------------------
        Gfx::SBufferDescriptor ConstanteBufferDesc;

        glm::vec3 EyePosition = glm::vec3(0.0f);
        glm::vec3 UpDirection;
        glm::vec3 LookDirection;
        
        SCubemapBuffer DefaultGSValues;
        
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

        Gfx::TextureManager::SetTextureLabel(m_OutputCubemapPtr, "Sky cubemap from image");

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

        Gfx::TextureManager::SetTextureLabel(m_LookUpTexturePtr, "Sky LUT");

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
        m_VSPtr = 0;
        m_GSPtr = 0;
        m_PSPtr = 0;
        m_CubemapBufferPtr = 0;
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

        Gfx::ContextManager::SetShaderVS(m_VSPtr);

        Gfx::ContextManager::SetShaderGS(m_GSPtr);

        Gfx::ContextManager::SetShaderPS(m_PSPtr);

        Gfx::ContextManager::SetVertexBuffer(m_MeshPtr->GetLOD(0)->GetSurface()->GetVertexBuffer());

        Gfx::ContextManager::SetIndexBuffer(m_MeshPtr->GetLOD(0)->GetSurface()->GetIndexBuffer(), 0);

        Gfx::ContextManager::SetInputLayout(m_MeshPtr->GetLOD(0)->GetSurface()->GetMVPShaderVS()->GetInputLayout());

        Gfx::ContextManager::SetConstantBuffer(0, m_CubemapBufferPtr);
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

        Gfx::ContextManager::ResetConstantBuffer(2);
        Gfx::ContextManager::ResetConstantBuffer(3);

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