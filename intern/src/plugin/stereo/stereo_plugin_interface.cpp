
//---Engine---
#include "plugin/stereo/stereo_precompiled.h"
#include "engine/engine.h"
#include "plugin/stereo/stereo_plugin_interface.h"

//---Config---
#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h"

//---GLSL for GPU Parallel Programming---
#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_texture.h"


CORE_PLUGIN_INFO(Stereo::CPluginInterface, "Stereo Matching", "1.0", "This plugin takes RGB and transformation data and provides 2.5D depth maps")

namespace
{
    #define TileSize_2D 16 // Define size of work group for GPU parallel processing. <= 16 suggested for 2D image (based on experience).

    int DivUp(int TotalShaderCount, int WorkGroupSize) // Calculate number of work groups.
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }

    struct SParallaxEquation // Info for Parallax Equation in GLSL.
    {
        float m_BaselineLength; // Unit = meter
        float m_FocalLength; // Unit = pixel
        glm::ivec2 m_Padding;
    };
}

namespace Stereo
{
    void CPluginInterface::SetIntrinsics(const glm::vec2& _rFocalLength, const glm::vec2& _rFocalPoint, const glm::ivec2& _rImageSize)
    {
        BASE_UNUSED(_rFocalLength); // Avoid warning about unused variable.
        BASE_UNUSED(_rFocalPoint); // Avoid warning about unused variable.

        //===== 00. Input Data =====

        m_OrigImgSize = glm::ivec3(_rImageSize, 4); // Width, Height, Channel

        //---Initialize OrigImg Texture Manager---
        Gfx::STextureDescriptor TextDesc_OrigImg = {};
        TextDesc_OrigImg.m_NumberOfPixelsU = m_OrigImgSize.x;
        TextDesc_OrigImg.m_NumberOfPixelsV = m_OrigImgSize.y;
        TextDesc_OrigImg.m_NumberOfPixelsW = 1;
        TextDesc_OrigImg.m_NumberOfMipMaps = 1;
        TextDesc_OrigImg.m_NumberOfTextures = 1;
        TextDesc_OrigImg.m_Binding = Gfx::CTexture::ShaderResource;
        TextDesc_OrigImg.m_Access = Gfx::CTexture::EAccess::CPURead;
        TextDesc_OrigImg.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
        TextDesc_OrigImg.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextDesc_OrigImg.m_Format = Gfx::CTexture::R8G8B8A8_UBYTE; // 4 channels and each channel is 8-bit.

        m_OrigImg_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextDesc_OrigImg);

        //===== 00. Select Keyframe =====

        m_KeyfCondition_BaseLineL = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_keyframe:baseline_length(m)", 0.03f); // Unit = meter

        //===== 01. Epipolarization =====

        m_Rectifier_Planar = FutoGCV::CPlanarRectification(m_OrigImgSize);

        //---Initialize Buffer Manager---
        Gfx::SBufferDescriptor BufferDesc_Homo = {};
        BufferDesc_Homo.m_Stride = 0;
        BufferDesc_Homo.m_Usage = Gfx::CBuffer::GPURead;
        BufferDesc_Homo.m_Binding = Gfx::CBuffer::ConstantBuffer;
        BufferDesc_Homo.m_Access = Gfx::CBuffer::CPUWrite;
        BufferDesc_Homo.m_NumberOfBytes = sizeof(FutoGCV::SHomography);
        BufferDesc_Homo.m_pBytes = nullptr;
        BufferDesc_Homo.m_pClassKey = nullptr;

        m_Homography_Curt_BufferPtr = Gfx::BufferManager::CreateBuffer(BufferDesc_Homo);
        m_Homography_Last_BufferPtr = Gfx::BufferManager::CreateBuffer(BufferDesc_Homo);

        //===== 02. Stereo Matching =====

        m_DisparityRange = Core::CProgramParameters::GetInstance().Get("mr:stereo:02_stereo_matching:disparity_range", 128);

        //===== 03 EpiDisparity to EpiDepth =====

        std::stringstream DefineStream;
        DefineStream
            << "#define TILE_SIZE_2D " << TileSize_2D << " \n";
        std::string DefineString = DefineStream.str();

        m_Disp2Depth_CSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/cs_Disparity_to_Depth.glsl", "main", DefineString.c_str());
        m_UpSampling_BiLinear_CSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/Scaling/cs_upsampling_bilinear.glsl", "main", DefineString.c_str());

        Gfx::SBufferDescriptor ParaxEq_BufferDesc = {};
        ParaxEq_BufferDesc.m_Stride = 0;
        ParaxEq_BufferDesc.m_Usage = Gfx::CBuffer::GPURead;
        ParaxEq_BufferDesc.m_Binding = Gfx::CBuffer::ConstantBuffer;
        ParaxEq_BufferDesc.m_Access = Gfx::CBuffer::CPUWrite;
        ParaxEq_BufferDesc.m_NumberOfBytes = sizeof(SParallaxEquation);
        ParaxEq_BufferDesc.m_pBytes = nullptr;
        ParaxEq_BufferDesc.m_pClassKey = nullptr;
        m_ParaxEq_BufferPtr = Gfx::BufferManager::CreateBuffer(ParaxEq_BufferDesc);

        //===== 04 EpiDepth to OrigDepth =====
        m_Depth_Rect2Orig_CSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/cs_Depth_Rect2Orig.glsl", "main", DefineString.c_str());

        //===== 05 Compare Depth between plugin_stereo & Sensor =====
        m_Is_CompareDepth = Core::CProgramParameters::GetInstance().Get("mr:stereo:05_depth_analysis:compare_sensor", true);

        m_Compare_Depth_CSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/cs_Compare_Depth.glsl", "main", DefineString.c_str());

        //===== 06 Return Results =====
        m_IsExport_OrigImg = Core::CProgramParameters::GetInstance().Get("mr:stereo:06_output:export_orig_img", true);
        m_IsExport_EpiImg = Core::CProgramParameters::GetInstance().Get("mr:stereo:06_output:export_rect_img", true);
        m_IsExport_Depth = Core::CProgramParameters::GetInstance().Get("mr:stereo:06_output:export_depth", true);
    }

    /*
    {


        //===== 02. Stereo Matching =====

        if (m_IsScaling)
        {
            m_pStereoMatcher_LibSGM = std::make_unique<sgm::StereoSGM>(m_EpiImgSize_LR.x, m_EpiImgSize_LR.y, m_DispRange, 8, 16, sgm::EXECUTE_INOUT_HOST2HOST);
                // Default disparity is pixel level => Disparity is the same in 8-bit & 16-bit.
                // If turn on sub-pixel => Output disparity must be 16-bit. => Divided by 16 to derive true disparity!!!;
        }




        //---Depth from EpiImg to OrigImg---
        Gfx::STextureDescriptor TextDesc_OrigDepth = {};
        TextDesc_OrigDepth.m_NumberOfPixelsU = m_OrigImgSize.x;
        TextDesc_OrigDepth.m_NumberOfPixelsV = m_OrigImgSize.y;
        TextDesc_OrigDepth.m_NumberOfPixelsW = 1;
        TextDesc_OrigDepth.m_NumberOfMipMaps = 1;
        TextDesc_OrigDepth.m_NumberOfTextures = 1;
        TextDesc_OrigDepth.m_Binding = Gfx::CTexture::ShaderResource;
        TextDesc_OrigDepth.m_Access = Gfx::CTexture::EAccess::CPUWrite;
        TextDesc_OrigDepth.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
        TextDesc_OrigDepth.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextDesc_OrigDepth.m_Format = Gfx::CTexture::R16_UINT; // 1 channels with 16-bit uint.
        TextDesc_OrigDepth.m_pFileName = nullptr;
        TextDesc_OrigDepth.m_pPixels = nullptr;

        m_OrigDepth_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextDesc_OrigDepth);
        m_OrigDepth_Sensor_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextDesc_OrigDepth);

        TextDesc_OrigDepth.m_Format = Gfx::CTexture::R16_INT; // 1 channels with 16-bit int.

        m_OrigDepth_Diff_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextDesc_OrigDepth);
    }
    */

    // -----------------------------------------------------------------------------

    bool CPluginInterface::GetLatestFrameCPU(std::vector<char>& _ColorImage, std::vector<char>& _rDepthImage, glm::mat4& _rTransform)
    {
        BASE_UNUSED(_ColorImage);
        BASE_UNUSED(_rDepthImage);
        BASE_UNUSED(_rTransform);
        return false;
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4& _Transform, const glm::vec2& _FocalLength, const glm::vec2& _FocalPoint, const std::vector<uint16_t>& _rDepthImage)
    {

        //=====00 Input Data from ARKit=====

        //---Image Data---
        Base::AABB2UInt TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_OrigImgSize.x, m_OrigImgSize.y));
        Gfx::TextureManager::CopyToTexture2D(m_OrigImg_TexturePtr, TargetRect, m_OrigImgSize.x, static_cast<const void*>(_rRGBImage.data()));

        //---Interior Orientations---
        glm::mat3 K = glm::mat3(_FocalLength.x, 0, 0, 0, _FocalLength.y, 0, _FocalPoint.x, _FocalPoint.y, 1); // ARKit's Intrinsic

        //---Exterior Orientations---
        glm::mat3 R = glm::transpose(glm::mat3(_Transform));// ARKit's Rotation is Camera2World (Computer Graphics), but we need Rotation as World2Camera (Computer Vision).
        glm::vec3 PC = glm::vec3(_Transform[3]); // The last column of _Transform given by ARKit is the Position of Camera in World.

        //=====00 Keyframe Selection=====

        //---Set 1st Keyframe---
        if (!m_KeyfStatus)
        {
            m_OrigKeyframe_Curt = FutoGCV::SFutoImg(m_OrigImg_TexturePtr, m_OrigImgSize, K, R, PC);
            m_KeyfStatus = true;

            return;
        }

        //---Select 2nd Keyframe based on BaseLine Length---
        float BaseLineLength = glm::l2Norm(PC - m_OrigKeyframe_Curt.m_Position);

        if (BaseLineLength < m_KeyfCondition_BaseLineL)
        {
            return;
        }

        m_OrigKeyframe_Last = m_OrigKeyframe_Curt;
        m_OrigKeyframe_Curt = FutoGCV::SFutoImg(m_OrigImg_TexturePtr, m_OrigImgSize, K, R, PC);

        m_KeyfID++;

        //---Export Result ?---
        if (m_IsExport_OrigImg)
        {
            export_OrigImg();
        }

        //===== 01 Epipolarization =====

        m_Rectifier_Planar.execute(m_EpiKeyframe_Curt, m_Homography_Curt_BufferPtr, 
                                   m_EpiKeyframe_Last, m_Homography_Last_BufferPtr, 
                                   m_OrigKeyframe_Curt, m_OrigKeyframe_Last);

        std::vector<char> vecEpiImg_Curt(m_EpiKeyframe_Curt.m_ImgSize.x * m_EpiKeyframe_Curt.m_ImgSize.y, 0);
        Gfx::TextureManager::CopyTextureToCPU(m_EpiKeyframe_Curt.m_Img_TexturePtr, reinterpret_cast<char*>(vecEpiImg_Curt.data()));

        std::vector<char> vecEpiImg_Last(m_EpiKeyframe_Last.m_ImgSize.x * m_EpiKeyframe_Last.m_ImgSize.y, 0);
        Gfx::TextureManager::CopyTextureToCPU(m_EpiKeyframe_Last.m_Img_TexturePtr, reinterpret_cast<char*>(vecEpiImg_Last.data()));

        if (m_IsExport_EpiImg)
        {
            std::string ExportStr;

            cv::Mat cvEpiImg_Curt(m_EpiKeyframe_Curt.m_ImgSize.y, m_EpiKeyframe_Curt.m_ImgSize.x, CV_8UC1);
            memcpy(cvEpiImg_Curt.data, vecEpiImg_Curt.data(), vecEpiImg_Curt.size() * sizeof(vecEpiImg_Curt[0]));
            ExportStr = "E:\\Project_ARCHITECT\\EpiImg_Curt_" + std::to_string(m_KeyfID) + ".png";
            cv::imwrite(ExportStr, cvEpiImg_Curt);

            cv::Mat cvEpiImg_Last(m_EpiKeyframe_Last.m_ImgSize.y, m_EpiKeyframe_Last.m_ImgSize.x, CV_8UC1);
            memcpy(cvEpiImg_Last.data, vecEpiImg_Last.data(), vecEpiImg_Last.size() * sizeof(vecEpiImg_Last[0]));
            ExportStr = "E:\\Project_ARCHITECT\\EpiImg_Last_" + std::to_string(m_KeyfID) + ".png";
            cv::imwrite(ExportStr, cvEpiImg_Last);
        }

        //===== 02. Stereo Matching =====

        //---Compute EpiDisparity by Implementing LibSGM---
        std::vector<uint16_t> vecEpiDisparity_uint16(m_EpiKeyframe_Curt.m_ImgSize.x * m_EpiKeyframe_Curt.m_ImgSize.y, 0);

        m_pStereoMatcher_LibSGM = std::make_unique<sgm::StereoSGM>(m_EpiKeyframe_Curt.m_ImgSize.x, m_EpiKeyframe_Curt.m_ImgSize.y, m_DisparityRange, 8, 16, sgm::EXECUTE_INOUT_HOST2HOST);
            // Default disparity is pixel level => Disparity is the same in 8-bit & 16-bit.
            // If turn on sub-pixel => Output disparity must be 16-bit. => Divided by 16 to derive true disparity!!!

        m_pStereoMatcher_LibSGM->execute(vecEpiImg_Curt.data(), vecEpiImg_Last.data(), vecEpiDisparity_uint16.data());

        //---Initialize Texture Manager of EpiDisparity---
        std::vector<float> vecEpiDisparity = std::vector<float>::vector(vecEpiDisparity_uint16.begin(), vecEpiDisparity_uint16.end());

        Gfx::STextureDescriptor TextureDesc_EpiDisp = {};
        TextureDesc_EpiDisp.m_NumberOfPixelsU = m_EpiKeyframe_Curt.m_ImgSize.x;
        TextureDesc_EpiDisp.m_NumberOfPixelsV = m_EpiKeyframe_Curt.m_ImgSize.y;
        TextureDesc_EpiDisp.m_NumberOfPixelsW = 1;
        TextureDesc_EpiDisp.m_NumberOfMipMaps = 1;
        TextureDesc_EpiDisp.m_NumberOfTextures = 1;
        TextureDesc_EpiDisp.m_Binding = Gfx::CTexture::ShaderResource;
        TextureDesc_EpiDisp.m_Access = Gfx::CTexture::EAccess::CPUWrite;
        TextureDesc_EpiDisp.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
        TextureDesc_EpiDisp.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextureDesc_EpiDisp.m_Format = Gfx::CTexture::R32_FLOAT; // 1 channels with 32-bit float.

        m_EpiDisparity_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc_EpiDisp);

        TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_EpiKeyframe_Curt.m_ImgSize.x, m_EpiKeyframe_Curt.m_ImgSize.y));
        Gfx::TextureManager::CopyToTexture2D(m_EpiDisparity_TexturePtr, TargetRect, m_EpiKeyframe_Curt.m_ImgSize.x, static_cast<const void*>(vecEpiDisparity.data()));

        //===== 03. EpiDisparity to EpiDepth =====

        imp_Disp2Depth();

        //---Set Texture Manager---
        Gfx::STextureDescriptor TextureDesc_EpiDepth = {};
        TextureDesc_EpiDepth.m_NumberOfPixelsU = m_EpiKeyframe_Curt.m_ImgSize.x;
        TextureDesc_EpiDepth.m_NumberOfPixelsV = m_EpiKeyframe_Curt.m_ImgSize.y;
        TextureDesc_EpiDepth.m_NumberOfPixelsW = 1;
        TextureDesc_EpiDepth.m_NumberOfMipMaps = 1;
        TextureDesc_EpiDepth.m_NumberOfTextures = 1;
        TextureDesc_EpiDepth.m_Binding = Gfx::CTexture::ShaderResource;
        TextureDesc_EpiDepth.m_Access = Gfx::CTexture::EAccess::CPUWrite;
        TextureDesc_EpiDepth.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
        TextureDesc_EpiDepth.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextureDesc_EpiDepth.m_Format = Gfx::CTexture::R16_UINT; // 1 channels with 16-bit uint.

        m_EpiDepth_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc_EpiDepth);

        //---Set Buffer Manager---
        SParallaxEquation ParaxEqInfo;
        ParaxEqInfo.m_BaselineLength = glm::l2Norm(m_EpiKeyframe_Curt.m_Position - m_EpiKeyframe_Last.m_Position);
        ParaxEqInfo.m_FocalLength = (m_EpiKeyframe_Curt.m_Camera[0].x + m_EpiKeyframe_Curt.m_Camera[1].y) / 2;

        Gfx::BufferManager::UploadBufferData(m_ParaxEq_BufferPtr, &ParaxEqInfo);

        //---Connecting Managers (@CPU) & GLSL (@GPU)---
        Gfx::ContextManager::SetShaderCS(m_Disp2Depth_CSPtr);

        Gfx::ContextManager::SetImageTexture(0, m_EpiDisparity_TexturePtr);
        Gfx::ContextManager::SetImageTexture(1, m_EpiDepth_TexturePtr);

        Gfx::ContextManager::SetConstantBuffer(0, m_ParaxEq_BufferPtr);

        //---GPU Start---
        Gfx::Performance::BeginEvent("Disparity to Depth");

        {
            const int WorkGroupsX = DivUp(m_EpiKeyframe_Curt.m_ImgSize.x, TileSize_2D);
            const int WorkGroupsY = DivUp(m_EpiKeyframe_Curt.m_ImgSize.y, TileSize_2D);

            Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

            Gfx::ContextManager::ResetShaderCS();
        }

        Gfx::Performance::EndEvent();
        //---GPU End---

        //===== 04. EpiDepth to OrigDepth =====

        imp_Depth_Epi2Orig();

        //---Connecting Managers (@CPU) & GLSL (@GPU)---
        Gfx::ContextManager::SetShaderCS(m_Depth_Rect2Orig_CSPtr);

        Gfx::ContextManager::SetImageTexture(0, m_EpiDepth_TexturePtr);
        Gfx::ContextManager::SetImageTexture(1, m_OrigDepth_TexturePtr);

        Gfx::ContextManager::SetConstantBuffer(0, m_Homography_Curt_BufferPtr);

        //---GPU Start---
        Gfx::Performance::BeginEvent("Depth from EpiImg to OrigImg");

        {
            const int WorkGroupsX = DivUp(m_OrigImgSize.x, TileSize_2D);
            const int WorkGroupsY = DivUp(m_OrigImgSize.y, TileSize_2D);

            Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

            Gfx::ContextManager::ResetShaderCS();
            Gfx::ContextManager::ResetImageTexture(1);
        }

        Gfx::Performance::EndEvent();
        // GPU End

        const auto MemSize = m_OrigDepth_TexturePtr->GetNumberOfPixelsU() * m_OrigDepth_TexturePtr->GetNumberOfPixelsV() * sizeof(uint16_t);
        m_DepthImg_Orig.resize(MemSize);
        Gfx::TextureManager::CopyTextureToCPU(m_OrigDepth_TexturePtr, reinterpret_cast<char*>(m_DepthImg_Orig.data()));

        if (m_Is_CompareDepth) // Compare Depth from plugin_stereo & Sensor
        {
            m_DepthImg_Sensor = _rDepthImage;

            cmp_Depth();
        }

        //===== 05. Output Result =====

        m_Delegate.Notify(_rRGBImage, m_DepthImg_Orig, _Transform, _FocalLength, _FocalPoint);

        if (m_IsExport_Depth)
        {
            export_Depth();
        }

    }


    // -----------------------------------------------------------------------------

    Gfx::CTexturePtr CPluginInterface::GetLatestDepthImageGPU() const
    {
        return nullptr;
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnFrameGPU(Gfx::CTexturePtr _RGBImage, const glm::mat4& _Transform)
    {
        BASE_UNUSED(_RGBImage);
        BASE_UNUSED(_Transform);
    }

    // -----------------------------------------------------------------------------
    
    void CPluginInterface::UpSampling()
    {
        /*
        //---Initialize Texture Manager for Disparity in Rectified Image---
        Gfx::STextureDescriptor TextureDesc_DispImg_Rect = {};
        TextureDesc_DispImg_Rect.m_NumberOfPixelsU = m_RectImg_Curt.get_ImgSize().x;
        TextureDesc_DispImg_Rect.m_NumberOfPixelsV = m_RectImg_Curt.get_ImgSize().y;
        TextureDesc_DispImg_Rect.m_NumberOfPixelsW = 1;
        TextureDesc_DispImg_Rect.m_NumberOfMipMaps = 1;
        TextureDesc_DispImg_Rect.m_NumberOfTextures = 1;
        TextureDesc_DispImg_Rect.m_Binding = Gfx::CTexture::ShaderResource;
        TextureDesc_DispImg_Rect.m_Access = Gfx::CTexture::EAccess::CPUWrite;
        TextureDesc_DispImg_Rect.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
        TextureDesc_DispImg_Rect.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextureDesc_DispImg_Rect.m_Format = Gfx::CTexture::R32_FLOAT; // 1 channels with 32-bit float.

        m_EpiDisparity_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc_DispImg_Rect);
        m_Disp_HR_BiLinear_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc_DispImg_Rect);

        TextureDesc_DispImg_Rect.m_Format = Gfx::CTexture::R8_UBYTE; // 1 channels with 8-bit uint.
        Gfx::CTexturePtr Temp_RectImg_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc_DispImg_Rect);

        Gfx::STextureDescriptor TextureDesc_DepthImg_Rect = TextureDesc_DispImg_Rect;
        TextureDesc_DepthImg_Rect.m_Format = Gfx::CTexture::R16_UINT; // 1 channels with 16-bit uint.
        m_EpiDepth_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc_DepthImg_Rect);

        if (m_Is_Scaling)
        {

            

            Strategy Design

            * Add Hierarchical Structure

            * Option 1. LR Depth -> Up-Sampling by BiLinear -> Color Guided FGS -> HR Depth

            * Option 2. LR Depth -> Up-Sampling by Color Guided Sparse-Input FGI -> HR Depth

            * Option 3. Fusion HR Depth from Option 1. & Option 2.
                * Still need to think about the strategy (Research Contribution !!!)
                * Consensus Check + Weighted Average / Weighted Least Square

            

            //---Up-Sampling by BiLinear Interpolation---
            {
                Gfx::Performance::BeginEvent("BiLinear Up-Sampling");

                Base::AABB2UInt TargetRect;
                TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_RectImgSize_DownSample.x, m_RectImgSize_DownSample.y));
                Gfx::TextureManager::CopyToTexture2D(m_Disp_LR_TexturePtr, TargetRect, m_RectImgSize_DownSample.x, static_cast<const void*>(m_DispImg_Rect.data()));

                Gfx::ContextManager::SetShaderCS(m_UpSampling_BiLinear_CSPtr);
                Gfx::ContextManager::SetImageTexture(0, m_Disp_LR_TexturePtr);
                Gfx::ContextManager::SetImageTexture(1, m_Disp_HR_BiLinear_TexturePtr);

                const int WorkGroupsX = DivUp(m_RectImg_Curt.get_ImgSize().x, TileSize_2D);
                const int WorkGroupsY = DivUp(m_RectImg_Curt.get_ImgSize().y, TileSize_2D);

                Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

                Gfx::ContextManager::ResetShaderCS();

                Gfx::Performance::EndEvent();
            }

            //---Color Guided FGS---

            {
                m_FGI_UpSampler = FutoGCV::CFGI(m_RectImg_Curt.get_ImgSize());

                Base::AABB2UInt TargetRect;
                TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_RectImg_Curt.get_ImgSize()));
                Gfx::TextureManager::CopyToTexture2D(Temp_RectImg_TexturePtr, TargetRect, m_RectImg_Curt.get_ImgSize().x, static_cast<const void*>(m_RectImg_Curt.get_Img().data()));

                m_FGI_UpSampler.FGS(m_EpiDisparity_TexturePtr, m_Disp_HR_BiLinear_TexturePtr, Temp_RectImg_TexturePtr);
            }

        }
        else
        {
            Base::AABB2UInt TargetRect;
            TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_RectImg_Curt.get_ImgSize()));
            Gfx::TextureManager::CopyToTexture2D(m_EpiDisparity_TexturePtr, TargetRect, m_RectImg_Curt.get_ImgSize().x, static_cast<const void*>(m_DispImg_Rect.data()));
        }
        */
    }

    void CPluginInterface::imp_Disp2Depth()
    {

    }

    void CPluginInterface::imp_Depth_Epi2Orig()
    {

    }

    void CPluginInterface::cmp_Depth()
    {
        Base::AABB2UInt TargetRect;
        TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_OrigImgSize.x, m_OrigImgSize.y));
        Gfx::TextureManager::CopyToTexture2D(m_OrigDepth_Sensor_TexturePtr, TargetRect, m_OrigImgSize.x, static_cast<const void*>(m_DepthImg_Sensor.data()));

        Gfx::Performance::BeginEvent("Compare Depth");

        Gfx::ContextManager::SetShaderCS(m_Compare_Depth_CSPtr);
        Gfx::ContextManager::SetImageTexture(0, m_OrigDepth_TexturePtr);
        Gfx::ContextManager::SetImageTexture(1, m_OrigDepth_Sensor_TexturePtr);
        Gfx::ContextManager::SetImageTexture(2, m_OrigDepth_Diff_TexturePtr);

        const int WorkGroupsX = DivUp(m_OrigImgSize.x, TileSize_2D);
        const int WorkGroupsY = DivUp(m_OrigImgSize.y, TileSize_2D);

        Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        Gfx::ContextManager::ResetShaderCS();
        Gfx::ContextManager::ResetImageTexture(1);

        Gfx::Performance::EndEvent();
    }


    // -----------------------------------------------------------------------------

    void CPluginInterface::export_OrigImg()
    {
        cv::Mat cvOrigImg_Curt(m_OrigImgSize.y, m_OrigImgSize.x, CV_8UC4);
        std::vector<char> vecOrigImg_Curt(m_OrigImgSize.x * m_OrigImgSize.y, 0);

        Gfx::TextureManager::CopyTextureToCPU(m_OrigKeyframe_Curt.m_Img_TexturePtr, reinterpret_cast<char*>(vecOrigImg_Curt.data()));

        memcpy(cvOrigImg_Curt.data, vecOrigImg_Curt.data(), vecOrigImg_Curt.size() * sizeof(vecOrigImg_Curt[0]));

        cv::cvtColor(cvOrigImg_Curt, cvOrigImg_Curt, cv::COLOR_BGRA2RGBA); // Transform to RGB before imshow & imwrite

        std::string ExportStr = "E:\\Project_ARCHITECT\\OrigImg_Curt_" + std::to_string(m_KeyfID) + ".png";
        cv::imwrite(ExportStr, cvOrigImg_Curt);
    }

    void CPluginInterface::export_RectImg()
    {
        
    }

    void CPluginInterface::export_Depth()
    {
        std::string ExportStr;

        int MemCpySize = 0;

        cv::Mat cvOrigDepth_SM(m_EpiKeyframe_Curt.m_ImgSize.y, m_EpiKeyframe_Curt.m_ImgSize.x, CV_16UC1);

        MemCpySize = static_cast<int>(m_DepthImg_Orig.size() * sizeof(m_DepthImg_Orig[0]));
        memcpy(cvOrigDepth_SM.data, m_DepthImg_Orig.data(), MemCpySize);

        ExportStr = "E:\\Project_ARCHITECT\\DepthImg_Orig_" + std::to_string(m_KeyfID) + ".png";
        cv::imwrite(ExportStr, cvOrigDepth_SM);


        cv::Mat cvOrigDepth_Sensor(cvOrigDepth_SM.size(), CV_16UC1);

        MemCpySize = static_cast<int>(m_DepthImg_Sensor.size() * sizeof(m_DepthImg_Sensor[0]));
        memcpy(cvOrigDepth_Sensor.data, m_DepthImg_Sensor.data(), MemCpySize);

        ExportStr = "E:\\Project_ARCHITECT\\DepthImg_Sensor_" + std::to_string(m_KeyfID) + ".png";
        cv::imwrite(ExportStr, cvOrigDepth_Sensor);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnStart()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin started!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        //===== 00. Input Data =====

        m_OrigImg_TexturePtr = nullptr;

        //===== 00. Select Keyframe =====

        //===== 01. Epipolarization =====

        m_Homography_Curt_BufferPtr = nullptr;
        m_Homography_Last_BufferPtr = nullptr;


        // *** OLD ***
        //---Release Manager---
        m_Disp2Depth_CSPtr = nullptr;
        m_EpiDisparity_TexturePtr = nullptr;
        m_EpiDepth_TexturePtr = nullptr;
        m_ParaxEq_BufferPtr = nullptr;

        m_Depth_Rect2Orig_CSPtr = nullptr;
        m_OrigDepth_TexturePtr = nullptr;

        m_Compare_Depth_CSPtr = nullptr;
        m_OrigDepth_Sensor_TexturePtr = nullptr;
        m_OrigDepth_Diff_TexturePtr = nullptr;

        //---plugin_stereo Finish !!!---
        ENGINE_CONSOLE_INFOV("Stereo matching plugin exited!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnPause()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin paused!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin resumed!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::EventHook()
    {
        ENGINE_CONSOLE_INFOV("Oh hello. An event has been raised!");
    }

    // -----------------------------------------------------------------------------

    CPluginInterface::CStereoDelegate::HandleType CPluginInterface::Register(Stereo::CPluginInterface::CStereoDelegate::FunctionType _Function)
    {
        return m_Delegate.Register(_Function);
    }

} // namespace Stereo

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4& _Transform, const glm::vec2& _FocalLength, const glm::vec2& _FocalPoint, const std::vector<uint16_t>& _rDepthImage)
{
    static_cast<Stereo::CPluginInterface&>(GetInstance()).OnFrameCPU(_rRGBImage, _Transform, _FocalLength, _FocalPoint, _rDepthImage);
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT bool GetLatestFrameCPU(std::vector<char>& _ColorImage, std::vector<char>& _rDepthImage, glm::mat4& _rTransform)
{
    return static_cast<Stereo::CPluginInterface&>(GetInstance()).GetLatestFrameCPU(_ColorImage, _rDepthImage, _rTransform);
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void OnFrameGPU(Gfx::CTexturePtr _RGBImage, const glm::mat4& _Transform)
{
    static_cast<Stereo::CPluginInterface&>(GetInstance()).OnFrameGPU(_RGBImage, _Transform);
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void GetLatestDepthImageGPU(Gfx::CTexturePtr& _rDepthImage)
{
    _rDepthImage = static_cast<Stereo::CPluginInterface&>(GetInstance()).GetLatestDepthImageGPU();
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void SetIntrinsics(const glm::vec2& _rFocalLength, const glm::vec2& _rFocalPoint, const glm::ivec2& _rImageSize)
{
    static_cast<Stereo::CPluginInterface&>(GetInstance()).SetIntrinsics(_rFocalLength, _rFocalPoint, _rImageSize);
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT Stereo::CPluginInterface::CStereoDelegate::HandleType Register(Stereo::CPluginInterface::CStereoDelegate::FunctionType _Function)
{
    return static_cast<Stereo::CPluginInterface&>(GetInstance()).Register(_Function);
}