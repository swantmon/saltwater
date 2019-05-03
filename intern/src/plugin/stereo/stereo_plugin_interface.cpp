
#include "plugin/stereo/stereo_precompiled.h"

#include "engine/core/core_console.h"
#include "engine/core/core_program_parameters.h" // For controlling parameters in Config.

#include "engine/engine.h"

#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_performance.h"
#include "engine/graphic/gfx_texture.h" // For Transmit Texture between CPU & GPU

#include "plugin/stereo/stereo_plugin_interface.h"



CORE_PLUGIN_INFO(Stereo::CPluginInterface, "Stereo Matching", "1.0", "This plugin takes RGB and transformation data and provides 2.5D depth maps")

namespace
{
    //---Definition for GPU Parallel Processing---
    #define TileSize_2D 16

    int DivUp(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }

    struct SParallaxEquation
    {
        float m_BaselineLength;
        float m_FocalLength;
        glm::ivec2 m_Padding;
    };
}

namespace Stereo
{
    void CPluginInterface::SetIntrinsics(const glm::vec2& _rFocalLength, const glm::vec2& _rFocalPoint, const glm::ivec2& _rImageSize)
    {
        //---For variables which has not been used yet. Use this statement to avoid warning.---
        BASE_UNUSED(_rFocalLength); 
        BASE_UNUSED(_rFocalPoint); 
        //------

        m_OrigImgSize = _rImageSize;

        // Initialize Texture Manager for Depth_Orig
        Gfx::STextureDescriptor TextureDescriptor_Depth = {};

        TextureDescriptor_Depth.m_NumberOfPixelsU = m_OrigImgSize.x;
        TextureDescriptor_Depth.m_NumberOfPixelsV = m_OrigImgSize.y;
        TextureDescriptor_Depth.m_NumberOfPixelsW = 1;
        TextureDescriptor_Depth.m_NumberOfMipMaps = 1;
        TextureDescriptor_Depth.m_NumberOfTextures = 1;
        TextureDescriptor_Depth.m_Binding = Gfx::CTexture::ShaderResource;
        TextureDescriptor_Depth.m_Access = Gfx::CTexture::EAccess::CPURead;
        TextureDescriptor_Depth.m_Usage = Gfx::CTexture::EUsage::GPUToCPU;
        TextureDescriptor_Depth.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextureDescriptor_Depth.m_Format = Gfx::CTexture::R16_UBYTE; // 1 channels and each channel is 16-Ubit.

        m_Depth_Orig_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_Depth);
    }

    // -----------------------------------------------------------------------------

    std::vector<char> CPluginInterface::GetLatestDepthImageCPU() const
    {
        return std::vector<char>();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4& _Transform, const glm::mat4& _Intrinsics, const std::vector<uint16_t>& _rDepthImage)
    {
        glm::mat3 Cam_mtx = glm::mat3(_Intrinsics) * m_FrameResolution; // Intrinsic should be modified according to frame resolution.
        Cam_mtx[2].z = 1;

        glm::mat3 Rot_mtx = glm::transpose(glm::mat3(_Transform));// Rotation given by ARKit is Camera2World, but Rotation in Photogrammetry needs World2Camera.

        glm::vec3 PC_vec = glm::vec3(_Transform[3]); // The last column of _Transform given by ARKit is the Position of Camera in World.

        FutoGmtCV::FutoImg frame(_rRGBImage, m_OrigImgSize, Cam_mtx, Rot_mtx, PC_vec);
        
        //---Select Keyframe for Computation---
        if (!m_idx_Keyf_Curt) // Current keyframe is empty -> Set current keyframe.
        {
            m_Keyframe_Curt = frame;
            m_idx_Keyf_Curt = true;
        }
        else if (m_idx_Keyf_Curt && !m_idx_Keyf_Last) // Current keyframe exists but Last keyframe is empty -> Set both current & last keyframes.
        {
            glm::vec3 BaseLine = frame.get_PC() - m_Keyframe_Curt.get_PC();
            float BaseLineLength = glm::l2Norm(BaseLine);

            if (BaseLineLength >= m_Cdt_Keyf_BaseLineL) // Select Keyframe: Baseline condition
            {
                m_Keyframe_Last = m_Keyframe_Curt;
                m_Keyframe_Curt = frame;
                m_idx_Keyf_Last = true;
            }
        }
        else // Both current & last keyframes exist. -> Processing -> Free last keyframe.
        {
            if (m_Is_ARKitData)
            {
                
                if (m_Is_imwrite)
                {
                    cv::Mat cvOrigImg_Curt(m_OrigImgSize.y, m_OrigImgSize.x, CV_8UC4);
                    memcpy(cvOrigImg_Curt.data, m_Keyframe_Curt.get_Img().data(), m_Keyframe_Curt.get_Img().size());
                    cv::cvtColor(cvOrigImg_Curt, cvOrigImg_Curt, cv::COLOR_BGRA2RGBA); // Transform to RGB before imshow & imwrite
                    cv::imwrite("E:\\Project_ARCHITECT\\ARKit_OrigImg_Curt.png", cvOrigImg_Curt);

                    cv::Mat cvOrigImg_Last(m_OrigImgSize.y, m_OrigImgSize.x, CV_8UC4);
                    memcpy(cvOrigImg_Last.data, m_Keyframe_Last.get_Img().data(), m_Keyframe_Last.get_Img().size());
                    cv::cvtColor(cvOrigImg_Last, cvOrigImg_Last, cv::COLOR_BGRA2RGBA); // Transform to RGB before imshow & imwrite
                    cv::imwrite("E:\\Project_ARCHITECT\\ARKit_OrigImg_Last.png", cvOrigImg_Last);
                }

                //---Rectification---
                FutoGmtCV::FutoImg RectImg_Curt, RectImg_Last;
                FutoGmtCV::SHomographyTransform Homo_B, Homo_M;

                FutoGmtCV::CRectification_Planar PlanarRectifier = FutoGmtCV::CRectification_Planar(m_Keyframe_Curt, m_Keyframe_Last);
                PlanarRectifier.execute(RectImg_Curt, RectImg_Last, Homo_B, Homo_M);

                // For OpenCV
                cv::Mat cvRectImg_Curt(RectImg_Curt.get_ImgSize().y, RectImg_Curt.get_ImgSize().x, CV_8UC1);
                memcpy(cvRectImg_Curt.data, RectImg_Curt.get_Img().data(), RectImg_Curt.get_Img().size());

                cv::Mat cvRectImg_Last(RectImg_Last.get_ImgSize().y, RectImg_Last.get_ImgSize().x, CV_8UC1);
                memcpy(cvRectImg_Last.data, RectImg_Last.get_Img().data(), RectImg_Last.get_Img().size());

                if (m_Is_imwrite)
                {
                    cv::imwrite("E:\\Project_ARCHITECT\\ARKit_RectImg_Curt.png", cvRectImg_Curt);

                    cv::imwrite("E:\\Project_ARCHITECT\\ARKit_RectImg_Last.png", cvRectImg_Last);
                }

                //---Stereo Matching---
                std::vector<char> DispImg_Rect(RectImg_Curt.get_Img().size(), 0.0);

                // For OpenCV
                cv::cuda::GpuMat cvRectImg_Curt_gpu, cvRectImg_Last_gpu;
                cvRectImg_Curt_gpu.upload(cvRectImg_Curt);
                cvRectImg_Last_gpu.upload(cvRectImg_Last);

                cv::cuda::GpuMat cvDispImg_Rect_gpu;
                cv::Mat cvDispImg_Rect_cpu;

                if (m_StereoMatching_Method == "LibSGM")
                {
                    m_pStereoMatcher_LibSGM = std::make_unique<sgm::StereoSGM>(RectImg_Curt.get_ImgSize().x, RectImg_Curt.get_ImgSize().y, m_DispRange, 8, 8, sgm::EXECUTE_INOUT_HOST2HOST);
                    m_pStereoMatcher_LibSGM->execute(RectImg_Curt.get_Img().data(), RectImg_Last.get_Img().data(), DispImg_Rect.data());

                    if (m_Is_imwrite)
                    {
                        cv::Mat cvDispImg_LibSGM(cvRectImg_Curt.size(), CV_8UC1);
                        memcpy(cvDispImg_LibSGM.data, DispImg_Rect.data(), DispImg_Rect.size());
                        cv::normalize(cvDispImg_LibSGM, cvDispImg_LibSGM, 0, 500, cv::NORM_MINMAX, CV_8UC1);
                        cv::imwrite("E:\\Project_ARCHITECT\\ARKit_DispImg_LibSGM.png", cvDispImg_LibSGM);
                    }
                    
                }

                if (m_StereoMatching_Method == "cv_bm_cuda")
                {
                    m_pStereoMatcher_cvBM_cuda = cv::cuda::createStereoBM(m_DispRange, 7); // Disparity Range, Böock Size
                    m_pStereoMatcher_cvBM_cuda->compute(cvRectImg_Curt_gpu, cvRectImg_Last_gpu, cvDispImg_Rect_gpu);

                    cvDispImg_Rect_gpu.download(cvDispImg_Rect_cpu);
                    if (cvDispImg_Rect_cpu.type() == CV_16S)
                    {
                        cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F, 1.0 / 16); 
                    }
                    
                    if (m_Is_imwrite)
                    {
                        cv::Mat cvDispImg_Rect_cpu_8bit(cvDispImg_Rect_cpu.size(), CV_8UC1);
                        cv::normalize(cvDispImg_Rect_cpu, cvDispImg_Rect_cpu_8bit, 0, 255, cv::NORM_MINMAX, CV_8UC1);
                        cv::imwrite("E:\\Project_ARCHITECT\\ARKit_DispImg_cvBM_cuda.png", cvDispImg_Rect_cpu_8bit);
                    }
                }

                if (m_StereoMatching_Method == "cv_bp_cuda")
                {
                    m_pStereoMatcher_cvBP_cuda = cv::cuda::createStereoBeliefPropagation();
                    m_pStereoMatcher_cvBP_cuda->compute(cvRectImg_Curt_gpu, cvRectImg_Last_gpu, cvDispImg_Rect_gpu);

                    cvDispImg_Rect_gpu.download(cvDispImg_Rect_cpu);
                    if (cvDispImg_Rect_cpu.type() == CV_16S)
                    {
                        cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F, 1.0 / 16);
                    }

                    if (m_Is_imwrite)
                    {
                        cv::Mat cvDispImg_Rect_cpu_8bit(cvDispImg_Rect_cpu.size(), CV_8UC1);
                        cv::normalize(cvDispImg_Rect_cpu, cvDispImg_Rect_cpu_8bit, 0, 255, cv::NORM_MINMAX, CV_8UC1);
                        cv::imwrite("E:\\Project_ARCHITECT\\ARKit_DispImg_cvBP_cuda.png", cvDispImg_Rect_cpu_8bit);
                    }
                }


                if (m_StereoMatching_Method == "cv_const_bp_cuda")
                {
                    m_pStereoMatcher_cvConstBP_cuda = cv::cuda::createStereoConstantSpaceBP();
                    m_pStereoMatcher_cvConstBP_cuda->compute(cvRectImg_Curt_gpu, cvRectImg_Last_gpu, cvDispImg_Rect_gpu);

                    cvDispImg_Rect_gpu.download(cvDispImg_Rect_cpu);
                    if (cvDispImg_Rect_cpu.type() == CV_16S)
                    {
                        cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F, 1.0 / 16);
                    }

                    if (m_Is_imwrite)
                    {
                        cv::Mat cvDispImg_Rect_cpu_8bit(cvDispImg_Rect_cpu.size(), CV_8UC1);
                        cv::normalize(cvDispImg_Rect_cpu, cvDispImg_Rect_cpu_8bit, 0, 255, cv::NORM_MINMAX, CV_8UC1);
                        cv::imwrite("E:\\Project_ARCHITECT\\ARKit_DispImg_cvConstBP_cuda.png", cvDispImg_Rect_cpu_8bit);
                    }
                }

                if (m_StereoMatching_Method == "cv_sgbm")
                {
                    m_pStereoMatcher_cvSGBM = cv::StereoSGBM::create();
                    m_pStereoMatcher_cvSGBM->compute(cvRectImg_Curt, cvRectImg_Last, cvDispImg_Rect_cpu);

                    cvDispImg_Rect_gpu.download(cvDispImg_Rect_cpu);
                    if (cvDispImg_Rect_cpu.type() == CV_16S)
                    {
                        cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F, 1.0 / 16);
                    }

                    if (m_Is_imwrite)
                    {
                        cv::Mat cvDispImg_Rect_cpu_8bit(cvDispImg_Rect_cpu.size(), CV_8UC1);
                        cv::normalize(cvDispImg_Rect_cpu, cvDispImg_Rect_cpu_8bit, 0, 255, cv::NORM_MINMAX, CV_8UC1);
                        cv::imwrite("E:\\Project_ARCHITECT\\ARKit_DispImg_cvSGBM.png", cvDispImg_Rect_cpu_8bit);
                    }
                }


                //---Transform Disparity in RectImg to Depth in OrigImg---
                // Initialize Texture Manager for Disparity in RectImg
                Gfx::STextureDescriptor TextureDescriptor_Disp = {};

                TextureDescriptor_Disp.m_NumberOfPixelsU = RectImg_Curt.get_ImgSize().x;
                TextureDescriptor_Disp.m_NumberOfPixelsV = RectImg_Curt.get_ImgSize().y;
                TextureDescriptor_Disp.m_NumberOfPixelsW = 1;
                TextureDescriptor_Disp.m_NumberOfMipMaps = 1;
                TextureDescriptor_Disp.m_NumberOfTextures = 1;
                TextureDescriptor_Disp.m_Binding = Gfx::CTexture::ShaderResource;
                TextureDescriptor_Disp.m_Access = Gfx::CTexture::EAccess::CPUWrite;
                TextureDescriptor_Disp.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
                TextureDescriptor_Disp.m_Semantic = Gfx::CTexture::UndefinedSemantic;
                TextureDescriptor_Disp.m_Format = Gfx::CTexture::R16_UBYTE; // 1 channels with 16-Ubit.

                m_Disp_Rect_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_Disp);

                // GPU Start
                Gfx::Performance::BeginEvent("Planar Rectification");

                // Submit Data to Managers
                Gfx::BufferManager::UploadBufferData(m_HomographyBufferPtr, &Homo_B);

                Base::AABB2UInt TargetRect;
                TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(RectImg_Curt.get_ImgSize().x, RectImg_Curt.get_ImgSize().y));
                Gfx::TextureManager::CopyToTexture2D(m_Disp_Rect_TexturePtr, TargetRect, RectImg_Curt.get_ImgSize().x, static_cast<const void*>(DispImg_Rect.data()));

                //---Connecting Managers (@CPU) & GLSL (@GPU)---
                Gfx::ContextManager::SetShaderCS(m_Disp2DepthCSPtr);
                Gfx::ContextManager::SetImageTexture(0, m_Disp_Rect_TexturePtr);
                Gfx::ContextManager::SetImageTexture(1, m_Depth_Orig_TexturePtr);
                Gfx::ContextManager::SetConstantBuffer(0, m_HomographyBufferPtr);
                Gfx::ContextManager::SetConstantBuffer(1, m_ParaxEqBufferPtr);

                //---Start GPU Parallel Processing---
                const int WorkGroupsX = DivUp(m_OrigImgSize.x, TileSize_2D);
                const int WorkGroupsY = DivUp(m_OrigImgSize.y, TileSize_2D);

                Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

                Gfx::ContextManager::ResetShaderCS();

                Gfx::Performance::EndEvent();
            }
            
            if (m_Is_TestData_MyMMS)
            {
                //---Input Test Images---
                cv::Mat cvInputImgL = cv::imread("E:\\Project_ARCHITECT\\01 Image Rectification\\My MMS\\Orig_01.jpg");
                cv::cvtColor(cvInputImgL, cvInputImgL, cv::COLOR_BGRA2RGBA);
                const int ImgSize_InputL_1D = cvInputImgL.rows * cvInputImgL.cols * 4;
                std::vector<char> InputImgL(ImgSize_InputL_1D);
                glm::ivec2 ImgSize_TestOrigL(cvInputImgL.cols, cvInputImgL.rows);
                memcpy(InputImgL.data(), cvInputImgL.data, ImgSize_InputL_1D);
                cv::Mat cvInputImgR = cv::imread("E:\\Project_ARCHITECT\\01 Image Rectification\\My MMS\\Orig_02.jpg");
                cv::cvtColor(cvInputImgR, cvInputImgR, cv::COLOR_BGRA2RGBA);
                const int ImgSize_InputR_1D = cvInputImgR.rows * cvInputImgR.cols * 4;
                std::vector<char> InputImgR(ImgSize_InputR_1D);
                glm::ivec2 ImgSize_TestOrigR(cvInputImgR.cols, cvInputImgR.rows);
                memcpy(InputImgR.data(), cvInputImgR.data, ImgSize_InputR_1D);

                glm::mat3 K_L(glm::vec3(1280.465, 0, 0), glm::vec3(0, 1280.465, 0), glm::vec3(712.961, 515.829, 1));
                glm::mat3 K_R(glm::vec3(1281.566, 0, 0), glm::vec3(0, 1281.566, 0), glm::vec3(698.496, 511.008, 1));

                glm::vec3 PC_L(0.0f);
                glm::vec3 PC_R(1.630, 0.016, -0.192);

                glm::mat3 R_L(1.0f);
                glm::mat3 R_R(glm::vec3(0.9999, -0.0084, 0.0132), glm::vec3(0.0085, 0.9999, -0.0122), glm::vec3(-0.0132, 0.0123, 0.9999));

                m_Keyframe_Curt = FutoGmtCV::FutoImg(InputImgL, ImgSize_TestOrigL, K_L, R_L, PC_L);
                m_Keyframe_Last = FutoGmtCV::FutoImg(InputImgR, ImgSize_TestOrigR, K_R, R_R, PC_R);

                //---Rectification---
                FutoGmtCV::FutoImg RectImg_Curt, RectImg_Last;
                FutoGmtCV::SHomographyTransform Homo_B, Homo_M;

                FutoGmtCV::CRectification_Planar PlanarRectifier = FutoGmtCV::CRectification_Planar(m_Keyframe_Curt, m_Keyframe_Last);
                PlanarRectifier.execute(RectImg_Curt, RectImg_Last, Homo_B, Homo_M);

                // For OpenCV
                cv::Mat cvRectImg_Curt(RectImg_Curt.get_ImgSize().y, RectImg_Curt.get_ImgSize().x, CV_8UC1);
                memcpy(cvRectImg_Curt.data, RectImg_Curt.get_Img().data(), RectImg_Curt.get_Img().size());

                cv::Mat cvRectImg_Last(RectImg_Last.get_ImgSize().y, RectImg_Last.get_ImgSize().x, CV_8UC1);
                memcpy(cvRectImg_Last.data, RectImg_Last.get_Img().data(), RectImg_Last.get_Img().size());

                if (m_Is_imwrite)
                {
                    cv::imwrite("E:\\Project_ARCHITECT\\ARKit_RectImg_Curt.png", cvRectImg_Curt);

                    cv::imwrite("E:\\Project_ARCHITECT\\ARKit_RectImg_Last.png", cvRectImg_Last);
                }

                //---Stereo Matching---
                std::vector<char> DispImg_Rect(RectImg_Curt.get_Img().size(), 0.0);

                // For OpenCV
                cv::cuda::GpuMat cvRectImg_Curt_gpu, cvRectImg_Last_gpu;
                cvRectImg_Curt_gpu.upload(cvRectImg_Curt);
                cvRectImg_Last_gpu.upload(cvRectImg_Last);

                cv::cuda::GpuMat cvDispImg_Rect_gpu;
                cv::Mat cvDispImg_Rect_cpu;

                if (m_StereoMatching_Method == "LibSGM")
                {
                    m_pStereoMatcher_LibSGM = std::make_unique<sgm::StereoSGM>(RectImg_Curt.get_ImgSize().x, RectImg_Curt.get_ImgSize().y, m_DispRange, 8, 8, sgm::EXECUTE_INOUT_HOST2HOST);
                    m_pStereoMatcher_LibSGM->execute(RectImg_Curt.get_Img().data(), RectImg_Last.get_Img().data(), DispImg_Rect.data());

                    if (m_Is_imwrite)
                    {
                        cv::Mat cvDispImg_LibSGM(cvRectImg_Curt.size(), CV_8UC1);
                        memcpy(cvDispImg_LibSGM.data, DispImg_Rect.data(), DispImg_Rect.size());
                        cv::normalize(cvDispImg_LibSGM, cvDispImg_LibSGM, 0, 500, cv::NORM_MINMAX, CV_8UC1);
                        cv::imwrite("E:\\Project_ARCHITECT\\MMS_DispImg_LibSGM.png", cvDispImg_LibSGM);
                    }

                }

                if (m_StereoMatching_Method == "cv_bm_cuda")
                {
                    m_pStereoMatcher_cvBM_cuda = cv::cuda::createStereoBM(m_DispRange, 7); // Disparity Range, Böock Size
                    m_pStereoMatcher_cvBM_cuda->compute(cvRectImg_Curt_gpu, cvRectImg_Last_gpu, cvDispImg_Rect_gpu);

                    cvDispImg_Rect_gpu.download(cvDispImg_Rect_cpu);
                    if (cvDispImg_Rect_cpu.type() == CV_16S)
                    {
                        cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F, 1.0 / 16);
                    }

                    if (m_Is_imwrite)
                    {
                        cv::Mat cvDispImg_Rect_cpu_8bit(cvDispImg_Rect_cpu.size(), CV_8UC1);
                        cv::normalize(cvDispImg_Rect_cpu, cvDispImg_Rect_cpu_8bit, 0, 255, cv::NORM_MINMAX, CV_8UC1);
                        cv::imwrite("E:\\Project_ARCHITECT\\MMS_DispImg_cvBM_cuda.png", cvDispImg_Rect_cpu_8bit);
                    }
                }

                if (m_StereoMatching_Method == "cv_bp_cuda")
                {
                    m_pStereoMatcher_cvBP_cuda = cv::cuda::createStereoBeliefPropagation();
                    m_pStereoMatcher_cvBP_cuda->compute(cvRectImg_Curt_gpu, cvRectImg_Last_gpu, cvDispImg_Rect_gpu);

                    cvDispImg_Rect_gpu.download(cvDispImg_Rect_cpu);
                    if (cvDispImg_Rect_cpu.type() == CV_16S)
                    {
                        cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F, 1.0 / 16);
                    }

                    if (m_Is_imwrite)
                    {
                        cv::Mat cvDispImg_Rect_cpu_8bit(cvDispImg_Rect_cpu.size(), CV_8UC1);
                        cv::normalize(cvDispImg_Rect_cpu, cvDispImg_Rect_cpu_8bit, 0, 255, cv::NORM_MINMAX, CV_8UC1);
                        cv::imwrite("E:\\Project_ARCHITECT\\MMS_DispImg_cvBP_cuda.png", cvDispImg_Rect_cpu_8bit);
                    }
                }


                if (m_StereoMatching_Method == "cv_const_bp_cuda")
                {
                    m_pStereoMatcher_cvConstBP_cuda = cv::cuda::createStereoConstantSpaceBP();
                    m_pStereoMatcher_cvConstBP_cuda->compute(cvRectImg_Curt_gpu, cvRectImg_Last_gpu, cvDispImg_Rect_gpu);

                    cvDispImg_Rect_gpu.download(cvDispImg_Rect_cpu);
                    if (cvDispImg_Rect_cpu.type() == CV_16S)
                    {
                        cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F, 1.0 / 16);
                    }

                    if (m_Is_imwrite)
                    {
                        cv::Mat cvDispImg_Rect_cpu_8bit(cvDispImg_Rect_cpu.size(), CV_8UC1);
                        cv::normalize(cvDispImg_Rect_cpu, cvDispImg_Rect_cpu_8bit, 0, 255, cv::NORM_MINMAX, CV_8UC1);
                        cv::imwrite("E:\\Project_ARCHITECT\\MMS_DispImg_cvConstBP_cuda.png", cvDispImg_Rect_cpu_8bit);
                    }
                }

                if (m_StereoMatching_Method == "cv_sgbm")
                {
                    m_pStereoMatcher_cvSGBM = cv::StereoSGBM::create();
                    m_pStereoMatcher_cvSGBM->compute(cvRectImg_Curt, cvRectImg_Last, cvDispImg_Rect_cpu);

                    cvDispImg_Rect_gpu.download(cvDispImg_Rect_cpu);
                    if (cvDispImg_Rect_cpu.type() == CV_16S)
                    {
                        cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F, 1.0 / 16);
                    }

                    if (m_Is_imwrite)
                    {
                        cv::Mat cvDispImg_Rect_cpu_8bit(cvDispImg_Rect_cpu.size(), CV_8UC1);
                        cv::normalize(cvDispImg_Rect_cpu, cvDispImg_Rect_cpu_8bit, 0, 255, cv::NORM_MINMAX, CV_8UC1);
                        cv::imwrite("E:\\Project_ARCHITECT\\MMS_DispImg_cvSGBM.png", cvDispImg_Rect_cpu_8bit);
                    }
                }



            }


            //===== OLD =====
            /*
            //DispImg_Rect.convertTo(DispImg_Rect, CV_32F, 1.0 / 16); // Disparity Image is in 16-bit -> Divide by 16 to get real Disparity.
            cv::Mat DispImg_Orig(m_Keyframe_Curt.get_Img().size(), CV_32F);
            cv::remap(DispImg_Rect, DispImg_Orig, Orig2Rect_Curt_x, Orig2Rect_Curt_y, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT); // !!! Warning: Using interpolation may cause additional errors !!!

            //---Show Disparity Image generated from Stereo Matching---
            cv::Mat DispImg_Rect_8U(DispImg_Rect.size(), CV_8UC1);
            cv::normalize(DispImg_Rect, DispImg_Rect_8U, 0, 255, cv::NORM_MINMAX, CV_8UC1);
            cv::imwrite("E:\\Project_ARCHITECT\\Disp_Rect.png", DispImg_Rect_8U);

            cv::Mat DispImg_Orig_8U(DispImg_Orig.size(), CV_8UC1);
            cv::normalize(DispImg_Orig, DispImg_Orig_8U, 0, 255, cv::NORM_MINMAX, CV_8UC1);
            cv::imwrite("E:\\Project_ARCHITECT\\Disp_Orig.png", DispImg_Orig_8U);

            //---Transform Disparity to Depth: Using Parallax Equation---
            cv::Mat DepthImg_Rect = cv::Mat::zeros(DispImg_Rect.size(), CV_32F);

            for (int x_idx = 0; x_idx < DepthImg_Rect.size().width; x_idx++)
            {
                for (int y_idx = 0; y_idx < DepthImg_Rect.size().height; y_idx++)
                {
                    DepthImg_Rect.ptr<float>(y_idx)[x_idx] = RectImg_Curt.get_Cam().ptr<float>(0)[0] * BaseLineLength / DispImg_Rect.ptr<float>(y_idx)[x_idx];
                    float f = RectImg_Curt.get_Cam().ptr<float>(0)[0];
                    float Disparity = DispImg_Rect.ptr<float>(y_idx)[x_idx];
                    float Depth = DepthImg_Rect.ptr<float>(y_idx)[x_idx];
                    DepthImg_Rect.ptr<float>(y_idx)[x_idx] *= 1000; // Unit = mm
                }
            }

            cv::Mat DepthImg_Orig = cv::Mat::zeros(DispImg_Orig.size(), CV_32F);
            cv::remap(DepthImg_Rect, DepthImg_Orig, Orig2Rect_Curt_x, Orig2Rect_Curt_y, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT); // !!! Warning: Using interpolation may cause additional errors !!!
            //---Show Depth in pixel---

            float Depth = DepthImg_Orig.ptr<float>(166)[192];

            //---

            //---Show Depth Image---
            cv::Mat DepthImg_Rect_8U(DepthImg_Rect.size(), CV_8UC1);
            cv::normalize(DepthImg_Rect, DepthImg_Rect_8U, 0, 255, cv::NORM_MINMAX, CV_8UC1);
            cv::imwrite("E:\\Project_ARCHITECT\\Depth_Rect.png", DepthImg_Rect_8U);
            cv::Mat DepthImg_Orig_8U(DepthImg_Orig.size(), CV_8UC1);
            cv::normalize(DepthImg_Orig, DepthImg_Orig_8U, 0, 255, cv::NORM_MINMAX, CV_8UC1);
            cv::imwrite("E:\\Project_ARCHITECT\\Depth_Orig.png", DepthImg_Orig_8U);

            cv::Mat DepthImg_Orig_Sensor(cv::Size(m_OrigImgSize.x, m_OrigImgSize.y), CV_16UC1); // 2D Matrix(x*y) with (16-bit unsigned character & 1 Channel)
            memcpy(DepthImg_Orig_Sensor.data, _rDepthImage.data(), _rDepthImage.size() * sizeof(_rDepthImage[0]));
            cv::Mat DepthImg_Orig_Sensor_8U = cv::Mat(DepthImg_Orig_Sensor.size(), CV_8UC1);
            cv::normalize(DepthImg_Orig_Sensor, DepthImg_Orig_Sensor_8U, 0, 255, cv::NORM_MINMAX, CV_8UC1);
            cv::imwrite("E:\\Project_ARCHITECT\\Depth_Orig_Sensor.png", DepthImg_Orig_Sensor_8U);
            */

            //---Finish Processing -> Free last keyframe---
            m_idx_Keyf_Last = false;

            m_Cdt_Keyf_BaseLineL = 1000000000; //---Temp Setting: Only calculate once, because it takes too much time calculating in CPU...
        }
        
        // Optional for internal check
        CPluginInterface::ShowImg(_rRGBImage); // Showing image for visual checking -> Modify to control in editor.config
        
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
    void CPluginInterface::imp_Rectification()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::ShowImg(const std::vector<char>& Img_RGBA) const
    {
        cv::Mat CV_Img(cv::Size(m_OrigImgSize.x, m_OrigImgSize.y), CV_8UC4); // 2D Matrix(x*y) with (8-bit unsigned character) + (4 bands)
            // cv::Mat is built in BGR/BGRA in default.
            // cv::Mat is ImgH(Num of Row), ImgW(Num of Col)
        
        memcpy(CV_Img.data, Img_RGBA.data(), Img_RGBA.size());
        
        cv::cvtColor(CV_Img, CV_Img, cv::COLOR_BGRA2RGBA); // Transform to RGB before imshow & imwrite
        
        cv::imshow("Hello", CV_Img);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnStart()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin started!");

        //---Programming Setting---
        m_Is_imwrite = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_setting:test_mms", true);

        m_Is_ARKitData = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_setting:input_data:arkit", true);
        m_Is_TestData_MyMMS = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_setting:input_data:test_mms", true);

        //---Frame Resolution given by ARKit---
        m_FrameResolution = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_frame:frame_resolution", 0.5); // Full = 1; Half = 0.5;

        //---Keyframe Selection---
        m_Cdt_Keyf_MaxNum = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_frame:max_keyframe_number", 2);
        m_Cdt_Keyf_BaseLineL = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_frame:max_keyframe_baseline_length", 0.03);

        //---Keyframe Status for Calculation---
        m_idx_Keyf_Curt = false;
        m_idx_Keyf_Last = false;

        //---Rectification-----
        m_RectImgSize = Core::CProgramParameters::GetInstance().Get("mr:stereo:01_rectification:rectified_image_size", glm::ivec2(1280, 1040));

        //---Stereo Matching---
        m_DispRange = Core::CProgramParameters::GetInstance().Get("mr:stereo:02_stereo_matching:disparity_range", 128);

        m_StereoMatching_Method = Core::CProgramParameters::GetInstance().Get("mr:stereo:02_stereo_matching:Method", "cvBM_cuda");

        //---Disparity to Depth---
        // Initialize Shader Manager
        std::stringstream DefineStream;
        DefineStream
            << "#define TILE_SIZE_2D " << TileSize_2D << " \n"; // 16 for work group size is suggested for 2D image (based on experience).
        std::string DefineString = DefineStream.str();

        m_Disp2DepthCSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/cs_Disp_to_Depth.glsl", "main", DefineString.c_str());

        // Initialize Buffer Manager
        Gfx::SBufferDescriptor HomoBufferDesc = {};

        HomoBufferDesc.m_Stride = 0;
        HomoBufferDesc.m_Usage = Gfx::CBuffer::GPURead;
        HomoBufferDesc.m_Binding = Gfx::CBuffer::ConstantBuffer;
        HomoBufferDesc.m_Access = Gfx::CBuffer::CPUWrite;
        HomoBufferDesc.m_NumberOfBytes = sizeof(FutoGmtCV::SHomographyTransform);
        HomoBufferDesc.m_pBytes = nullptr;
        HomoBufferDesc.m_pClassKey = 0;

        m_HomographyBufferPtr = Gfx::BufferManager::CreateBuffer(HomoBufferDesc);

        Gfx::SBufferDescriptor ParaxEqBufferDesc = {};

        ParaxEqBufferDesc.m_Stride = 0;
        ParaxEqBufferDesc.m_Usage = Gfx::CBuffer::GPURead;
        ParaxEqBufferDesc.m_Binding = Gfx::CBuffer::ConstantBuffer;
        ParaxEqBufferDesc.m_Access = Gfx::CBuffer::CPUWrite;
        ParaxEqBufferDesc.m_NumberOfBytes = sizeof(SParallaxEquation);
        ParaxEqBufferDesc.m_pBytes = nullptr;
        ParaxEqBufferDesc.m_pClassKey = 0;

        m_ParaxEqBufferPtr = Gfx::BufferManager::CreateBuffer(ParaxEqBufferDesc);

    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        //---Release Manager---
        m_Disp2DepthCSPtr = nullptr;
        m_Disp_Rect_TexturePtr = nullptr;
        m_Depth_Orig_TexturePtr = nullptr;
        m_HomographyBufferPtr = nullptr;
        m_ParaxEqBufferPtr = nullptr;
        
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

} // namespace HW

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4& _Transform, const glm::mat4& _Intrinsics, const std::vector<uint16_t>& _rDepthImage)
{
    static_cast<Stereo::CPluginInterface&>(GetInstance()).OnFrameCPU(_rRGBImage, _Transform, _Intrinsics, _rDepthImage);
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void GetLatestDepthImageCPU(std::vector<char>& _rDepthImage)
{
    _rDepthImage = static_cast<Stereo::CPluginInterface&>(GetInstance()).GetLatestDepthImageCPU();
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