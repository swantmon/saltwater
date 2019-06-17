
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

    #define TileSize_2D 16 // Define size of work group for GPU parallel processing.

    int DivUp(int TotalShaderCount, int WorkGroupSize) // Calculate number of work groups.
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }

    struct SParallaxEquation // Info of Parallax Equation for GLSL.
    {
        float m_BaselineLength; // Unit = meter
        float m_FocalLength; // Unit = pixel
        glm::ivec2 m_Padding;
    };

    enum EStereoMatchingMethod
    {
        LibSGM,
        cvBM_cuda
    };
}

namespace Stereo
{
    void CPluginInterface::SetIntrinsics(const glm::vec2& _rFocalLength, const glm::vec2& _rFocalPoint, const glm::ivec2& _rImageSize)
    {
        BASE_UNUSED(_rFocalLength); // Avoid warning about unused variable.
        BASE_UNUSED(_rFocalPoint); // Avoid warning about unused variable.

        m_OrigImgSize = _rImageSize;

        //---Setting Texture of Depth in Original Image for GLSL---
        Gfx::STextureDescriptor TextureDesc_DepthImg_Orig = {};

        TextureDesc_DepthImg_Orig.m_NumberOfPixelsU = m_OrigImgSize.x;
        TextureDesc_DepthImg_Orig.m_NumberOfPixelsV = m_OrigImgSize.y;
        TextureDesc_DepthImg_Orig.m_NumberOfPixelsW = 1;
        TextureDesc_DepthImg_Orig.m_NumberOfMipMaps = 1;
        TextureDesc_DepthImg_Orig.m_NumberOfTextures = 1;
        TextureDesc_DepthImg_Orig.m_Binding = Gfx::CTexture::ShaderResource;
        TextureDesc_DepthImg_Orig.m_Access = Gfx::CTexture::EAccess::CPUWrite;
        TextureDesc_DepthImg_Orig.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
        TextureDesc_DepthImg_Orig.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextureDesc_DepthImg_Orig.m_Format = Gfx::CTexture::R16_UINT; // 1 channels with 16-bit uint.
        TextureDesc_DepthImg_Orig.m_pFileName = nullptr;
        TextureDesc_DepthImg_Orig.m_pPixels = nullptr;

        m_DepthImg_Orig_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc_DepthImg_Orig);

        m_DepthImg_Sensor_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc_DepthImg_Orig);

        //---Setting Texture of Depth Difference for GLSL---
        Gfx::STextureDescriptor TextureDesc_DepthImg_Diff = TextureDesc_DepthImg_Orig;
        TextureDesc_DepthImg_Diff.m_Format = Gfx::CTexture::R16_INT; // 1 channels with 16-bit int.
        m_Depth_Difference_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc_DepthImg_Diff);
    }

    // -----------------------------------------------------------------------------

    bool CPluginInterface::GetLatestFrameCPU(std::vector<char>& _ColorImage, std::vector<char>& _rDepthImage, glm::mat4& _rTransform)
    {
        return false;
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4& _Transform, const glm::mat4& _Intrinsics, const std::vector<uint16_t>& _rDepthImage)
    {
        glm::mat3 CamMtx = glm::mat3(_Intrinsics) * m_FrameResolution; // Intrinsic should be modified according to frame resolution.
        CamMtx[1].x = 0; // No skew
        CamMtx[2].z = 1; // Last element should keep 1 because of homogeneous coordinates

        glm::mat3 RotMtx = glm::transpose(glm::mat3(_Transform));// ARKit's Rotation is Camera2World (Computer Graphics), but we need Rotation as World2Camera (Computer Vision).

        glm::vec3 PCVec = glm::vec3(_Transform[3]); // The last column of _Transform given by ARKit is the Position of Camera in World.
        
        //---Only Compute Keyframes---
        if (!m_is_Keyf_Curt)
        {
            m_OrigImg_Curt = FutoGmtCV::CFutoImg(_rRGBImage, m_OrigImgSize, 4, CamMtx, RotMtx, PCVec);
            m_is_Keyf_Curt = true;
        }
        else
        {
            glm::vec3 BaseLine = PCVec - m_OrigImg_Curt.get_PC();
            float BaseLineLength = glm::l2Norm(BaseLine);

            if (BaseLineLength < m_Cdt_Keyf_BaseLineL) // Keyframe Selection: Baseline Length
            {
                return;
            }

            //---Set Current & Last Keyframe---
            m_OrigImg_Last = m_OrigImg_Curt;
            m_OrigImg_Curt = FutoGmtCV::CFutoImg(_rRGBImage, m_OrigImgSize, 4, CamMtx, RotMtx, PCVec);

            m_KeyfID++;

            //---Epipolarizytion---
            m_Rectifier_Planar = FutoGmtCV::CPlanarRectification(m_OrigImg_Curt, m_OrigImg_Last); // Apply Planar Rectification
            m_Rectifier_Planar.execute(m_RectImg_Curt, m_RectImg_Last, m_Homo_Curt, m_Homo_Last);

            if (m_RectImg_Curt.get_ImgSize().x > 2500 || m_RectImg_Curt.get_ImgSize().y > 2500)
            {
                return; // LibSGM will break if the size of rectified image is too large.
            }

            cv::Mat cvRectImg_Curt(m_RectImg_Curt.get_ImgSize().y, m_RectImg_Curt.get_ImgSize().x, CV_8UC1); // For Stereo Matching in OpenCV
            memcpy(cvRectImg_Curt.data, m_RectImg_Curt.get_Img().data(), m_RectImg_Curt.get_Img().size());

            cv::Mat cvRectImg_Last(m_RectImg_Last.get_ImgSize().y, m_RectImg_Last.get_ImgSize().x, CV_8UC1); // For Stereo Matching in OpenCV
            memcpy(cvRectImg_Last.data, m_RectImg_Last.get_Img().data(), m_RectImg_Last.get_Img().size());

            //---Stereo Matching---
            m_DispImg_Rect.resize(m_RectImg_Curt.get_Img().size(), 0.0);

            cv::cuda::GpuMat cvRectImg_Curt_gpu, cvRectImg_Last_gpu; // For Stereo Matching in OpenCV+Cuda
            cvRectImg_Curt_gpu.upload(cvRectImg_Curt);
            cvRectImg_Last_gpu.upload(cvRectImg_Last);

            cv::cuda::GpuMat cvDispImg_Rect_gpu; // For Stereo Matching in OpenCV+Cuda
            cv::Mat cvDispImg_Rect_cpu; // For Stereo Matching in OpenCV




            //---Export Setting---
            std::string ExportStr;
            //----------

            if (m_IsExport_OrigImg) // Export current keyframe
            {
                cv::Mat cvOrigImg_Curt(m_OrigImgSize.y, m_OrigImgSize.x, CV_8UC4);
                memcpy(cvOrigImg_Curt.data, m_OrigImg_Curt.get_Img().data(), m_OrigImg_Curt.get_Img().size());
                cv::cvtColor(cvOrigImg_Curt, cvOrigImg_Curt, cv::COLOR_BGRA2RGBA); // Transform to RGB before imshow & imwrite

                ExportStr = "E:\\Project_ARCHITECT\\ARKit_OrigImg_Curt_" + std::to_string(m_KeyfID) + ".png";
                cv::imwrite(ExportStr, cvOrigImg_Curt);
                ExportStr = "";
            }



            //***Export Rectified Images***
            if (m_IsExport_OrigImg)
            {
                ExportStr = "E:\\Project_ARCHITECT\\ARKit_RectImg_Curt_" + std::to_string(m_KeyfID) + ".png";
                cv::imwrite(ExportStr, cvRectImg_Curt);
                ExportStr = "";

                ExportStr = "E:\\Project_ARCHITECT\\ARKit_RectImg_Last_" + std::to_string(m_KeyfID) + ".png";
                cv::imwrite(ExportStr, cvRectImg_Last);
                ExportStr = "";
            }


            //---Stereo Matching: Generate Disparity in Rectified Current Keyframe---


            if (m_StereoMatching_Method == "LibSGM")
            {
                m_pStereoMatcher_LibSGM = std::make_unique<sgm::StereoSGM>(m_RectImg_Curt.get_ImgSize().x, m_RectImg_Curt.get_ImgSize().y, m_DispRange, 8, 16, sgm::EXECUTE_INOUT_HOST2HOST);
                // Default disparity is pixel level => Disparity is the same in 8-bit & 16-bit.
                // If turn on sub-pixel => Output disparity must be 16-bit. => Divided by 16 to derive true disparity!!!

                std::vector<uint16_t> Disp_RectImg_Temp(m_RectImg_Curt.get_Img().size(), 0.0);

                m_pStereoMatcher_LibSGM->execute(m_RectImg_Curt.get_Img().data(), m_RectImg_Last.get_Img().data(), Disp_RectImg_Temp.data());

                m_DispImg_Rect = std::vector<float>::vector(Disp_RectImg_Temp.begin(), Disp_RectImg_Temp.end());

                //***Export Disparity in Rectified Images (in 16-bit)***
                if (m_IsExport_OrigImg)
                {
                    cv::Mat cvDispImg_LibSGM(cvRectImg_Curt.size(), CV_16UC1);
                    memcpy(cvDispImg_LibSGM.data, Disp_RectImg_Temp.data(), Disp_RectImg_Temp.size() * sizeof(Disp_RectImg_Temp[0]));
                    cv::imwrite("E:\\Project_ARCHITECT\\ARKit_DispImg_LibSGM.png", cvDispImg_LibSGM);
                }
            }

            if (m_StereoMatching_Method == "cvBM_cuda")
            {
                m_pStereoMatcher_cvBM_cuda = cv::cuda::createStereoBM(m_DispRange, 7); // Disparity Range, Block Size
                m_pStereoMatcher_cvBM_cuda->compute(cvRectImg_Curt_gpu, cvRectImg_Last_gpu, cvDispImg_Rect_gpu);

                cvDispImg_Rect_gpu.download(cvDispImg_Rect_cpu);

                cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F); // The Disparity should be 32F.

                //***Export Disparity in Rectified Images (in 16-bit)***
                if (m_IsExport_OrigImg)
                {
                    cv::Mat cvDisp_RectImg_cpu_16UC1(cvDispImg_Rect_cpu.size(), CV_16UC1);
                    cv::normalize(cvDispImg_Rect_cpu, cvDisp_RectImg_cpu_16UC1, 0, 65535, cv::NORM_MINMAX, CV_16UC1);

                    cv::imwrite("E:\\Project_ARCHITECT\\ARKit_DispImg_cvBM_cuda.png", cvDisp_RectImg_cpu_16UC1);
                }

                const int cvMemCpySize = cvDispImg_Rect_cpu.cols * cvDispImg_Rect_cpu.rows * cvDispImg_Rect_cpu.elemSize();
                memcpy(m_DispImg_Rect.data(), cvDispImg_Rect_cpu.data, cvMemCpySize);
            }

            if (m_StereoMatching_Method == "cvBP_cuda")
            {
                m_pStereoMatcher_cvBP_cuda = cv::cuda::createStereoBeliefPropagation();
                m_pStereoMatcher_cvBP_cuda->compute(cvRectImg_Curt_gpu, cvRectImg_Last_gpu, cvDispImg_Rect_gpu);

                cvDispImg_Rect_gpu.download(cvDispImg_Rect_cpu);

                cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F); // Disparity is 16S but without fractional bit.

                //***Export Disparity in Rectified Images (in 16-bit)***
                if (m_IsExport_OrigImg)
                {
                    cv::Mat cvDisp_RectImg_cpu_16UC1(cvDispImg_Rect_cpu.size(), CV_16UC1);
                    cv::normalize(cvDispImg_Rect_cpu, cvDisp_RectImg_cpu_16UC1, 0, 65535, cv::NORM_MINMAX, CV_16UC1);

                    cv::imwrite("E:\\Project_ARCHITECT\\ARKit_DispImg_cvBP_cuda.png", cvDisp_RectImg_cpu_16UC1);
                }

                const int cvMemCpySize = cvDispImg_Rect_cpu.cols * cvDispImg_Rect_cpu.rows * cvDispImg_Rect_cpu.elemSize();
                memcpy(m_DispImg_Rect.data(), cvDispImg_Rect_cpu.data, cvMemCpySize);
            }

            if (m_StereoMatching_Method == "cvConstBP_cuda")
            {
                m_pStereoMatcher_cvConstBP_cuda = cv::cuda::createStereoConstantSpaceBP();
                m_pStereoMatcher_cvConstBP_cuda->compute(cvRectImg_Curt_gpu, cvRectImg_Last_gpu, cvDispImg_Rect_gpu);

                cvDispImg_Rect_gpu.download(cvDispImg_Rect_cpu);

                cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F); // Disparity is 16S but without fractional bit.

                //***Export Disparity in Rectified Images (in 16-bit)***
                if (m_IsExport_OrigImg)
                {
                    cv::Mat cvDisp_RectImg_cpu_16UC1(cvDispImg_Rect_cpu.size(), CV_16UC1);
                    cv::normalize(cvDispImg_Rect_cpu, cvDisp_RectImg_cpu_16UC1, 0, 65535, cv::NORM_MINMAX, CV_16UC1);

                    cv::imwrite("E:\\Project_ARCHITECT\\ARKit_DispImg_cvConstBP_cuda.png", cvDisp_RectImg_cpu_16UC1);
                }

                const int cvMemCpySize = cvDispImg_Rect_cpu.cols * cvDispImg_Rect_cpu.rows * cvDispImg_Rect_cpu.elemSize();
                memcpy(m_DispImg_Rect.data(), cvDispImg_Rect_cpu.data, cvMemCpySize);
            }

            if (m_StereoMatching_Method == "cv_SGBM")
            {
                m_pStereoMatcher_cvSGBM = cv::StereoSGBM::create();
                m_pStereoMatcher_cvSGBM->compute(cvRectImg_Curt, cvRectImg_Last, cvDispImg_Rect_cpu);

                if (cvDispImg_Rect_cpu.type() == CV_16S)
                {
                    cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F, 1.0 / 16); // Disparity is 16S with 4 fractional bits.
                }

                if (m_IsExport_OrigImg)
                {
                    cv::Mat cvDispImg_Rect_cpu_16UC1(cvDispImg_Rect_cpu.size(), CV_16UC1);
                    cv::normalize(cvDispImg_Rect_cpu, cvDispImg_Rect_cpu_16UC1, 0, 65535, cv::NORM_MINMAX, CV_16UC1);
                    cv::imwrite("E:\\Project_ARCHITECT\\ARKit_DispImg_cvSGBM.png", cvDispImg_Rect_cpu_16UC1);
                }

                const int cvMemCpySize = cvDispImg_Rect_cpu.cols * cvDispImg_Rect_cpu.rows * cvDispImg_Rect_cpu.elemSize();
                memcpy(m_DispImg_Rect.data(), cvDispImg_Rect_cpu.data, cvMemCpySize);
            }

            //---Disparity to Depth in Rectified Current Keyframe---
            imp_Disp2Depth();

            //---Depth from Rectified to Original Current Keyframe---
            imp_Depth_Rect2Orig();

            //---Compare Depth from Stereo Matching & Sensor---
            Base::AABB2UInt TargetRect;
            TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_OrigImgSize.x, m_OrigImgSize.y));
            Gfx::TextureManager::CopyToTexture2D(m_DepthImg_Sensor_TexturePtr, TargetRect, m_OrigImgSize.x, static_cast<const void*>(_rDepthImage.data()));

            cmp_Depth();

            //***** Export Depth in OrigImg (from Stereo Matching & Sensor) with 16-bit *****
            if (m_IsExport_OrigImg)
            {
                cv::Mat cvDepth_OrigImg(m_OrigImg_Curt.get_ImgSize().y, m_OrigImg_Curt.get_ImgSize().x, CV_16UC1);
                memcpy(cvDepth_OrigImg.data, m_Depth_OrigImg.data(), m_Depth_OrigImg.size() * sizeof(m_Depth_OrigImg[0]));
                ExportStr = "E:\\Project_ARCHITECT\\ARKit_DepthImg_OrigImg_" + std::to_string(m_KeyfID) + ".png";
                cv::imwrite(ExportStr, cvDepth_OrigImg);
                ExportStr = "";

                cv::Mat cvDepth_Sensor(m_OrigImg_Curt.get_ImgSize().y, m_OrigImg_Curt.get_ImgSize().x, CV_16UC1);
                memcpy(cvDepth_Sensor.data, _rDepthImage.data(), _rDepthImage.size() * sizeof(_rDepthImage[0]));
                ExportStr = "E:\\Project_ARCHITECT\\ARKit_DepthImg_Sensor_" + std::to_string(m_KeyfID) + ".png";
                cv::imwrite(ExportStr, cvDepth_Sensor);
                ExportStr = "";
            }


            //---Return Depth in Original Image---
            const int MemCpySize = m_DepthImg_Orig_TexturePtr->GetNumberOfPixelsU() * m_DepthImg_Orig_TexturePtr->GetNumberOfPixelsV() * sizeof(uint16_t);
            std::vector<char> DepthImage(MemCpySize);
            Gfx::TextureManager::CopyTextureToCPU(m_DepthImg_Orig_TexturePtr, DepthImage.data());
            glm::mat4 Transform = glm::mat4(glm::transpose(m_OrigImg_Curt.get_Rot()));
            Transform[3] = glm::vec4(m_OrigImg_Curt.get_PC(), 1.0f);

            m_Delegate.Notify(m_OrigImg_Curt.get_Img(), DepthImage, Transform);

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
    void CPluginInterface::imp_Disp2Depth()
    {
        //---Initialize Texture Manager for Disparity in Rectified Image---
        Gfx::STextureDescriptor TextureDescriptor_RectImg = {};

        TextureDescriptor_RectImg.m_NumberOfPixelsU = m_RectImg_Curt.get_ImgSize().x;
        TextureDescriptor_RectImg.m_NumberOfPixelsV = m_RectImg_Last.get_ImgSize().y;
        TextureDescriptor_RectImg.m_NumberOfPixelsW = 1;
        TextureDescriptor_RectImg.m_NumberOfMipMaps = 1;
        TextureDescriptor_RectImg.m_NumberOfTextures = 1;
        TextureDescriptor_RectImg.m_Binding = Gfx::CTexture::ShaderResource;
        TextureDescriptor_RectImg.m_Access = Gfx::CTexture::EAccess::CPUWrite;
        TextureDescriptor_RectImg.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
        TextureDescriptor_RectImg.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextureDescriptor_RectImg.m_Format = Gfx::CTexture::R32_FLOAT; // 1 channels with 32-float.

        m_Disp_RectImg_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_RectImg);

        m_Depth_RectImg_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_RectImg);

        //---GPU Start---
        Gfx::Performance::BeginEvent("Disparity to Depth");

        // Submit Data to Managers
        SParallaxEquation ParaxEq;
        ParaxEq.m_BaselineLength = glm::l2Norm(m_RectImg_Curt.get_PC() - m_RectImg_Last.get_PC());
        ParaxEq.m_FocalLength = m_RectImg_Curt.get_Cam()[0].x;

        Gfx::BufferManager::UploadBufferData(m_ParaxEq_BufferPtr, &ParaxEq);

        Base::AABB2UInt TargetRect;
        TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_RectImg_Curt.get_ImgSize().x, m_RectImg_Curt.get_ImgSize().y));
        Gfx::TextureManager::CopyToTexture2D(m_Disp_RectImg_TexturePtr, TargetRect, m_RectImg_Curt.get_ImgSize().x, static_cast<const void*>(m_DispImg_Rect.data()));

        // Connecting Managers (@CPU) & GLSL (@GPU)
        Gfx::ContextManager::SetShaderCS(m_Disp2Depth_CSPtr);
        Gfx::ContextManager::SetImageTexture(0, m_Disp_RectImg_TexturePtr);
        Gfx::ContextManager::SetImageTexture(1, m_Depth_RectImg_TexturePtr);
        Gfx::ContextManager::SetConstantBuffer(0, m_ParaxEq_BufferPtr);

        // Start GPU Parallel Processing
        const int WorkGroupsX = DivUp(m_RectImg_Curt.get_ImgSize().x, TileSize_2D);
        const int WorkGroupsY = DivUp(m_RectImg_Curt.get_ImgSize().y, TileSize_2D);

        Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        Gfx::ContextManager::ResetShaderCS();

        Gfx::Performance::EndEvent();
        // GPU End
    }

    void CPluginInterface::imp_Depth_Rect2Orig()
    {
        //---GPU Start---
        Gfx::Performance::BeginEvent("Depth from Rectified to Original");

        // Submit Data to Managers
        Gfx::BufferManager::UploadBufferData(m_Homogrampy_BufferPtr, &m_Homo_Curt);

        // Connecting Managers (@CPU) & GLSL (@GPU)
        Gfx::ContextManager::SetShaderCS(m_Depth_Rect2Orig_CSPtr);
        Gfx::ContextManager::SetImageTexture(0, m_Depth_RectImg_TexturePtr);
        Gfx::ContextManager::SetImageTexture(1, m_DepthImg_Orig_TexturePtr);
        Gfx::ContextManager::SetConstantBuffer(0, m_Homogrampy_BufferPtr);

        // Start GPU Parallel Processing
        const int WorkGroupsX = DivUp(m_OrigImgSize.x, TileSize_2D);
        const int WorkGroupsY = DivUp(m_OrigImgSize.y, TileSize_2D);

        Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        Gfx::ContextManager::ResetShaderCS();
        Gfx::ContextManager::ResetImageTexture(1);

        Gfx::Performance::EndEvent();
        // GPU End
        m_Depth_OrigImg.resize(m_DepthImg_Orig_TexturePtr->GetNumberOfPixelsU() * m_DepthImg_Orig_TexturePtr->GetNumberOfPixelsV() * sizeof(uint16_t));
        Gfx::TextureManager::CopyTextureToCPU(m_DepthImg_Orig_TexturePtr, m_Depth_OrigImg.data());
        glm::mat4 Transform = glm::mat4(glm::transpose(m_OrigImg_Curt.get_Rot()));
        Transform[3] = glm::vec4(m_OrigImg_Curt.get_PC(), 1.0f);

        m_Delegate.Notify(m_OrigImg_Curt.get_Img(), m_Depth_OrigImg, Transform);
    }

    void CPluginInterface::cmp_Depth()
    {
        //---GPU Start---
        Gfx::Performance::BeginEvent("Compare Depth");

        // Connecting Managers (@CPU) & GLSL (@GPU)
        Gfx::ContextManager::SetShaderCS(m_Compare_Depth_CSPtr);
        Gfx::ContextManager::SetImageTexture(0, m_DepthImg_Orig_TexturePtr);
        Gfx::ContextManager::SetImageTexture(1, m_DepthImg_Sensor_TexturePtr);
        Gfx::ContextManager::SetImageTexture(2, m_Depth_Difference_TexturePtr);

        // Start GPU Parallel Processing
        const int WorkGroupsX = DivUp(m_OrigImgSize.x, TileSize_2D);
        const int WorkGroupsY = DivUp(m_OrigImgSize.y, TileSize_2D);

        Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        Gfx::ContextManager::ResetShaderCS();
        Gfx::ContextManager::ResetImageTexture(1);

        Gfx::Performance::EndEvent();
        // GPU End
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::export_OrigImg()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnStart()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin started!");

        //---Export Result---
        m_IsExport_OrigImg = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_export_result:00_orig_img", true);

        //---Input Setting---
        m_FrameResolution = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_pre-setting:frame_resolution", 0.5); // Full = 1; Half = 0.5;

        //---Keyframe Setting---
        m_Cdt_Keyf_BaseLineL = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_pre-setting:baseline_length", 0.03);

        m_is_Keyf_Curt = false;
        m_is_Keyf_Last = false;

        //---Rectification-----

        //---Stereo Matching---
        m_DispRange = Core::CProgramParameters::GetInstance().Get("mr:stereo:02_stereo_matching:disparity_range", 128);

        m_StereoMatching_Method = Core::CProgramParameters::GetInstance().Get("mr:stereo:02_stereo_matching:Method", "LibSGM");

        //---Disparity to Depth---
        std::stringstream DefineStream;
        DefineStream
            << "#define TILE_SIZE_2D " << TileSize_2D << " \n"; // 16 for work group size is suggested for 2D image (based on experience).
        std::string DefineString = DefineStream.str();
        m_Disp2Depth_CSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/cs_Disparity_to_Depth.glsl", "main", DefineString.c_str());

        Gfx::SBufferDescriptor ParaxEq_BufferDesc = {};
        ParaxEq_BufferDesc.m_Stride = 0;
        ParaxEq_BufferDesc.m_Usage = Gfx::CBuffer::GPURead;
        ParaxEq_BufferDesc.m_Binding = Gfx::CBuffer::ConstantBuffer;
        ParaxEq_BufferDesc.m_Access = Gfx::CBuffer::CPUWrite;
        ParaxEq_BufferDesc.m_NumberOfBytes = sizeof(SParallaxEquation);
        ParaxEq_BufferDesc.m_pBytes = nullptr;
        ParaxEq_BufferDesc.m_pClassKey = 0;
        m_ParaxEq_BufferPtr = Gfx::BufferManager::CreateBuffer(ParaxEq_BufferDesc);

        //---Depth from Rectified to Original---
        m_Depth_Rect2Orig_CSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/cs_Depth_Rect2Orig.glsl", "main", DefineString.c_str());

        Gfx::SBufferDescriptor Homography_BufferDesc = {};
        Homography_BufferDesc.m_Stride = 0;
        Homography_BufferDesc.m_Usage = Gfx::CBuffer::GPURead;
        Homography_BufferDesc.m_Binding = Gfx::CBuffer::ConstantBuffer;
        Homography_BufferDesc.m_Access = Gfx::CBuffer::CPUWrite;
        Homography_BufferDesc.m_NumberOfBytes = sizeof(FutoGmtCV::SHomographyTransform);
        Homography_BufferDesc.m_pBytes = nullptr;
        Homography_BufferDesc.m_pClassKey = 0;
        m_Homogrampy_BufferPtr = Gfx::BufferManager::CreateBuffer(Homography_BufferDesc);

        //---Compare Depth---
        m_Compare_Depth_CSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/cs_Compare_Depth.glsl", "main", DefineString.c_str());
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        //---Release Manager---
        m_Disp2Depth_CSPtr = nullptr;
        m_Disp_RectImg_TexturePtr = nullptr;
        m_Depth_RectImg_TexturePtr = nullptr;
        m_ParaxEq_BufferPtr = nullptr;

        m_Depth_Rect2Orig_CSPtr = nullptr;
        m_DepthImg_Orig_TexturePtr = nullptr;
        m_Homogrampy_BufferPtr = nullptr;

        m_Compare_Depth_CSPtr = nullptr;
        m_DepthImg_Sensor_TexturePtr = nullptr;
        m_Depth_Difference_TexturePtr = nullptr;

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

extern "C" CORE_PLUGIN_API_EXPORT void OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4& _Transform, const glm::mat4& _Intrinsics, const std::vector<uint16_t>& _rDepthImage)
{
    static_cast<Stereo::CPluginInterface&>(GetInstance()).OnFrameCPU(_rRGBImage, _Transform, _Intrinsics, _rDepthImage);
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