
#include "plugin/stereo/stereo_precompiled.h"

#include "base/base_json.h"

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

    nlohmann::json Container_AgiIO, Container_AgiEO;
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
        TextureDescriptor_Depth_OrigImg.m_Access = Gfx::CTexture::EAccess::CPUWrite;
        TextureDescriptor_Depth_OrigImg.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
        TextureDescriptor_Depth_OrigImg.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextureDescriptor_Depth_OrigImg.m_Format = Gfx::CTexture::R16_UINT; // 1 channels with 16-bit uint.
        TextureDescriptor_Depth_OrigImg.m_pFileName = nullptr;
        TextureDescriptor_Depth_OrigImg.m_pPixels = nullptr;

        m_Depth_OrigImg_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_Depth_OrigImg);

        m_Depth_Sensor_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_Depth_OrigImg);

        m_Depth_Difference_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_Depth_OrigImg);
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
            //---Set Current Keyframe---
            m_Keyframe_Curt = frame;
            m_idx_Keyf_Curt = true;
        }
        else if (m_idx_Keyf_Curt && !m_idx_Keyf_Last) // Current keyframe exists but Last keyframe is empty -> Set both current & last keyframes.
        {
            //---Calculate Baseline Length---
            glm::vec3 BaseLine = frame.get_PC() - m_Keyframe_Curt.get_PC();
            float BaseLineLength = glm::l2Norm(BaseLine);

            if (BaseLineLength >= m_Cdt_Keyf_BaseLineL) // Select Keyframe: Baseline condition
            {
                //---Set Current & Last Keyframe---
                m_Keyframe_Last = m_Keyframe_Curt;
                m_Keyframe_Curt = frame;
                m_idx_Keyf_Last = true;

                m_KeyfID++;

                if (m_Is_AgiOri)
                {
                    using namespace nlohmann;

                    auto Count_jsonPtr = 0;

                    glm::mat3 _rCamMtx;

                    while (true)
                    {
                        std::string jsonPtrStr("/sensors/sensor/" + std::to_string(Count_jsonPtr) + "/-label");
                        auto jsonPtr = json::json_pointer(jsonPtrStr);
                        std::string jsonValue = Container_AgiIO[jsonPtr];
                        int AgiLabel = std::stoi(jsonValue);

                        if (AgiLabel == m_KeyfID)
                        {
                            jsonPtrStr = "/sensors/sensor/" + std::to_string(Count_jsonPtr) + "/calibration/fx";
                            auto jsonPtr_fx = json::json_pointer(jsonPtrStr);
                            std::string jsonValue_fx = Container_AgiIO[jsonPtr_fx];
                            _rCamMtx[0].x = std::stof(jsonValue_fx);

                            jsonPtrStr = "/sensors/sensor/" + std::to_string(Count_jsonPtr) + "/calibration/fy";
                            auto jsonPtr_fy = json::json_pointer(json::json_pointer(jsonPtrStr));
                            std::string jsonValue_fy = Container_AgiIO[jsonPtr_fy];
                            _rCamMtx[1].y = std::stof(jsonValue_fy);

                            jsonPtrStr = "/sensors/sensor/" + std::to_string(Count_jsonPtr) + "/calibration/cx";
                            auto jsonPtr_cx = json::json_pointer(json::json_pointer(json::json_pointer(jsonPtrStr)));
                            std::string jsonValue_cx = Container_AgiIO[jsonPtr_cx];
                            _rCamMtx[2].x = std::stof(jsonValue_cx);

                            jsonPtrStr = "/sensors/sensor/" + std::to_string(Count_jsonPtr) + "/calibration/cy";
                            auto jsonPtr_cy = json::json_pointer(json::json_pointer(json::json_pointer(jsonPtrStr)));
                            std::string jsonValue_cy = Container_AgiIO[jsonPtr_cy];
                            _rCamMtx[2].y = std::stof(jsonValue_cy);

                            break;
                        }

                        Count_jsonPtr++;
                    }

                    Count_jsonPtr = 0;

                    glm::mat3 _rRotMtx;
                    glm::vec3 _rPCVec;

                    glm::mat3 Rot_Foto2CV;
                    Rot_Foto2CV[1].y = -1;
                    Rot_Foto2CV[2].z = -1;

                    while (true)
                    {
                        std::string jsonPtrStr("/" + std::to_string(Count_jsonPtr) + "/PhotoID");
                        auto jsonPtr = json::json_pointer(jsonPtrStr);
                        std::string jsonValue = Container_AgiEO[jsonPtr];
                        int AgiLabel = std::stoi(jsonValue);

                        if (AgiLabel == m_KeyfID)
                        {
                            jsonPtrStr = "/" + std::to_string(Count_jsonPtr) + "/X";
                            auto jsonPtr_X = json::json_pointer(jsonPtrStr);
                            _rPCVec.x = Container_AgiEO[jsonPtr_X];

                            jsonPtrStr = "/" + std::to_string(Count_jsonPtr) + "/Y";
                            auto jsonPtr_Y = json::json_pointer(jsonPtrStr);
                            _rPCVec.y = Container_AgiEO[jsonPtr_Y];

                            jsonPtrStr = "/" + std::to_string(Count_jsonPtr) + "/Z";
                            auto jsonPtr_Z = json::json_pointer(jsonPtrStr);
                            _rPCVec.z = Container_AgiEO[jsonPtr_Z];

                            jsonPtrStr = "/" + std::to_string(Count_jsonPtr) + "/r11";
                            auto jsonPtr_r11 = json::json_pointer(jsonPtrStr);
                            _rRotMtx[0].x = Container_AgiEO[jsonPtr_r11];

                            jsonPtrStr = "/" + std::to_string(Count_jsonPtr) + "/r12";
                            auto jsonPtr_r12 = json::json_pointer(jsonPtrStr);
                            _rRotMtx[1].x = Container_AgiEO[jsonPtr_r12];

                            jsonPtrStr = "/" + std::to_string(Count_jsonPtr) + "/r13";
                            auto jsonPtr_r13 = json::json_pointer(jsonPtrStr);
                            _rRotMtx[2].x = Container_AgiEO[jsonPtr_r13];

                            jsonPtrStr = "/" + std::to_string(Count_jsonPtr) + "/r21";
                            auto jsonPtr_r21 = json::json_pointer(jsonPtrStr);
                            _rRotMtx[0].y = Container_AgiEO[jsonPtr_r21];

                            jsonPtrStr = "/" + std::to_string(Count_jsonPtr) + "/r22";
                            auto jsonPtr_r22 = json::json_pointer(jsonPtrStr);
                            _rRotMtx[1].y = Container_AgiEO[jsonPtr_r22];

                            jsonPtrStr = "/" + std::to_string(Count_jsonPtr) + "/r23";
                            auto jsonPtr_r23 = json::json_pointer(jsonPtrStr);
                            _rRotMtx[2].y = Container_AgiEO[jsonPtr_r23];

                            jsonPtrStr = "/" + std::to_string(Count_jsonPtr) + "/r31";
                            auto jsonPtr_r31 = json::json_pointer(jsonPtrStr);
                            _rRotMtx[0].z = Container_AgiEO[jsonPtr_r31];

                            jsonPtrStr = "/" + std::to_string(Count_jsonPtr) + "/r32";
                            auto jsonPtr_r32 = json::json_pointer(jsonPtrStr);
                            _rRotMtx[1].z = Container_AgiEO[jsonPtr_r32];

                            jsonPtrStr = "/" + std::to_string(Count_jsonPtr) + "/r33";
                            auto jsonPtr_r33 = json::json_pointer(jsonPtrStr);
                            _rRotMtx[2].z = Container_AgiEO[jsonPtr_r33];

                            _rRotMtx = Rot_Foto2CV * _rRotMtx;

                            break;
                        }

                        Count_jsonPtr++;
                    }

                    m_Keyframe_Curt = FutoGmtCV::CFutoImg(m_Keyframe_Curt.get_Img(), m_Keyframe_Curt.get_ImgSize(), 4, _rCamMtx, _rRotMtx, _rPCVec);
                }

                //***Export Original Images***
                std::string ExportStr;

                if (m_Is_imwrite)
                {
                    cv::Mat cvOrigImg_Curt(m_OrigImgSize.y, m_OrigImgSize.x, CV_8UC4);
                    memcpy(cvOrigImg_Curt.data, m_Keyframe_Curt.get_Img().data(), m_Keyframe_Curt.get_Img().size());
                    cv::cvtColor(cvOrigImg_Curt, cvOrigImg_Curt, cv::COLOR_BGRA2RGBA); // Transform to RGB before imshow & imwrite

                    ExportStr = "E:\\Project_ARCHITECT\\ARKit_OrigImg_Curt_" + std::to_string(m_KeyfID) + ".png";
                    cv::imwrite(ExportStr, cvOrigImg_Curt);
                    ExportStr = "";

                    m_ofstream_PC << "ARKit_OrigImg_Curt_" << m_KeyfID << ".png, ";
                    m_ofstream_PC << m_Keyframe_Curt.get_PC().x << ", ";
                    m_ofstream_PC << m_Keyframe_Curt.get_PC().y << ", ";
                    m_ofstream_PC << m_Keyframe_Curt.get_PC().z << std::endl;

                }

                //---Planar Rectification: Generate Rectified Keyframes---
                m_PlanarRectifier = FutoGmtCV::CRectification_Planar(m_Keyframe_Curt, m_Keyframe_Last);
                m_PlanarRectifier.execute(m_RectImg_Curt, m_RectImg_Last, m_Homo_Curt, m_Homo_Last);

                //***For OpenCV***
                cv::Mat cvRectImg_Curt(m_RectImg_Curt.get_ImgSize().y, m_RectImg_Curt.get_ImgSize().x, CV_8UC1);
                memcpy(cvRectImg_Curt.data, m_RectImg_Curt.get_Img().data(), m_RectImg_Curt.get_Img().size());

                cv::Mat cvRectImg_Last(m_RectImg_Last.get_ImgSize().y, m_RectImg_Last.get_ImgSize().x, CV_8UC1);
                memcpy(cvRectImg_Last.data, m_RectImg_Last.get_Img().data(), m_RectImg_Last.get_Img().size());

                //***Export Rectified Images***
                if (m_Is_imwrite)
                {
                    ExportStr << "E:\\Project_ARCHITECT\\ARKit_RectImg_Curt_" << m_KeyfID << ".png";
                    cv::imwrite(ExportStr.str(), cvRectImg_Curt);
                    ExportStr.clear();
                    ExportStr.str("");

                    ExportStr << "E:\\Project_ARCHITECT\\ARKit_RectImg_Last_" << m_KeyfID << ".png";
                    cv::imwrite(ExportStr.str(), cvRectImg_Last);
                    ExportStr.clear();
                    ExportStr.str("");
                }

                //---Stereo Matching: Generate Disparity in Rectified Current Keyframe---
                m_Disparity_RectImg.resize(m_RectImg_Curt.get_Img().size(), 0.0);

                //***For OpenCV***
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

                    if (cvDispImg_Rect_cpu.depth() == CV_16S)
                    {
                        cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F, 1.0 / 16);
                    }
                    else
                    {
                        cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F);
                    }

                    //***Export Disparity in Rectified Images (in 16-bit)***
                    if (m_Is_imwrite)
                    {
                        cv::Mat cvDisp_RectImg_cpu_16UC1(cvDispImg_Rect_cpu.size(), CV_16UC1);
                        cv::normalize(cvDispImg_Rect_cpu, cvDisp_RectImg_cpu_16UC1, 0, 65535, cv::NORM_MINMAX, CV_16UC1);

                        cv::imwrite("E:\\Project_ARCHITECT\\ARKit_DispImg_cvBM_cuda.png", cvDisp_RectImg_cpu_16UC1);
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

                //---Disparity to Depth in Rectified Current Keyframe---
                imp_Disp2Depth();

                //---Depth from Rectified to Original Current Keyframe---
                imp_Depth_Rect2Orig();

                //---Compare Depth from Stereo Matching & Sensor---
                Base::AABB2UInt TargetRect;
                TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_OrigImgSize.x, m_OrigImgSize.y));
                Gfx::TextureManager::CopyToTexture2D(m_Depth_Sensor_TexturePtr, TargetRect, m_OrigImgSize.x, static_cast<const void*>(_rDepthImage.data()));

                chk_Depth();

                //***** Export Depth in OrigImg (from Stereo Matching & Sensor) with 16-bit *****
                if (m_Is_imwrite)
                {
                    cv::Mat cvDepth_OrigImg(m_Keyframe_Curt.get_ImgSize().y, m_Keyframe_Curt.get_ImgSize().x, CV_16UC1);
                    memcpy(cvDepth_OrigImg.data, m_Depth_OrigImg.data(), m_Depth_OrigImg.size() * sizeof(m_Depth_OrigImg[0]));
                    ExportStr = "E:\\Project_ARCHITECT\\ARKit_DepthImg_OrigImg_" + std::to_string(m_KeyfID) + ".png";
                    cv::imwrite(ExportStr, cvDepth_OrigImg);
                    ExportStr = "";

                    cv::Mat cvDepth_Sensor(m_Keyframe_Curt.get_ImgSize().y, m_Keyframe_Curt.get_ImgSize().x, CV_16UC1);
                    memcpy(cvDepth_Sensor.data, _rDepthImage.data(), _rDepthImage.size() * sizeof(_rDepthImage[0]));
                    ExportStr = "E:\\Project_ARCHITECT\\ARKit_DepthImg_Sensor_" + std::to_string(m_KeyfID) + ".png";
                    cv::imwrite(ExportStr, cvDepth_Sensor);
                    ExportStr = "";
                }


                //---Return Depth in Original Image---
                const int MemCpySize = m_Depth_OrigImg_TexturePtr->GetNumberOfPixelsU() * m_Depth_OrigImg_TexturePtr->GetNumberOfPixelsV() * sizeof(uint16_t);
                std::vector<char> DepthImage(MemCpySize);
                Gfx::TextureManager::CopyTextureToCPU(m_Depth_OrigImg_TexturePtr, DepthImage.data());
                glm::mat4 Transform = glm::mat4(glm::transpose(m_Keyframe_Curt.get_Rot()));
                Transform[3] = glm::vec4(m_Keyframe_Curt.get_PC(), 1.0f);

                m_Delegate.Notify(m_Keyframe_Curt.get_Img(), DepthImage, Transform);

                //---Free Last Keyframe---
                m_idx_Keyf_Last = false;
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
        const int WorkGroupsX = DivUp(m_OrigImgSize.x, TileSize_2D);
        const int WorkGroupsY = DivUp(m_OrigImgSize.y, TileSize_2D);

        Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        Gfx::ContextManager::ResetShaderCS();
        Gfx::ContextManager::ResetImageTexture(1);

        Gfx::Performance::EndEvent();
        // GPU End
        m_Depth_OrigImg.resize(m_Depth_OrigImg_TexturePtr->GetNumberOfPixelsU() * m_Depth_OrigImg_TexturePtr->GetNumberOfPixelsV() * sizeof(uint16_t));
        Gfx::TextureManager::CopyTextureToCPU(m_Depth_OrigImg_TexturePtr, m_Depth_OrigImg.data());
        glm::mat4 Transform = glm::mat4(glm::transpose(m_Keyframe_Curt.get_Rot()));
        Transform[3] = glm::vec4(m_Keyframe_Curt.get_PC(), 1.0f);

        m_Delegate.Notify(m_Keyframe_Curt.get_Img(), m_Depth_OrigImg, Transform);
    }

    void CPluginInterface::chk_Depth()
    {
        //---GPU Start---
        Gfx::Performance::BeginEvent("Compare Depth");

        // Connecting Managers (@CPU) & GLSL (@GPU)
        Gfx::ContextManager::SetShaderCS(m_chk_Depth_CSPtr);
        Gfx::ContextManager::SetImageTexture(0, m_Depth_OrigImg_TexturePtr);
        Gfx::ContextManager::SetImageTexture(1, m_Depth_Sensor_TexturePtr);
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

    void CPluginInterface::OnStart()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin started!");

        //---Program Design Setting---
        m_Is_imwrite = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_program_design_setting:show_result", true);

        m_Is_AgiOri = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_program_design_setting:reference_ori", false);

        m_ofstream_PC = std::ofstream("E:\\Project_ARCHITECT\\ARKit_CameraPosition.txt", std::ios::trunc);

        std::ifstream fin_AgiIO("AgiIO.json");

        if (m_Is_AgiOri)
        {
            std::ifstream fin_AgiIO("AgiIO.json");
            if (fin_AgiIO.is_open())
            {
                std::string FileContent((std::istreambuf_iterator<char>(fin_AgiIO)), std::istreambuf_iterator<char>());

                Container_AgiIO = nlohmann::json::parse(FileContent);

                fin_AgiIO.close();
            }

            std::ifstream fin_AgiEO("AgiEO.json");
            if (fin_AgiEO.is_open())
            {
                std::string FileContent((std::istreambuf_iterator<char>(fin_AgiEO)), std::istreambuf_iterator<char>());

                Container_AgiEO = nlohmann::json::parse(FileContent);

                fin_AgiEO.close();
            }
        }

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

        //---Compare Depth---
        m_chk_Depth_CSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/cs_Compare_Depth.glsl", "main", DefineString.c_str());
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        //---Release Manager---
        m_Disp2Depth_CSPtr = nullptr;
        m_Disp_RectImg_TexturePtr = nullptr;
        m_Depth_RectImg_TexturePtr = nullptr;
        m_ParaxEq_BufferPtr = nullptr;

        m_ofstream_PC.close();
        
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