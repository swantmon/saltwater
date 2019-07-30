
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

        //---Disparity Computation (Epipolarization & Stereo Matching)---
        if (m_Strategy == "normal")
        {
            m_Rectifier_Planar = FutoGCV::CPlanarRectification(m_OrigImgSize, FutoGCV::NORMAL);
        }
        else if (m_Strategy == "sub-image")
        {
            m_Rectifier_Planar = FutoGCV::CPlanarRectification(m_OrigImgSize, FutoGCV::SUBIMG, m_EpiImgSize);

            m_pStereoMatcher_LibSGM = std::make_unique<sgm::StereoSGM>(m_EpiImgSize.x, m_EpiImgSize.y, m_DispRange, 8, 16, sgm::EXECUTE_INOUT_HOST2HOST);
        } 
        else if (m_Strategy == "scaling")
        {
            m_Rectifier_Planar = FutoGCV::CPlanarRectification(m_OrigImgSize, FutoGCV::DOWNSAMPLING, m_EpiImgSize);

            m_pStereoMatcher_LibSGM = std::make_unique<sgm::StereoSGM>(m_EpiImgSize.x, m_EpiImgSize.y, m_DispRange, 8, 16, sgm::EXECUTE_INOUT_HOST2HOST);
        }

        //---Initialize EpiDisparity Texture Manager---
        Gfx::STextureDescriptor TextDesc_EpiDisp = {};
        TextDesc_EpiDisp.m_NumberOfPixelsU = m_OrigImgSize.x;
        TextDesc_EpiDisp.m_NumberOfPixelsV = m_OrigImgSize.y;
        TextDesc_EpiDisp.m_NumberOfPixelsW = 1;
        TextDesc_EpiDisp.m_NumberOfMipMaps = 1;
        TextDesc_EpiDisp.m_NumberOfTextures = 1;
        TextDesc_EpiDisp.m_Binding = Gfx::CTexture::ShaderResource;
        TextDesc_EpiDisp.m_Access = Gfx::CTexture::EAccess::CPURead;
        TextDesc_EpiDisp.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
        TextDesc_EpiDisp.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextDesc_EpiDisp.m_Format = Gfx::CTexture::R32_FLOAT; // 1 channels with 32-bit float.

        m_EpiDisparity_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextDesc_EpiDisp);

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
        //---Setting Orientations from ARKit---
        Base::AABB2UInt TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_OrigImgSize.x, m_OrigImgSize.y));
        Gfx::TextureManager::CopyToTexture2D(m_OrigImg_TexturePtr, TargetRect, m_OrigImgSize.x, static_cast<const void*>(_rRGBImage.data()));

        glm::mat3 K = glm::mat3(_FocalLength.x, 0, 0, 0, _FocalLength.y, 0, _FocalPoint.x, _FocalPoint.y, 1); 

        glm::mat3 R = glm::transpose(glm::mat3(_Transform));// ARKit's Rotation is Camera2World (Computer Graphics), but we need Rotation as World2Camera (Computer Vision).

        glm::vec3 PC = glm::vec3(_Transform[3]); // The last column of _Transform given by ARKit is the Position of Camera in World.
        
        //---Only Compute 2 Keyframes Once---
        if (!m_IsKeyfExist)
        {
            m_OrigKeyframe_Curt = FutoGCV::SFutoImg(m_OrigImg_TexturePtr, m_OrigImgSize, K, R, PC);
            m_IsKeyfExist = true;
        }
        else
        {
            //---Keyframe Selection---
            float BaseLineLength = glm::l2Norm(PC - m_OrigKeyframe_Curt.m_Position);

            if (BaseLineLength < m_SelectKeyf_BaseLineL) // Keyframe Selection: Baseline Length
            {
                return;
            }

            m_OrigKeyframe_Last = m_OrigKeyframe_Curt;
            m_OrigKeyframe_Curt = FutoGCV::SFutoImg(m_OrigImg_TexturePtr, m_OrigImgSize, K, R, PC);

            m_KeyfID++;

            if (m_IsExport_OrigImg)
            {
                export_OrigImg();
            }

            //---Epipolarization---
            m_Rectifier_Planar.ComputeEpiGeometry(m_OrigKeyframe_Curt, m_OrigKeyframe_Last);

            if (m_Rectifier_Planar.m_Is_LargeSize)
            {
                return; // LibSGM will break if the size of rectified image is too large.
            }

            m_Rectifier_Planar.ReturnResult(m_EpiKeyframe_Curt, m_EpiKeyframe_Last, m_Homo_Curt, m_Homo_Last);

            std::vector<char> vecEpiImg_Curt(m_EpiKeyframe_Curt.m_ImgSize.x * m_EpiKeyframe_Curt.m_ImgSize.y, 0);
            Gfx::TextureManager::CopyTextureToCPU(m_EpiKeyframe_Curt.m_Img_TexturePtr, reinterpret_cast<char*>(vecEpiImg_Curt.data()));

            std::vector<char> vecEpiImg_Last(m_EpiKeyframe_Curt.m_ImgSize.x * m_EpiKeyframe_Curt.m_ImgSize.y, 0);
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

            //---Stereo Matching---

            std::vector<uint16_t> vecEpiDisparity_uint16(m_EpiKeyframe_Curt.m_ImgSize.x * m_EpiKeyframe_Curt.m_ImgSize.y, 0);

            

            if (m_Strategy == "normal")
            {
                m_pStereoMatcher_LibSGM = std::make_unique<sgm::StereoSGM>(m_EpiKeyframe_Curt.m_ImgSize.x, m_EpiKeyframe_Curt.m_ImgSize.y, m_DispRange, 8, 16, sgm::EXECUTE_INOUT_HOST2HOST);
                    // Default disparity is pixel level => Disparity is the same in 8-bit & 16-bit.
                    // If turn on sub-pixel => Output disparity must be 16-bit. => Divided by 16 to derive true disparity!!!
                m_pStereoMatcher_LibSGM->execute(vecEpiImg_Curt.data(), vecEpiImg_Last.data(), vecEpiDisparity_uint16.data());
            }
            else if (m_Strategy == "sub-image")
            {
                
            }
            else if (m_Strategy == "scaling")
            {

            }

            std::vector<float> vecEpiDisparity = std::vector<float>::vector(vecEpiDisparity_uint16.begin(), vecEpiDisparity_uint16.end());

            

            // *** OLD ***

            //---Stereo Matching---
            // * Calculate Disparity in Rectified Current Image

            //const clock_t Time_SM_begin = clock();
            if (m_Is_Scaling)
            {
                m_DispImg_Rect.resize(RectImg_Curt_DownSample.get_Img().size(), 0.0f);
            } 
            else
            {
                m_DispImg_Rect.resize(m_RectImg_Curt.get_Img().size(), 0.0f);
            }

            if (m_StereoMatching_Mode == "Original")
            {
                if (m_Is_RectSubImg)
                {
                    imp_StereoMatching_Fix();
                } 
                else if (m_Is_Scaling)
                {
                    imp_StereoMatching_Scaling(RectImg_Curt_DownSample.get_Img(), RectImg_Last_DownSample.get_Img());
                }
                else
                {
                    imp_StereoMatching();
                }
            }
            else if (m_StereoMatching_Mode == "Tile")
            {
                imp_StereoMatching_Tile();
            }
            else if (m_StereoMatching_Mode == "Sub")
            {
                imp_StereoMatching_Sub();
            }

            //const clock_t Time_SM_end = clock();

            //float CalTime_SM = float(Time_SM_end - Time_SM_begin) / CLOCKS_PER_SEC;

            //---Disparity to Depth---
            // * Using Parallax Equation to Transform Disparity to Depth
            // * Depth is in Rectified Current Image

            imp_Disp2Depth();

            //---Depth from Rectified to Original---
            // * Based on Inverse Homography calculated in Rectification
            // * Depth is in Original Current Image & Horizontal Flipped (for Reconstruction)

            imp_Depth_Rect2Orig();

            //---Depth Analysis---
            if (m_Is_CompareDepth) // Compare Depth from plugin_stereo & Sensor
            {
                m_DepthImg_Sensor = _rDepthImage;

                cmp_Depth();
            }

            //---Test: Apply cvFGS---
            /*
            {
                cv::Mat cvOrigImg_Curt_BGRA(m_OrigImgSize.y, m_OrigImgSize.x, CV_8UC4), cvOrigImg_Curt_Gray(m_OrigImgSize.y, m_OrigImgSize.x, CV_8UC1);
                memcpy(cvOrigImg_Curt_BGRA.data, m_OrigImg_Curt.get_Img().data(), m_OrigImg_Curt.get_Img().size() * sizeof(m_OrigImg_Curt.get_Img()[0]));
                cv::cvtColor(cvOrigImg_Curt_BGRA, cvOrigImg_Curt_Gray, cv::COLOR_BGRA2GRAY); // Transform to RGB before imshow & imwrite

                m_pSmoother_cvFGS = cv::ximgproc::createFastGlobalSmootherFilter(cvOrigImg_Curt_Gray, 30, 0.01);

                cv::Mat cvDepthIn(m_OrigImgSize.y, m_OrigImgSize.x, CV_16SC1), cvDepthOut(m_OrigImgSize.y, m_OrigImgSize.x, CV_16SC1);
                memcpy(cvDepthIn.data, DepthImage.data(), MemCpySize);

                m_pSmoother_cvFGS->filter(cvDepthIn, cvDepthOut);

                DepthImage.clear();
                memcpy(DepthImage.data(), cvDepthOut.data, cvDepthOut.cols * cvDepthOut.rows * cvDepthOut.elemSize());
            }
            */

            //---Return Depth to plugin_slam---
            m_Delegate.Notify(_rRGBImage, m_DepthImg_Orig, _Transform, _FocalLength, _FocalPoint);

            if (m_IsExport_Depth)
            {
                export_Depth();
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

    void CPluginInterface::imp_StereoMatching()
    {
        if (m_StereoMatching_Method == "LibSGM")
        {
            
        }
        else if (m_StereoMatching_Method == "FuSGM")
        {
        }
        else // Method provided by OpenCV
        {
            cv::Mat cvRectImg_Curt(m_RectImg_Curt.get_ImgSize().y, m_RectImg_Curt.get_ImgSize().x, CV_8UC1);
            memcpy(cvRectImg_Curt.data, m_RectImg_Curt.get_Img().data(), m_RectImg_Curt.get_Img().size());

            cv::Mat cvRectImg_Last(m_RectImg_Last.get_ImgSize().y, m_RectImg_Last.get_ImgSize().x, CV_8UC1);
            memcpy(cvRectImg_Last.data, m_RectImg_Last.get_Img().data(), m_RectImg_Last.get_Img().size());

            cv::cuda::GpuMat cvRectImg_Curt_gpu, cvRectImg_Last_gpu;
            cvRectImg_Curt_gpu.upload(cvRectImg_Curt);
            cvRectImg_Last_gpu.upload(cvRectImg_Last);

            cv::cuda::GpuMat cvDispImg_Rect_gpu;
            cv::Mat cvDispImg_Rect_cpu;

            if (m_StereoMatching_Method == "cvBM_cuda")
            {
                m_pStereoMatcher_cvBM_cuda = cv::cuda::createStereoBM(m_DispRange, 7); // Disparity Range, Block Size

                m_pStereoMatcher_cvBM_cuda->compute(cvRectImg_Curt_gpu, cvRectImg_Last_gpu, cvDispImg_Rect_gpu);

                cvDispImg_Rect_gpu.download(cvDispImg_Rect_cpu);
                cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F);
            }
            else if (m_StereoMatching_Method == "cvBP_cuda")
            {
                m_pStereoMatcher_cvBP_cuda = cv::cuda::createStereoBeliefPropagation();

                m_pStereoMatcher_cvBP_cuda->compute(cvRectImg_Curt_gpu, cvRectImg_Last_gpu, cvDispImg_Rect_gpu);

                cvDispImg_Rect_gpu.download(cvDispImg_Rect_cpu); // Disparity is 16S but without fractional bit.
                cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F);
            }
            else if (m_StereoMatching_Method == "cvConstBP_cuda")
            {
                m_pStereoMatcher_cvConstBP_cuda = cv::cuda::createStereoConstantSpaceBP();

                m_pStereoMatcher_cvConstBP_cuda->compute(cvRectImg_Curt_gpu, cvRectImg_Last_gpu, cvDispImg_Rect_gpu);

                cvDispImg_Rect_gpu.download(cvDispImg_Rect_cpu); // Disparity is 16S but without fractional bit.
                cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F);
            }
            else if (m_StereoMatching_Method == "cv_SGBM")
            {
                m_pStereoMatcher_cvSGBM = cv::StereoSGBM::create();

                m_pStereoMatcher_cvSGBM->compute(cvRectImg_Curt, cvRectImg_Last, cvDispImg_Rect_cpu);
                cvDispImg_Rect_cpu.convertTo(cvDispImg_Rect_cpu, CV_32F, 1.0 / 16); // Disparity is 16S with 4 fractional bits.
            }
            else
            {

            }

            const auto cvMemCpySize = cvDispImg_Rect_cpu.cols * cvDispImg_Rect_cpu.rows * cvDispImg_Rect_cpu.elemSize();
            memcpy(m_DispImg_Rect.data(), cvDispImg_Rect_cpu.data, cvMemCpySize);

        }
    }

    void CPluginInterface::imp_StereoMatching_Fix()
    {
        if (m_StereoMatching_Method == "LibSGM")
        {
            std::vector<uint16_t> DispImg_Rect_uint16(m_RectImg_Curt.get_Img().size(), 0);

            m_pStereoMatcher_LibSGM->execute(m_RectImg_Curt.get_Img().data(), m_RectImg_Last.get_Img().data(), DispImg_Rect_uint16.data());

            m_DispImg_Rect = std::vector<float>::vector(DispImg_Rect_uint16.begin(), DispImg_Rect_uint16.end());
        }
    }

    void CPluginInterface::imp_StereoMatching_Tile()
    {
        const auto Tile_Size = m_OrigImgSize.x < m_OrigImgSize.y ? m_OrigImgSize.x : m_OrigImgSize.y;
        const auto BufferPix = int(Tile_Size * 0.1);
        const auto BuffTile_Size = Tile_Size + 2 * BufferPix;

        std::vector<char> BuffTile_CurtImg(BuffTile_Size * BuffTile_Size, 0), BuffTile_LastImg(BuffTile_Size * BuffTile_Size, 0);
        std::vector<uint16_t> BuffTile_Disp(BuffTile_CurtImg.size(), 0);


        glm::ivec2 Tile_Num;
        Tile_Num.x = static_cast<int>(ceil(float(m_RectImg_Curt.get_ImgSize().x) / Tile_Size));
        Tile_Num.y = static_cast<int>(ceil(float(m_RectImg_Curt.get_ImgSize().y) / Tile_Size));

        for (auto idx_TileNum_y = 0; idx_TileNum_y < Tile_Num.y; idx_TileNum_y++)
        {
            for (auto idx_TileNum_x = 0; idx_TileNum_x < Tile_Num.x; idx_TileNum_x++)
            {
                //---Extract tiles---
                for (auto idx_BuffTilePix_y = 0; idx_BuffTilePix_y < BuffTile_Size; idx_BuffTilePix_y++)
                {
                    glm::ivec2 ImgBound;

                    ImgBound.x = (idx_TileNum_y * Tile_Size + idx_BuffTilePix_y - BufferPix) * m_RectImg_Curt.get_ImgSize().x; // Left Boundary
                    if (ImgBound.x < 0)
                    {
                        ImgBound.x = 0;
                    }
                    if (ImgBound.x >= m_RectImg_Curt.get_Img().size())
                    {
                        ImgBound.x = static_cast<int>(m_RectImg_Curt.get_Img().size() - m_RectImg_Curt.get_ImgSize().x);
                    }

                    ImgBound.y = ImgBound.x + m_RectImg_Curt.get_ImgSize().x - 1; // Right Boundary
                    assert(ImgBound.y < m_RectImg_Curt.get_Img().size());

                    for (auto idx_BuffTilePix_x = 0; idx_BuffTilePix_x < BuffTile_Size; idx_BuffTilePix_x++)
                    {
                        const auto BuffTilePos_Tile = idx_BuffTilePix_x + idx_BuffTilePix_y * BuffTile_Size;
                        const auto BuffTilePos_Img = (idx_BuffTilePix_x - BufferPix) + (idx_BuffTilePix_y - BufferPix) * m_RectImg_Curt.get_ImgSize().x;
                        const auto ImgPos = BuffTilePos_Img + idx_TileNum_x * Tile_Size + idx_TileNum_y * Tile_Size * m_RectImg_Curt.get_ImgSize().x;

                        if (ImgPos < ImgBound.x || ImgPos > ImgBound.y)
                        {
                            BuffTile_CurtImg[BuffTilePos_Tile] = 0;
                            BuffTile_LastImg[BuffTilePos_Tile] = 0;
                        }
                        else
                        {
                            BuffTile_CurtImg[BuffTilePos_Tile] = m_RectImg_Curt.get_Img()[ImgPos];
                            BuffTile_LastImg[BuffTilePos_Tile] = m_RectImg_Last.get_Img()[ImgPos];
                        }
                    }
                }

                /*
                cv::Mat cvTileImg_Curt(BuffTile_Size, BuffTile_Size, CV_8UC1);
                memcpy(cvTileImg_Curt.data, BuffTile_CurtImg.data(), BuffTile_CurtImg.size() * sizeof(BuffTile_CurtImg[0]));
                cv::imshow("TileImg_Curt", cvTileImg_Curt);

                cv::Mat cvTileImg_Last(BuffTile_Size, BuffTile_Size, CV_8UC1);
                memcpy(cvTileImg_Last.data, BuffTile_LastImg.data(), BuffTile_LastImg.size() * sizeof(BuffTile_LastImg[0]));
                cv::imshow("TileImg_Last", cvTileImg_Last);

                cv::waitKey();
                */

                m_pStereoMatcher_LibSGM->execute(BuffTile_CurtImg.data(), BuffTile_LastImg.data(), BuffTile_Disp.data());

                //---Combine tile result back to whole result---
                for (auto idx_TilePix_y = 0; idx_TilePix_y < Tile_Size; idx_TilePix_y++)
                {
                    for (auto idx_TilePix_x = 0; idx_TilePix_x < Tile_Size; idx_TilePix_x++)
                    {
                        const auto TilePos_Tile = (idx_TilePix_x + BufferPix) + (idx_TilePix_y + BufferPix) * BuffTile_Size;
                        const auto TilePos_Img = idx_TilePix_x + idx_TilePix_y * m_RectImg_Curt.get_ImgSize().x;
                        const auto ImgPos = TilePos_Img + idx_TileNum_x * Tile_Size + idx_TileNum_y * Tile_Size * m_RectImg_Curt.get_ImgSize().x;

                        if (ImgPos < m_DispImg_Rect.size())
                        {
                            m_DispImg_Rect[ImgPos] = BuffTile_Disp[TilePos_Tile];
                        } 
                    }
                }
            }
        }
    }

    void CPluginInterface::imp_StereoMatching_Sub()
    {
        const glm::ivec2 SubImg_Size = m_RectImgSize_Sub;

        const glm::ivec2 SubImg_Center = m_RectImg_Curt.get_ImgSize() / 2;

        const glm::ivec2 SubImg_LU(SubImg_Center.x - SubImg_Size.x / 2, SubImg_Center.y - SubImg_Size.y / 2);
        const int SubImg_LU_1D = SubImg_LU.x + SubImg_LU.y * m_RectImg_Curt.get_ImgSize().x;

        std::vector<char> SubRectImg_Curt(SubImg_Size.x * SubImg_Size.y, 0), SubRectImg_Last(SubImg_Size.x * SubImg_Size.y, 0);
        std::vector<uint16_t> SubDisparity(SubRectImg_Curt.size(), 0);

        for (auto idx_y = 0; idx_y < SubImg_Size.y; idx_y++)
        {
            for (auto idx_x = 0; idx_x < SubImg_Size.x; idx_x++)
            {
                const int PixPosition_Sub = idx_x + idx_y * SubImg_Size.x;
                const int PixPosition_Img = SubImg_LU_1D + idx_x + idx_y * m_RectImg_Curt.get_ImgSize().x;

                if (PixPosition_Img >= 0 && PixPosition_Img < m_RectImg_Curt.get_Img().size())
                {
                    SubRectImg_Curt[PixPosition_Sub] = m_RectImg_Curt.get_Img()[PixPosition_Img];
                    SubRectImg_Last[PixPosition_Sub] = m_RectImg_Last.get_Img()[PixPosition_Img];
                }
                else
                {
                    SubRectImg_Curt[PixPosition_Sub] = 0;
                    SubRectImg_Last[PixPosition_Sub] = 0;
                }
            }
        }

        /*
        cv::Mat cvSubImg_Curt(SubImg_Size.y, SubImg_Size.x, CV_8UC1);
        memcpy(cvSubImg_Curt.data, SubRectImg_Curt.data(), SubRectImg_Curt.size() * sizeof(SubRectImg_Curt[0]));
        cv::imshow("SubImg_Curt", cvSubImg_Curt);

        cv::Mat cvSubImg_Last(SubImg_Size.y, SubImg_Size.x, CV_8UC1);
        memcpy(cvSubImg_Last.data, SubRectImg_Last.data(), SubRectImg_Last.size() * sizeof(SubRectImg_Last[0]));
        cv::imshow("SubImg_Last", cvSubImg_Last);

        cv::waitKey();
         */

        m_pStereoMatcher_LibSGM->execute(SubRectImg_Curt.data(), SubRectImg_Last.data(), SubDisparity.data());

        for (auto idx_y = 0; idx_y < SubImg_Size.y; idx_y++)
        {
            for (auto idx_x = 0; idx_x < SubImg_Size.x; idx_x++)
            {
                const int PixPosition_Sub = idx_x + idx_y * SubImg_Size.x;
                const int PixPosition_Img = SubImg_LU_1D + idx_x + idx_y * m_RectImg_Curt.get_ImgSize().x;

                if (PixPosition_Img >= 0 && PixPosition_Img < m_RectImg_Curt.get_Img().size())
                {
                    m_DispImg_Rect[PixPosition_Img] = SubDisparity[PixPosition_Sub];
                }
            }
        }
    }

    void CPluginInterface::imp_StereoMatching_Scaling(const std::vector<char>& RectImg_Curt_DownSample, const std::vector<char>& RectImg_Last_DownSample)
    {
        if (m_StereoMatching_Method == "LibSGM")
        {
            std::vector<uint16_t> DispImg_Rect_uint16(RectImg_Curt_DownSample.size(), 0);

            m_pStereoMatcher_LibSGM->execute(RectImg_Curt_DownSample.data(), RectImg_Last_DownSample.data(), DispImg_Rect_uint16.data());

            m_DispImg_Rect = std::vector<float>::vector(DispImg_Rect_uint16.begin(), DispImg_Rect_uint16.end());
        }
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::imp_Disp2Depth()
    {
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

        m_DispImg_Rect_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc_DispImg_Rect);
        m_Disp_HR_BiLinear_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc_DispImg_Rect);

        TextureDesc_DispImg_Rect.m_Format = Gfx::CTexture::R8_UBYTE; // 1 channels with 8-bit uint.
        Gfx::CTexturePtr Temp_RectImg_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc_DispImg_Rect);

        Gfx::STextureDescriptor TextureDesc_DepthImg_Rect = TextureDesc_DispImg_Rect;
        TextureDesc_DepthImg_Rect.m_Format = Gfx::CTexture::R16_UINT; // 1 channels with 16-bit uint.
        m_DepthImg_Rect_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc_DepthImg_Rect);

        if (m_Is_Scaling)
        {

            /*

            Strategy Design

            * Add Hierarchical Structure
            
            * Option 1. LR Depth -> Up-Sampling by BiLinear -> Color Guided FGS -> HR Depth

            * Option 2. LR Depth -> Up-Sampling by Color Guided Sparse-Input FGI -> HR Depth

            * Option 3. Fusion HR Depth from Option 1. & Option 2.
                * Still need to think about the strategy (Research Contribution !!!)
                * Consensus Check + Weighted Average / Weighted Least Square

            */

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

                m_FGI_UpSampler.FGS(m_DispImg_Rect_TexturePtr, m_Disp_HR_BiLinear_TexturePtr, Temp_RectImg_TexturePtr);
            }
            
        }
        else
        {
            Base::AABB2UInt TargetRect;
            TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_RectImg_Curt.get_ImgSize()));
            Gfx::TextureManager::CopyToTexture2D(m_DispImg_Rect_TexturePtr, TargetRect, m_RectImg_Curt.get_ImgSize().x, static_cast<const void*>(m_DispImg_Rect.data()));
        }

        //---GPU Start---
        Gfx::Performance::BeginEvent("Disparity to Depth");

        // Submit Data to Managers
        SParallaxEquation ParaxEqInfo;
        ParaxEqInfo.m_BaselineLength = glm::l2Norm(m_RectImg_Curt.get_PC() - m_RectImg_Last.get_PC());
        ParaxEqInfo.m_FocalLength = m_RectImg_Curt.get_Cam()[0].x;

        Gfx::BufferManager::UploadBufferData(m_ParaxEq_BufferPtr, &ParaxEqInfo);

        // Connecting Managers (@CPU) & GLSL (@GPU)
        Gfx::ContextManager::SetShaderCS(m_Disp2Depth_CSPtr);
        Gfx::ContextManager::SetImageTexture(0, m_DispImg_Rect_TexturePtr);
        Gfx::ContextManager::SetImageTexture(1, m_DepthImg_Rect_TexturePtr);
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
        Gfx::ContextManager::SetImageTexture(0, m_DepthImg_Rect_TexturePtr);
        Gfx::ContextManager::SetImageTexture(1, m_OrigDepth_TexturePtr);
        Gfx::ContextManager::SetConstantBuffer(0, m_Homogrampy_BufferPtr);

        // Start GPU Parallel Processing
        const int WorkGroupsX = DivUp(m_OrigImgSize.x, TileSize_2D);
        const int WorkGroupsY = DivUp(m_OrigImgSize.y, TileSize_2D);

        Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        Gfx::ContextManager::ResetShaderCS();
        Gfx::ContextManager::ResetImageTexture(1);

        Gfx::Performance::EndEvent();
        // GPU End

        const auto MemSize = m_OrigDepth_TexturePtr->GetNumberOfPixelsU() * m_OrigDepth_TexturePtr->GetNumberOfPixelsV() * sizeof(uint16_t);
        m_DepthImg_Orig.resize(MemSize);
        Gfx::TextureManager::CopyTextureToCPU(m_OrigDepth_TexturePtr, reinterpret_cast<char*>(m_DepthImg_Orig.data()));
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

        cv::Mat cvDepthImg_Orig(m_OrigImg_Curt.get_ImgSize().y, m_OrigImg_Curt.get_ImgSize().x, CV_16UC1);

        MemCpySize = static_cast<int>(m_DepthImg_Orig.size() * sizeof(m_DepthImg_Orig[0]));
        memcpy(cvDepthImg_Orig.data, m_DepthImg_Orig.data(), MemCpySize);

        ExportStr = "E:\\Project_ARCHITECT\\DepthImg_Orig_" + std::to_string(m_KeyfID) + ".png";
        cv::imwrite(ExportStr, cvDepthImg_Orig);


        cv::Mat cvDepthImg_Sensor(m_OrigImg_Curt.get_ImgSize().y, m_OrigImg_Curt.get_ImgSize().x, CV_16UC1);

        MemCpySize = static_cast<int>(m_DepthImg_Sensor.size() * sizeof(m_DepthImg_Sensor[0]));
        memcpy(cvDepthImg_Sensor.data, m_DepthImg_Sensor.data(), MemCpySize);

        ExportStr = "E:\\Project_ARCHITECT\\DepthImg_Sensor_" + std::to_string(m_KeyfID) + ".png";
        cv::imwrite(ExportStr, cvDepthImg_Sensor);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnStart()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin started!");

        //---00 Input Data---

        //---00 Select Keyframe---
        m_SelectKeyf_BaseLineL = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_keyframe:baseline_length", 0.03f); // Unit = meter

        //---01 Calculate Disparity-----
        m_Strategy = Core::CProgramParameters::GetInstance().Get("mr:stereo:01_disparity:strategy", "normal");

        m_EpiImgSize = Core::CProgramParameters::GetInstance().Get("mr:stereo:01_disparity:epipolar_image_size", glm::ivec3(256, 256, 1));

        // *** OLD ***

        //---02 Stereo Matching---
        m_StereoMatching_Method = Core::CProgramParameters::GetInstance().Get("mr:stereo:02_stereo_matching:method", "LibSGM");
        m_StereoMatching_Mode = Core::CProgramParameters::GetInstance().Get("mr:stereo:02_stereo_matching:mode", "Original");

        m_DispRange = Core::CProgramParameters::GetInstance().Get("mr:stereo:02_stereo_matching:disparity_range", 128);

        //---03 Disparity to Depth in Rectified Current Image---
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
        ParaxEq_BufferDesc.m_pClassKey = 0;
        m_ParaxEq_BufferPtr = Gfx::BufferManager::CreateBuffer(ParaxEq_BufferDesc);

        //---04 Depth from Rectified to Original Current Image---
        m_Depth_Rect2Orig_CSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/cs_Depth_Rect2Orig.glsl", "main", DefineString.c_str());

        Gfx::SBufferDescriptor Homography_BufferDesc = {};
        Homography_BufferDesc.m_Stride = 0;
        Homography_BufferDesc.m_Usage = Gfx::CBuffer::GPURead;
        Homography_BufferDesc.m_Binding = Gfx::CBuffer::ConstantBuffer;
        Homography_BufferDesc.m_Access = Gfx::CBuffer::CPUWrite;
        Homography_BufferDesc.m_NumberOfBytes = sizeof(FutoGCV::SHomography);
        Homography_BufferDesc.m_pBytes = nullptr;
        Homography_BufferDesc.m_pClassKey = 0;
        m_Homogrampy_BufferPtr = Gfx::BufferManager::CreateBuffer(Homography_BufferDesc);

        //---05 Compare Depth between plugin_stereo & Sensor---
        m_Is_CompareDepth = Core::CProgramParameters::GetInstance().Get("mr:stereo:05_depth_analysis:compare_sensor", true);

        m_Compare_Depth_CSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/cs_Compare_Depth.glsl", "main", DefineString.c_str());

        //---06 Return Results---
        m_IsExport_OrigImg = Core::CProgramParameters::GetInstance().Get("mr:stereo:06_output:export_orig_img", true);
        m_IsExport_EpiImg = Core::CProgramParameters::GetInstance().Get("mr:stereo:06_output:export_rect_img", true);
        m_IsExport_Depth = Core::CProgramParameters::GetInstance().Get("mr:stereo:06_output:export_depth", true);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        //---00 Input Data---
        m_OrigImg_TexturePtr = nullptr;

        // *** OLD ***
        //---Release Manager---
        m_Disp2Depth_CSPtr = nullptr;
        m_DispImg_Rect_TexturePtr = nullptr;
        m_DepthImg_Rect_TexturePtr = nullptr;
        m_ParaxEq_BufferPtr = nullptr;

        m_Depth_Rect2Orig_CSPtr = nullptr;
        m_OrigDepth_TexturePtr = nullptr;
        m_Homogrampy_BufferPtr = nullptr;

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