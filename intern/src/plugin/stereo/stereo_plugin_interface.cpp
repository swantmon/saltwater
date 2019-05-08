
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
        float m_BaselineLength; // Unit = meter
        float m_FocalLength; // Unit = pixel
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

        //
        Gfx::STextureDescriptor TextureDescriptor_Depth_OrigImg = {};

        TextureDescriptor_Depth_OrigImg.m_NumberOfPixelsU = m_OrigImgSize.x;
        TextureDescriptor_Depth_OrigImg.m_NumberOfPixelsV = m_OrigImgSize.y;
        TextureDescriptor_Depth_OrigImg.m_NumberOfPixelsW = 1;
        TextureDescriptor_Depth_OrigImg.m_NumberOfMipMaps = 1;
        TextureDescriptor_Depth_OrigImg.m_NumberOfTextures = 1;
        TextureDescriptor_Depth_OrigImg.m_Binding = Gfx::CTexture::ShaderResource;
        TextureDescriptor_Depth_OrigImg.m_Access = Gfx::CTexture::EAccess::CPURead;
        TextureDescriptor_Depth_OrigImg.m_Usage = Gfx::CTexture::EUsage::GPUToCPU;
        TextureDescriptor_Depth_OrigImg.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextureDescriptor_Depth_OrigImg.m_Format = Gfx::CTexture::R16_UINT; // 1 channels with 16-bit uint.

        m_Depth_OrigImg_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_Depth_OrigImg);
    }

    // -----------------------------------------------------------------------------

    bool CPluginInterface::GetLatestFrameCPU(std::vector<char>& _ColorImage, std::vector<char>& _rDepthImage, glm::mat4& _rTransform)
    {
        return false;
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnFrameCPU(const std::vector<char>& _rRGBImage, const glm::mat4& _Transform, const glm::mat4& _Intrinsics, const std::vector<uint16_t>& _rDepthImage)
    {
        glm::mat3 Cam_mtx = glm::mat3(_Intrinsics) * m_FrameResolution; // Intrinsic should be modified according to frame resolution.
        Cam_mtx[2].z = 1;

        glm::mat3 Rot_mtx = glm::transpose(glm::mat3(_Transform));// Rotation given by ARKit is Camera2World, but Rotation in Photogrammetry needs World2Camera.

        glm::vec3 PC_vec = glm::vec3(_Transform[3]); // The last column of _Transform given by ARKit is the Position of Camera in World.

        FutoGmtCV::CFutoImg frame(_rRGBImage, m_OrigImgSize, 4, Cam_mtx, Rot_mtx, PC_vec);
        
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
        else // Both current & last keyframes exist. -> Computing -> Free last keyframe.
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
                m_PlanarRectifier = FutoGmtCV::CRectification_Planar(m_Keyframe_Curt, m_Keyframe_Last);
                m_PlanarRectifier.execute(m_RectImg_Curt, m_RectImg_Last, m_Homo_Curt, m_Homo_Last);

                // For OpenCV
                cv::Mat cvRectImg_Curt(m_RectImg_Curt.get_ImgSize().y, m_RectImg_Curt.get_ImgSize().x, CV_8UC1);
                memcpy(cvRectImg_Curt.data, m_RectImg_Curt.get_Img().data(), m_RectImg_Curt.get_Img().size());

                cv::Mat cvRectImg_Last(m_RectImg_Last.get_ImgSize().y, m_RectImg_Last.get_ImgSize().x, CV_8UC1);
                memcpy(cvRectImg_Last.data, m_RectImg_Last.get_Img().data(), m_RectImg_Last.get_Img().size());

                if (m_Is_imwrite)
                {
                    cv::imwrite("E:\\Project_ARCHITECT\\ARKit_RectImg_Curt.png", cvRectImg_Curt);

                    cv::imwrite("E:\\Project_ARCHITECT\\ARKit_RectImg_Last.png", cvRectImg_Last);
                }

                //---Stereo Matching---
                m_Disparity_RectImg.resize(m_RectImg_Curt.get_Img().size(), 0.0);

                // For OpenCV
                cv::cuda::GpuMat cvRectImg_Curt_gpu, cvRectImg_Last_gpu;
                cvRectImg_Curt_gpu.upload(cvRectImg_Curt);
                cvRectImg_Last_gpu.upload(cvRectImg_Last);

                cv::cuda::GpuMat cvDispImg_Rect_gpu;
                cv::Mat cvDispImg_Rect_cpu;

                if (m_StereoMatching_Method == "LibSGM")
                {
                    m_pStereoMatcher_LibSGM = std::make_unique<sgm::StereoSGM>(m_RectImg_Curt.get_ImgSize().x, m_RectImg_Curt.get_ImgSize().y, m_DispRange, 8, 16, sgm::EXECUTE_INOUT_HOST2HOST);
                    // * Disparity needs to be Transformed from 16-bit signed integer to 32-bit float 
                    m_pStereoMatcher_LibSGM->execute(m_RectImg_Curt.get_Img().data(), m_RectImg_Last.get_Img().data(), m_Disparity_RectImg.data());

                    if (m_Is_imwrite)
                    {
                        cv::Mat cvDispImg_LibSGM(cvRectImg_Curt.size(), CV_8UC1);
                        memcpy(cvDispImg_LibSGM.data, m_Disparity_RectImg.data(), m_Disparity_RectImg.size());
                        cv::normalize(cvDispImg_LibSGM, cvDispImg_LibSGM, 0, 500, cv::NORM_MINMAX, CV_8UC1);
                        cv::imwrite("E:\\Project_ARCHITECT\\ARKit_DispImg_LibSGM.png", cvDispImg_LibSGM);
                    }
                    
                }

                if (m_StereoMatching_Method == "cvBM_cuda")
                {
                    m_pStereoMatcher_cvBM_cuda = cv::cuda::createStereoBM(m_DispRange, 7); // Disparity Range, Block Size
                    m_pStereoMatcher_cvBM_cuda->compute(cvRectImg_Curt_gpu, cvRectImg_Last_gpu, cvDispImg_Rect_gpu);

                    cvDispImg_Rect_gpu.download(cvDispImg_Rect_cpu);

                    switch (cvDispImg_Rect_cpu.depth())
                    {
                    case CV_16S:
                        cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F, 1.0 / 16);
                        break;
                    case CV_32F:
                        cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F);
                        break;
                    case CV_8U:
                        cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F);
                        break;
                    }
                    
                    if (m_Is_imwrite)
                    {
                        cv::Mat cvDisp_RectImg_cpu_8bit(cvDispImg_Rect_cpu.size(), CV_8UC1);
                        cv::normalize(cvDispImg_Rect_cpu, cvDisp_RectImg_cpu_8bit, 0, 255, cv::NORM_MINMAX, CV_8UC1);
                        cv::imwrite("E:\\Project_ARCHITECT\\ARKit_DispImg_cvBM_cuda.png", cvDisp_RectImg_cpu_8bit);
                    }

                    const int cvMemCpySize = cvDispImg_Rect_cpu.cols * cvDispImg_Rect_cpu.rows * cvDispImg_Rect_cpu.elemSize();
                    memcpy(m_Disparity_RectImg.data(), cvDispImg_Rect_cpu.data, cvMemCpySize);
                }

                if (m_StereoMatching_Method == "cvBP_cuda")
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


                if (m_StereoMatching_Method == "cvConstBP_cuda")
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

                if (m_StereoMatching_Method == "cv_SGBM")
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


                //---Transform Disparity to Depth in Rectified Image---
                imp_Disp2Depth();

                //---Transform Depth from Rectified to Original Image---
                imp_Depth_Rect2Orig();

                //---Return Depth in Original Image---

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

                m_Keyframe_Curt = FutoGmtCV::CFutoImg(InputImgL, ImgSize_TestOrigL, 3, K_L, R_L, PC_L);
                m_Keyframe_Last = FutoGmtCV::CFutoImg(InputImgR, ImgSize_TestOrigR, 3, K_R, R_R, PC_R);

                //---Rectification---
                FutoGmtCV::CFutoImg RectImg_Curt, RectImg_Last;
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
                    cv::imwrite("E:\\Project_ARCHITECT\\MMS_RectImg_Curt.png", cvRectImg_Curt);

                    cv::imwrite("E:\\Project_ARCHITECT\\MMS_RectImg_Last.png", cvRectImg_Last);
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

                if (m_StereoMatching_Method == "cvBM_cuda")
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

                if (m_StereoMatching_Method == "cvBP_cuda")
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


                if (m_StereoMatching_Method == "cvConstBP_cuda")
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

                if (m_StereoMatching_Method == "cv_SGBM")
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
        Gfx::TextureManager::CopyToTexture2D(m_Disp_RectImg_TexturePtr, TargetRect, m_RectImg_Curt.get_ImgSize().x, static_cast<const void*>(m_Disparity_RectImg.data()));

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
        Gfx::ContextManager::SetImageTexture(1, m_Depth_OrigImg_TexturePtr);
        Gfx::ContextManager::SetConstantBuffer(0, m_Homogrampy_BufferPtr);

        // Start GPU Parallel Processing
        const int WorkGroupsX = DivUp(m_Keyframe_Curt.get_ImgSize().x, TileSize_2D);
        const int WorkGroupsY = DivUp(m_Keyframe_Curt.get_ImgSize().y, TileSize_2D);

        Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        Gfx::ContextManager::ResetShaderCS();
        Gfx::ContextManager::ResetImageTexture(1);

        Gfx::Performance::EndEvent();
        // GPU End

        std::vector<char> DepthImage(m_Depth_OrigImg_TexturePtr->GetNumberOfPixelsU() * m_Depth_OrigImg_TexturePtr->GetNumberOfPixelsV() * sizeof(uint16_t));
        Gfx::TextureManager::CopyTextureToCPU(m_Depth_OrigImg_TexturePtr, DepthImage.data());
        glm::mat4 Transform = glm::mat4(glm::transpose(m_Keyframe_Curt.get_Rot()));
        Transform[3] = glm::vec4(m_Keyframe_Curt.get_PC(), 1.0f);

        m_Delegate.Notify(m_Keyframe_Curt.get_Img(), DepthImage, Transform);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnStart()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin started!");

        //---Program Design Setting---
        m_Is_ARKitData = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_program_design_setting:input_data:arkit", true);
        m_Is_TestData_MyMMS = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_program_design_setting:input_data:test_mms", false);

        m_Is_imwrite = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_program_design_setting:show_result", true);

        //---ARKit Data---
        m_FrameResolution = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_input_setting:frame_resolution", 0.5); // Full = 1; Half = 0.5;

        //---Keyframe---
        m_Cdt_Keyf_MaxNum = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_keyframe:max_keyframe_number", 2);
        m_Cdt_Keyf_BaseLineL = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_keyframe:max_keyframe_baseline_length", 0.03);

        m_idx_Keyf_Curt = false;
        m_idx_Keyf_Last = false;

        //---Rectification-----

        //---Stereo Matching---
        m_DispRange = Core::CProgramParameters::GetInstance().Get("mr:stereo:02_stereo_matching:disparity_range", 128);

        m_StereoMatching_Method = Core::CProgramParameters::GetInstance().Get("mr:stereo:02_stereo_matching:Method", "cvBM_cuda");

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
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        //---Release Manager---
        m_Disp2Depth_CSPtr = nullptr;
        m_Disp_RectImg_TexturePtr = nullptr;
        m_Depth_RectImg_TexturePtr = nullptr;
        m_ParaxEq_BufferPtr = nullptr;
        
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

} // namespace HW

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