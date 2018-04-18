
// -----------------------------------------------------------------------------
// Settings
// -----------------------------------------------------------------------------
#define HAS_OPENCV_SUPPORT 0

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
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

#if HAS_OPENCV_SUPPORT == 1
#include "opencv2/opencv.hpp"
#endif

#include <array>

// -----------------------------------------------------------------------------
// Register plugin
// -----------------------------------------------------------------------------
CORE_PLUGIN_INFO(LE::CPluginInterface, "Light Estimation LUT", "1.0", "This plugin generates a cubemap based on a precomputed look-up texture.")

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
        m_VSPtr = Gfx::ShaderManager::CompileVS("../../plugins/light_estimation_lut/vs.glsl", "main");;
        m_GSPtr = Gfx::ShaderManager::CompileGS("../../plugins/light_estimation_lut/gs.glsl", "main");
        m_PSPtr = Gfx::ShaderManager::CompilePS("../../plugins/light_estimation_lut/fs.glsl", "main");

        // -----------------------------------------------------------------------------
        // Buffer
        // -----------------------------------------------------------------------------
        SCubemapBuffer DefaultGSValues;
        
        std::array<glm::vec3, 6> LookDirections = {
            glm::vec3(+1.0f,  0.0f,  0.0f),
            glm::vec3(-1.0f,  0.0f,  0.0f),
            glm::vec3( 0.0f, +1.0f,  0.0f),
            glm::vec3( 0.0f, -1.0f,  0.0f),
            glm::vec3( 0.0f,  0.0f, +1.0f),
            glm::vec3( 0.0f,  0.0f, -1.0f),
        };

        std::array<glm::vec3, 6> UpDirections = {
            glm::vec3(0.0f, -1.0f,  0.0f),
            glm::vec3(0.0f, -1.0f,  0.0f),
            glm::vec3(0.0f,  0.0f,  1.0f),
            glm::vec3(0.0f,  0.0f, -1.0f),
            glm::vec3(0.0f, -1.0f,  0.0f),
            glm::vec3(0.0f, -1.0f,  0.0f),
        };

        DefaultGSValues.m_CubeProjectionMatrix = glm::perspective(glm::half_pi<float>(), 1.0f, 0.1f, 1.0f);

        for (int IndexOfCubeface = 0; IndexOfCubeface < 6; ++IndexOfCubeface)
        {
            DefaultGSValues.m_CubeViewMatrix[IndexOfCubeface]  = glm::lookAt(glm::vec3(0.0f), LookDirections[IndexOfCubeface], UpDirections[IndexOfCubeface]);
            DefaultGSValues.m_CubeViewMatrix[IndexOfCubeface] *= glm::eulerAngleX(glm::radians(90.0f));
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
        m_MeshPtr = Gfx::MeshManager::CreateSphereIsometric(1.0f, 2);

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

#if HAS_OPENCV_SUPPORT == 1
        m_LookUpTexturePtr = CreateLUTFromOpenCV();
#else
        TextureDescriptor.m_NumberOfPixelsU  = Gfx::STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsV  = Gfx::STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_NumberOfMipMaps  = Gfx::STextureDescriptor::s_GenerateAllMipMaps;
        TextureDescriptor.m_NumberOfTextures = 6;
        TextureDescriptor.m_Binding          = Gfx::CTexture::ShaderResource;
        TextureDescriptor.m_Access           = Gfx::CTexture::CPUWrite;
        TextureDescriptor.m_Format           = Gfx::CTexture::Unknown;
        TextureDescriptor.m_Usage            = Gfx::CTexture::GPURead;
        TextureDescriptor.m_Semantic         = Gfx::CTexture::Diffuse;
        TextureDescriptor.m_pFileName        = "../../plugins/light_estimation_lut/cubemap.dds";
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_Format           = Gfx::CTexture::R8G8B8A8_UBYTE;

        m_LookUpTexturePtr = Gfx::TextureManager::CreateCubeTexture(TextureDescriptor);
#endif

        Gfx::TextureManager::SetTextureLabel(m_LookUpTexturePtr, "Sky LUT");

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
        if (m_InputTexturePtr == nullptr) return;

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

        Gfx::ContextManager::SetConstantBuffer(0, m_ModelMatrixBufferPtr);
        Gfx::ContextManager::SetConstantBuffer(1, m_CubemapBufferPtr);

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
    static_cast<LE::CPluginInterface&>(GetInstance()).SetInputTexture(_InputTexturePtr);
}

extern "C" CORE_PLUGIN_API_EXPORT Gfx::CTexturePtr GetOutputCubemap()
{
    return static_cast<LE::CPluginInterface&>(GetInstance()).GetOutputCubemap();
}

#if HAS_OPENCV_SUPPORT == 1
namespace 
{
    Gfx::CTexturePtr CreateLUTFromOpenCV()
    {
        using namespace cv;

#define CROP_PERCENTAGE 0.8f
#define IMAGE_EDGE_LENGTH 512

        auto CropImage = [&](const Mat& _rOriginal, Mat& _rCroppedImage, Mat& _rLeftPart, Mat& _rRightPart, Mat& _rTopPart, Mat& _rBottomPart)
        {
            int LengthX;
            int LengthY;
            int ShortedLength;
            int PercentualShortedLength;

            LengthX = _rOriginal.size[0];
            LengthY = _rOriginal.size[1];

            ShortedLength = LengthX < LengthY ? LengthX : LengthY;

            PercentualShortedLength = static_cast<int>(static_cast<float>(ShortedLength) * CROP_PERCENTAGE);

            unsigned int X = (LengthX - PercentualShortedLength) / 2;
            unsigned int Y = (LengthY - PercentualShortedLength) / 2;

            Rect CroppedRectangel(Y, X, PercentualShortedLength, PercentualShortedLength);

            _rCroppedImage = _rOriginal(CroppedRectangel);

            resize(_rCroppedImage, _rCroppedImage, Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH));

            // -----------------------------------------------------------------------------

            _rLeftPart = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOriginal.type());
            _rRightPart = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOriginal.type());
            _rTopPart = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOriginal.type());
            _rBottomPart = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOriginal.type());

            // -----------------------------------------------------------------------------

            Point2f MaskPoints[4];
            Point2f DestPoints[4];
            Mat     WarpMat;

            DestPoints[0] = Point2f(static_cast<float>(0), static_cast<float>(0));
            DestPoints[1] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH - 1));
            DestPoints[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH - 1), static_cast<float>(IMAGE_EDGE_LENGTH - 1));
            DestPoints[3] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH - 1), static_cast<float>(0));

            // -----------------------------------------------------------------------------

            MaskPoints[0] = Point2f(static_cast<float>(0), static_cast<float>(0));
            MaskPoints[1] = Point2f(static_cast<float>(0), static_cast<float>(LengthX - 1));
            MaskPoints[2] = Point2f(static_cast<float>(Y - 1), static_cast<float>(X + PercentualShortedLength - 1));
            MaskPoints[3] = Point2f(static_cast<float>(Y - 1), static_cast<float>(X - 1));

            WarpMat = getPerspectiveTransform(MaskPoints, DestPoints);

            warpPerspective(_rOriginal, _rLeftPart, WarpMat, _rLeftPart.size());

            // -----------------------------------------------------------------------------

            MaskPoints[0] = Point2f(static_cast<float>(Y + PercentualShortedLength - 1), static_cast<float>(X - 1));
            MaskPoints[1] = Point2f(static_cast<float>(Y + PercentualShortedLength - 1), static_cast<float>(X + PercentualShortedLength - 1));
            MaskPoints[2] = Point2f(static_cast<float>(LengthY - 1), static_cast<float>(LengthX - 1));
            MaskPoints[3] = Point2f(static_cast<float>(LengthY - 1), static_cast<float>(0));

            WarpMat = getPerspectiveTransform(MaskPoints, DestPoints);

            warpPerspective(_rOriginal, _rRightPart, WarpMat, _rRightPart.size());

            // -----------------------------------------------------------------------------

            MaskPoints[0] = Point2f(static_cast<float>(0), static_cast<float>(0));
            MaskPoints[1] = Point2f(static_cast<float>(Y - 1), static_cast<float>(X - 1));
            MaskPoints[2] = Point2f(static_cast<float>(Y + PercentualShortedLength - 1), static_cast<float>(X - 1));
            MaskPoints[3] = Point2f(static_cast<float>(LengthY - 1), static_cast<float>(0));

            WarpMat = getPerspectiveTransform(MaskPoints, DestPoints);

            warpPerspective(_rOriginal, _rTopPart, WarpMat, _rTopPart.size());

            // -----------------------------------------------------------------------------

            MaskPoints[0] = Point2f(static_cast<float>(Y - 1), static_cast<float>(X + PercentualShortedLength - 1));
            MaskPoints[1] = Point2f(static_cast<float>(0), static_cast<float>(LengthX - 1));
            MaskPoints[2] = Point2f(static_cast<float>(LengthY - 1), static_cast<float>(LengthX - 1));
            MaskPoints[3] = Point2f(static_cast<float>(Y + PercentualShortedLength - 1), static_cast<float>(X + PercentualShortedLength - 1));

            WarpMat = getPerspectiveTransform(MaskPoints, DestPoints);

            warpPerspective(_rOriginal, _rBottomPart, WarpMat, _rBottomPart.size());
        };

        // -----------------------------------------------------------------------------

        auto CombineFaces = [&](const Mat& _rOne, const Mat& _rTwo, const Point2f* _pDestinationOne, const Point2f* _pDestinationTwo)->cv::Mat
        {
            Mat CombinedOne, CombinedTwo;

            CombinedOne = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOne.type());
            CombinedTwo = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOne.type());

            // -----------------------------------------------------------------------------

            Point2f MaskPoints[3];

            MaskPoints[0] = Point2f(static_cast<float>(0), static_cast<float>(0));
            MaskPoints[1] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH));
            MaskPoints[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH));

            Mat WarpMat;

            // -----------------------------------------------------------------------------

            WarpMat = getAffineTransform(MaskPoints, _pDestinationOne);

            warpAffine(_rOne, CombinedOne, WarpMat, CombinedOne.size());

            // -----------------------------------------------------------------------------

            WarpMat = getAffineTransform(MaskPoints, _pDestinationTwo);

            warpAffine(_rTwo, CombinedTwo, WarpMat, CombinedTwo.size());

            return CombinedOne + CombinedTwo;
        };

        // -----------------------------------------------------------------------------

        auto CombineRightFaces = [&](const Mat& _rOne, const Mat& _rTwo)->cv::Mat
        {
            Point2f DestPointsOne[3];
            Point2f DestPointsTwo[3];

            DestPointsOne[0] = Point2f(static_cast<float>(0), static_cast<float>(0));
            DestPointsOne[1] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsOne[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2), static_cast<float>(IMAGE_EDGE_LENGTH));

            DestPointsTwo[0] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2), static_cast<float>(0));
            DestPointsTwo[1] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2), static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsTwo[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH));

            Mat Combination = CombineFaces(_rOne, _rTwo, DestPointsOne, DestPointsTwo);

            return Combination;
        };

        // -----------------------------------------------------------------------------

        auto CombineLeftFaces = [&](const Mat& _rOne, const Mat& _rTwo)->cv::Mat
        {
            Point2f DestPointsOne[3];
            Point2f DestPointsTwo[3];

            DestPointsOne[0] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2), static_cast<float>(0));
            DestPointsOne[1] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2), static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsOne[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH));

            DestPointsTwo[0] = Point2f(static_cast<float>(0), static_cast<float>(0));
            DestPointsTwo[1] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsTwo[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2), static_cast<float>(IMAGE_EDGE_LENGTH));

            Mat Combination = CombineFaces(_rOne, _rTwo, DestPointsOne, DestPointsTwo);

            return Combination;
        };

        // -----------------------------------------------------------------------------

        auto CombineTopFaces = [&](const Mat& _rOne, const Mat& _rTwo)->cv::Mat
        {
            Point2f DestPointsOne[3];
            Point2f DestPointsTwo[3];

            DestPointsOne[0] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH / 2));
            DestPointsOne[1] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsOne[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH));

            DestPointsTwo[0] = Point2f(static_cast<float>(0), static_cast<float>(0));
            DestPointsTwo[1] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH / 2));
            DestPointsTwo[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH / 2));

            Mat Combination = CombineFaces(_rOne, _rTwo, DestPointsOne, DestPointsTwo);

            return Combination;
        };

        // -----------------------------------------------------------------------------

        auto CombineBottomFaces = [&](const Mat& _rOne, const Mat& _rTwo)->cv::Mat
        {
            Point2f DestPointsOne[3];
            Point2f DestPointsTwo[3];

            DestPointsOne[0] = Point2f(static_cast<float>(0), static_cast<float>(0));
            DestPointsOne[1] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH / 2));
            DestPointsOne[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH / 2));

            DestPointsTwo[0] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH / 2));
            DestPointsTwo[1] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsTwo[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH));

            Mat Combination = CombineFaces(_rOne, _rTwo, DestPointsOne, DestPointsTwo);

            return Combination;
        };

        cv::Mat OriginalFrontImage, FrontCroped, FrontLeftPart, FrontRightPart, FrontTopPart, FrontBottomPart;
        cv::Mat OriginalBackImage, BackCroped, BackLeftPart, BackRightPart, BackTopPart, BackBottomPart;

        cv::Mat CombinedRight, CombinedLeft, CombinedTop, CombinedBottom;

        CombinedRight.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_32FC2);
        CombinedLeft.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_32FC2);
        CombinedTop.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_32FC2);
        CombinedBottom.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_32FC2);
        FrontCroped.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_32FC2);
        BackCroped.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_32FC2);

        OriginalFrontImage.create(cv::Size(1280, 720), CV_32FC2);

        for (int CurrentY = 0; CurrentY < OriginalFrontImage.rows; CurrentY++)
        {
            for (int CurrentX = 0; CurrentX < OriginalFrontImage.cols; CurrentX++)
            {
                OriginalFrontImage.at<Vec2f>(Point(CurrentX, CurrentY)) = cv::Vec2f(static_cast<float>(CurrentX) / static_cast<float>(OriginalFrontImage.cols), static_cast<float>(CurrentY) / static_cast<float>(OriginalFrontImage.rows));
            }
        }

        // -----------------------------------------------------------------------------

        flip(OriginalFrontImage, OriginalBackImage, 1);

        // -----------------------------------------------------------------------------
        // Crop front image
        // -----------------------------------------------------------------------------
        CropImage(OriginalFrontImage, FrontCroped, FrontLeftPart, FrontRightPart, FrontTopPart, FrontBottomPart);
        CropImage(OriginalBackImage, BackCroped, BackLeftPart, BackRightPart, BackTopPart, BackBottomPart);

        // -----------------------------------------------------------------------------
        // Flip back images because of negative direction on back face
        // -----------------------------------------------------------------------------
        flip(BackTopPart, BackTopPart, -1);
        flip(BackBottomPart, BackBottomPart, -1);

        // -----------------------------------------------------------------------------
        // Fill Images
        // -----------------------------------------------------------------------------
        CombinedRight = CombineRightFaces(FrontRightPart, BackLeftPart);
        CombinedLeft = CombineLeftFaces(FrontLeftPart, BackRightPart);
        CombinedTop = CombineTopFaces(FrontTopPart, BackTopPart);
        CombinedBottom = CombineBottomFaces(FrontBottomPart, BackBottomPart);

        // -----------------------------------------------------------------------------
        // Create and update texture
        // -----------------------------------------------------------------------------
        Gfx::STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = IMAGE_EDGE_LENGTH;
        TextureDescriptor.m_NumberOfPixelsV  = IMAGE_EDGE_LENGTH;
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
        TextureDescriptor.m_Format           = Gfx::CTexture::R32G32_FLOAT;

        Gfx::CTexturePtr LookUpTexturePtr = Gfx::TextureManager::CreateCubeTexture(TextureDescriptor);

        glm::ivec2 CubemapResolution = glm::ivec2(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH);

        Base::AABB2UInt CubemapRect(glm::ivec2(0), CubemapResolution);

        Gfx::TextureManager::CopyToTextureArray2D(LookUpTexturePtr, 0, CubemapRect, CubemapRect[1][0], CombinedRight.data, false);
        Gfx::TextureManager::CopyToTextureArray2D(LookUpTexturePtr, 1, CubemapRect, CubemapRect[1][0], CombinedLeft.data, false);
        Gfx::TextureManager::CopyToTextureArray2D(LookUpTexturePtr, 2, CubemapRect, CubemapRect[1][0], CombinedTop.data, false);
        Gfx::TextureManager::CopyToTextureArray2D(LookUpTexturePtr, 3, CubemapRect, CubemapRect[1][0], CombinedBottom.data, false);
        Gfx::TextureManager::CopyToTextureArray2D(LookUpTexturePtr, 4, CubemapRect, CubemapRect[1][0], FrontCroped.data, false);
        Gfx::TextureManager::CopyToTextureArray2D(LookUpTexturePtr, 5, CubemapRect, CubemapRect[1][0], BackCroped.data, false);

        Gfx::TextureManager::UpdateMipmap(LookUpTexturePtr);

        return LookUpTexturePtr;
    }
} // namespace 
#endif