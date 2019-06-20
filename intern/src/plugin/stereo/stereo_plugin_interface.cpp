
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
        Gfx::STextureDescriptor TextureDesc_DepthDiff = TextureDesc_DepthImg_Orig;
        TextureDesc_DepthDiff.m_Format = Gfx::CTexture::R16_INT; // 1 channels with 16-bit int.
        m_Depth_Difference_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc_DepthDiff);
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
        
        //---Only Compute 2 Keyframes Once---
        if (!m_Is_KeyFrame)
        {
            m_OrigImg_Curt = FutoGmtCV::CFutoImg(_rRGBImage, m_OrigImgSize, 4, CamMtx, RotMtx, PCVec);
            m_Is_KeyFrame = true;
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

            m_KeyFrameID++;

            if (m_Is_ExportOrigImg)
            {
                export_OrigImg();
            }

            //---Epipolarizytion---
            // * Original Image Pair => Rectified Image Pair

            m_Rectifier_Planar = FutoGmtCV::CPlanarRectification(m_OrigImg_Curt, m_OrigImg_Last); // Apply Planar Rectification
            m_Rectifier_Planar.execute(m_RectImg_Curt, m_RectImg_Last, m_Homo_Curt, m_Homo_Last);

            if (m_Is_ExportRectImg)
            {
                export_RectImg();
            }

            if (m_RectImg_Curt.get_ImgSize().x > 2500 || m_RectImg_Curt.get_ImgSize().y > 2500)
            {
                return; // LibSGM will break if the size of rectified image is too large.
            }

            //---Stereo Matching---
            // * Calculate Disparity in Rectified Current Image

            m_DispImg_Rect.resize(m_RectImg_Curt.get_Img().size(), 0.0);

            const clock_t Time_SM_begin = clock();

            imp_StereoMatching_Tile();
            //imp_StereoMatching();

            const clock_t Time_SM_end = clock();

            float CalTime_SM = float(Time_SM_end - Time_SM_begin) / CLOCKS_PER_SEC;

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

            //---Return Depth to plugin_slam---
            const int MemCpySize = m_DepthImg_Orig_TexturePtr->GetNumberOfPixelsU() * m_DepthImg_Orig_TexturePtr->GetNumberOfPixelsV() * sizeof(uint16_t);
            std::vector<char> DepthImage(MemCpySize);
            Gfx::TextureManager::CopyTextureToCPU(m_DepthImg_Orig_TexturePtr, DepthImage.data());
            glm::mat4 Transform = glm::mat4(glm::transpose(m_OrigImg_Curt.get_Rot()));
            Transform[3] = glm::vec4(m_OrigImg_Curt.get_PC(), 1.0f);

            m_Delegate.Notify(m_OrigImg_Curt.get_Img(), DepthImage, Transform);

            if (m_Is_ExportDepth)
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

    void CPluginInterface::imp_StereoMatching_Tile()
    {
        /*
        Step 1. Determine Tile Size.
        Step 2. Calculate Tile Numbers.
        Step 3. Extract Tile from Whole Image.
            Per Tile: 1D Whole Image -> 2D Kernal / 2D Tile  -> 1D Tile
            Multi-Tile = Move Kernal
        Step 4. Processing
        Step 5. Combine Tiled Result back to Whole Image.
        */

        auto Tile_Size = m_OrigImgSize.x < m_OrigImgSize.y ? m_OrigImgSize.x : m_OrigImgSize.y;
        std::vector<char> TileImg_Curt(Tile_Size * Tile_Size, 0), TileImg_Last(Tile_Size * Tile_Size, 0);
        std::vector<uint16_t> TileDisp_Curt(Tile_Size * Tile_Size, 0);
        m_pStereoMatcher_LibSGM = std::make_unique<sgm::StereoSGM>(Tile_Size, Tile_Size, m_DispRange, 8, 16, sgm::EXECUTE_INOUT_HOST2HOST);

        glm::uvec2 Tile_Num(m_RectImg_Curt.get_ImgSize().x / Tile_Size, m_RectImg_Curt.get_ImgSize().y / Tile_Size);
            // If RectImgSize % TileSize != 0  =>  Processing the remainings individually.

        for (auto idx_TileNum_y = 0; idx_TileNum_y < Tile_Num.y; idx_TileNum_y++)
        {
            for (auto idx_TileNum_x = 0; idx_TileNum_x < Tile_Num.x; idx_TileNum_x++)
            {
                for (auto idx_Tile_Pix_y = 0; idx_Tile_Pix_y < Tile_Size; idx_Tile_Pix_y++)
                {
                    for (auto idx_Tile_Pix_x = 0; idx_Tile_Pix_x < Tile_Size; idx_Tile_Pix_x++)
                    {
                        const auto TilePos_Tile = idx_Tile_Pix_x + idx_Tile_Pix_y * Tile_Size;
                        const auto TilePos_Img = idx_Tile_Pix_x + idx_Tile_Pix_y * m_RectImg_Curt.get_ImgSize().x;
                        const auto ImgPos = TilePos_Img + idx_TileNum_x * Tile_Size + idx_TileNum_y * Tile_Size * m_RectImg_Curt.get_ImgSize().x;

                        TileImg_Curt.at(TilePos_Tile) = m_RectImg_Curt.get_Img().at(ImgPos);
                        TileImg_Last.at(TilePos_Tile) = m_RectImg_Last.get_Img().at(ImgPos);
                    }
                }

                cv::Mat cvTileImg_Curt(Tile_Size, Tile_Size, CV_8UC1);
                memcpy(cvTileImg_Curt.data, TileImg_Curt.data(), TileImg_Curt.size() * sizeof(TileImg_Curt[0]));
                cv::imshow("TileImg_Curt", cvTileImg_Curt);

                cv::Mat cvTileImg_Last(Tile_Size, Tile_Size, CV_8UC1);
                memcpy(cvTileImg_Last.data, TileImg_Last.data(), TileImg_Last.size() * sizeof(TileImg_Last[0]));
                cv::imshow("TileImg_Last", cvTileImg_Last);

                cv::waitKey();

                m_pStereoMatcher_LibSGM->execute(TileImg_Curt.data(), TileImg_Last.data(), TileDisp_Curt.data());

                for (auto idx_Tile_Pix_y = 0; idx_Tile_Pix_y < Tile_Size; idx_Tile_Pix_y++)
                {
                    for (auto idx_Tile_Pix_x = 0; idx_Tile_Pix_x < Tile_Size; idx_Tile_Pix_x++)
                    {
                        const auto TilePos_Tile = idx_Tile_Pix_x + idx_Tile_Pix_y * Tile_Size;
                        const auto TilePos_Img = idx_Tile_Pix_x + idx_Tile_Pix_y * m_RectImg_Curt.get_ImgSize().x;
                        const auto ImgPos = TilePos_Img + idx_TileNum_x * Tile_Size + idx_TileNum_y * Tile_Size * m_RectImg_Curt.get_ImgSize().x;

                        m_DispImg_Rect.at(ImgPos) = TileDisp_Curt.at(TilePos_Tile);
                    }
                }
            }
        }

        if (m_RectImg_Curt.get_ImgSize().x % Tile_Size)
        {
        }

        if (m_RectImg_Curt.get_ImgSize().y % Tile_Size)
        {
        }
    }

    void CPluginInterface::imp_StereoMatching()
    {
        if (m_StereoMatching_Method == "LibSGM")
        {
            m_pStereoMatcher_LibSGM = std::make_unique<sgm::StereoSGM>(m_RectImg_Curt.get_ImgSize().x, m_RectImg_Curt.get_ImgSize().y, m_DispRange, 8, 16, sgm::EXECUTE_INOUT_HOST2HOST);
                // Default disparity is pixel level => Disparity is the same in 8-bit & 16-bit.
                // If turn on sub-pixel => Output disparity must be 16-bit. => Divided by 16 to derive true disparity!!!

            std::vector<uint16_t> DispImg_Rect_uint16(m_RectImg_Curt.get_Img().size(), 0.0);

            m_pStereoMatcher_LibSGM->execute(m_RectImg_Curt.get_Img().data(), m_RectImg_Last.get_Img().data(), DispImg_Rect_uint16.data());

            m_DispImg_Rect = std::vector<float>::vector(DispImg_Rect_uint16.begin(), DispImg_Rect_uint16.end());
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

            const int cvMemCpySize = cvDispImg_Rect_cpu.cols * cvDispImg_Rect_cpu.rows * cvDispImg_Rect_cpu.elemSize();
            memcpy(m_DispImg_Rect.data(), cvDispImg_Rect_cpu.data, cvMemCpySize);

        }
    }

    void CPluginInterface::imp_Disp2Depth()
    {
        //---Initialize Texture Manager for Disparity in Rectified Image---
        Gfx::STextureDescriptor TextureDesc_DispImg_Rect = {};
        TextureDesc_DispImg_Rect.m_NumberOfPixelsU = m_RectImg_Curt.get_ImgSize().x;
        TextureDesc_DispImg_Rect.m_NumberOfPixelsV = m_RectImg_Last.get_ImgSize().y;
        TextureDesc_DispImg_Rect.m_NumberOfPixelsW = 1;
        TextureDesc_DispImg_Rect.m_NumberOfMipMaps = 1;
        TextureDesc_DispImg_Rect.m_NumberOfTextures = 1;
        TextureDesc_DispImg_Rect.m_Binding = Gfx::CTexture::ShaderResource;
        TextureDesc_DispImg_Rect.m_Access = Gfx::CTexture::EAccess::CPUWrite;
        TextureDesc_DispImg_Rect.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
        TextureDesc_DispImg_Rect.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextureDesc_DispImg_Rect.m_Format = Gfx::CTexture::R32_FLOAT; // 1 channels with 32-bit float.

        m_DispImg_Rect_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc_DispImg_Rect);

        Gfx::STextureDescriptor TextureDesc_DepthImg_Rect = TextureDesc_DispImg_Rect;
        TextureDesc_DepthImg_Rect.m_Format = Gfx::CTexture::R16_UINT; // 1 channels with 16-bit uint.
        m_DepthImg_Rect_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDesc_DepthImg_Rect);

        //---GPU Start---
        Gfx::Performance::BeginEvent("Disparity to Depth");

        // Submit Data to Managers
        SParallaxEquation ParaxEqInfo;
        ParaxEqInfo.m_BaselineLength = glm::l2Norm(m_RectImg_Curt.get_PC() - m_RectImg_Last.get_PC());
        ParaxEqInfo.m_FocalLength = m_RectImg_Curt.get_Cam()[0].x;

        Gfx::BufferManager::UploadBufferData(m_ParaxEq_BufferPtr, &ParaxEqInfo);

        Base::AABB2UInt TargetRect;
        TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_RectImg_Curt.get_ImgSize().x, m_RectImg_Curt.get_ImgSize().y));
        Gfx::TextureManager::CopyToTexture2D(m_DispImg_Rect_TexturePtr, TargetRect, m_RectImg_Curt.get_ImgSize().x, static_cast<const void*>(m_DispImg_Rect.data()));

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

        const uint MemSize = m_DepthImg_Orig_TexturePtr->GetNumberOfPixelsU() * m_DepthImg_Orig_TexturePtr->GetNumberOfPixelsV() * sizeof(uint16_t);
        m_DepthImg_Orig.resize(MemSize);
        Gfx::TextureManager::CopyTextureToCPU(m_DepthImg_Orig_TexturePtr, m_DepthImg_Orig.data());
        glm::mat4 Transform = glm::mat4(glm::transpose(m_OrigImg_Curt.get_Rot()));
        Transform[3] = glm::vec4(m_OrigImg_Curt.get_PC(), 1.0f);

        m_Delegate.Notify(m_OrigImg_Curt.get_Img(), m_DepthImg_Orig, Transform);
    }

    void CPluginInterface::cmp_Depth()
    {
        Base::AABB2UInt TargetRect;
        TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_OrigImgSize.x, m_OrigImgSize.y));
        Gfx::TextureManager::CopyToTexture2D(m_DepthImg_Sensor_TexturePtr, TargetRect, m_OrigImgSize.x, static_cast<const void*>(m_DepthImg_Sensor.data()));

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
        std::string ExportStr;
        uint MemCpySize = 0;

        cv::Mat cvOrigImg_Curt(m_OrigImgSize.y, m_OrigImgSize.x, CV_8UC4);

        MemCpySize = m_OrigImg_Curt.get_Img().size() * sizeof(m_OrigImg_Curt.get_Img()[0]);
        memcpy(cvOrigImg_Curt.data, m_OrigImg_Curt.get_Img().data(), MemCpySize);

        cv::cvtColor(cvOrigImg_Curt, cvOrigImg_Curt, cv::COLOR_BGRA2RGBA); // Transform to RGB before imshow & imwrite

        ExportStr = "E:\\Project_ARCHITECT\\OrigImg_Curt_" + std::to_string(m_KeyFrameID) + ".png";
        cv::imwrite(ExportStr, cvOrigImg_Curt);
    }

    void CPluginInterface::export_RectImg()
    {
        std::string ExportStr;
        uint MemCpySize = 0;

        cv::Mat cvRectImg_Curt(m_RectImg_Curt.get_ImgSize().y, m_RectImg_Curt.get_ImgSize().x, CV_8UC1);
        memcpy(cvRectImg_Curt.data, m_RectImg_Curt.get_Img().data(), m_RectImg_Curt.get_Img().size());

        ExportStr = "E:\\Project_ARCHITECT\\RectImg_Curt_" + std::to_string(m_KeyFrameID) + ".png";
        cv::imwrite(ExportStr, cvRectImg_Curt);


        cv::Mat cvRectImg_Last(m_RectImg_Last.get_ImgSize().y, m_RectImg_Last.get_ImgSize().x, CV_8UC1);
        memcpy(cvRectImg_Last.data, m_RectImg_Last.get_Img().data(), m_RectImg_Last.get_Img().size());

        ExportStr = "E:\\Project_ARCHITECT\\RectImg_Last_" + std::to_string(m_KeyFrameID) + ".png";
        cv::imwrite(ExportStr, cvRectImg_Last);
    }

    void CPluginInterface::export_Depth()
    {
        std::string ExportStr;
        uint MemCpySize = 0;

        cv::Mat cvDepthImg_Orig(m_OrigImg_Curt.get_ImgSize().y, m_OrigImg_Curt.get_ImgSize().x, CV_16UC1);

        MemCpySize = m_DepthImg_Orig.size() * sizeof(m_DepthImg_Orig[0]);
        memcpy(cvDepthImg_Orig.data, m_DepthImg_Orig.data(), MemCpySize);

        ExportStr = "E:\\Project_ARCHITECT\\DepthImg_Orig_" + std::to_string(m_KeyFrameID) + ".png";
        cv::imwrite(ExportStr, cvDepthImg_Orig);


        cv::Mat cvDepthImg_Sensor(m_OrigImg_Curt.get_ImgSize().y, m_OrigImg_Curt.get_ImgSize().x, CV_16UC1);

        MemCpySize = m_DepthImg_Sensor.size() * sizeof(m_DepthImg_Sensor[0]);
        memcpy(cvDepthImg_Sensor.data, m_DepthImg_Sensor.data(), MemCpySize);

        ExportStr = "E:\\Project_ARCHITECT\\DepthImg_Sensor_" + std::to_string(m_KeyFrameID) + ".png";
        cv::imwrite(ExportStr, cvDepthImg_Sensor);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnStart()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin started!");

        //---00 Input---
        m_FrameResolution = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_input:frame_resolution", 0.5); // Full = 1; Half = 0.5;

        //---00 Keyframe---
        m_Cdt_Keyf_BaseLineL = Core::CProgramParameters::GetInstance().Get("mr:stereo:00_Keyframe:baseline_length", 0.03); // Unit = meter

        m_Is_KeyFrame = false;

        //---01 Rectification-----

        //---02 Stereo Matching---
        m_StereoMatching_Method = Core::CProgramParameters::GetInstance().Get("mr:stereo:02_stereo_matching:Method", "LibSGM");

        m_DispRange = Core::CProgramParameters::GetInstance().Get("mr:stereo:02_stereo_matching:disparity_range", 128);

        //---03 Disparity to Depth in Rectified Current Image---
        std::stringstream DefineStream;
        DefineStream
            << "#define TILE_SIZE_2D " << TileSize_2D << " \n";
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

        //---04 Depth from Rectified to Original Current Image---
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

        //---05 Compare Depth between plugin_stereo & Sensor---
        m_Is_CompareDepth = Core::CProgramParameters::GetInstance().Get("mr:stereo:05_depth_analysis:compare_sensor", true);

        m_Compare_Depth_CSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/cs_Compare_Depth.glsl", "main", DefineString.c_str());

        //---06 Return Results---
        m_Is_ExportOrigImg = Core::CProgramParameters::GetInstance().Get("mr:stereo:06_output:export_orig_img", true);
        m_Is_ExportRectImg = Core::CProgramParameters::GetInstance().Get("mr:stereo:06_output:export_rect_img", true);
        m_Is_ExportDepth = Core::CProgramParameters::GetInstance().Get("mr:stereo:06_output:export_depth", true);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        //---Release Manager---
        m_Disp2Depth_CSPtr = nullptr;
        m_DispImg_Rect_TexturePtr = nullptr;
        m_DepthImg_Rect_TexturePtr = nullptr;
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