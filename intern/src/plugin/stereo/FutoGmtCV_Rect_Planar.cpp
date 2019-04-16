#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\FutoGmtCV_Rect_Planar.h"

#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_performance.h"

namespace // No specific namespace => Only allowed to use in this page.
{
    int DivUp(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }
}

namespace FutoGmtCV
{
    //---Constructors & Destructor---
    PlanarRect::PlanarRect()
    {
        std::stringstream DefineStream;

        DefineStream
            << "#define TILE_SIZE_2D " << 16 << " \n"; // It is suggested to use 16 for 2D image (based on experience).

        std::string DefineString = DefineStream.str();

        m_CSPtr_PlanarRecr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/cs_Rect_Planar.glsl", "main", DefineString.c_str());

        Gfx::SBufferDescriptor BufferDesc = {};

        BufferDesc.m_Stride = 0;
        BufferDesc.m_Usage = Gfx::CBuffer::GPURead;
        BufferDesc.m_Binding = Gfx::CBuffer::ConstantBuffer;
        BufferDesc.m_Access = Gfx::CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(glm::mat4);
        BufferDesc.m_pBytes = nullptr;
        BufferDesc.m_pClassKey = 0;

        m_BufferPtr_Homography = Gfx::BufferManager::CreateBuffer(BufferDesc);
    }


    PlanarRect::~PlanarRect()
    {
        m_CSPtr_PlanarRecr = nullptr;
        m_TexturePtr_OrigImg = nullptr;
        m_TexturePtr_RectImg = nullptr;
    }

    //---Execution Functions---
    void PlanarRect::imp_PlanarRect()
    {
        //---Compute Orientations of Rectified Images---
        cal_K_Rect();
        cal_R_Rect();
        cal_P_Rect();
        cal_H();

        //---Center Rectified Images---
        imp_CenterRectImg();
        cal_H();

        //---Generate Rectified Images---
        determ_RectImgSize();

    }

    //---Assistant Functions: Compute Orientations---
    void PlanarRect::cal_K_Rect()
    {
        m_K_Rect_B = 0.5 * (m_Img_Orig_B.get_Cam() + m_Img_Orig_M.get_Cam());
        m_K_Rect_B.ptr<float>(0)[1] = 0; // Let skew = 0
        m_K_Rect_M = 0.5 * (m_Img_Orig_B.get_Cam() + m_Img_Orig_M.get_Cam());
        m_K_Rect_M.ptr<float>(0)[1] = 0; // Let skew = 0
    }

    void PlanarRect::cal_R_Rect()
    {
        m_R_Rect = cv::Mat::eye(3, 3, CV_32F);

        cv::Mat R_Rect_row0 = m_Img_Orig_M.get_PC() - m_Img_Orig_B.get_PC();
        if (cv::sum(R_Rect_row0)[0] < 0)
        {
            R_Rect_row0 *= -1; // Make the rotation always left2right and up2down.
        }

        R_Rect_row0 /= cv::norm(R_Rect_row0, cv::NORM_L2);
        cv::transpose(R_Rect_row0, R_Rect_row0);

        cv::Mat R_Rect_row1 = m_Img_Orig_B.get_Rot().row(2).cross(R_Rect_row0);
        R_Rect_row1 /= cv::norm(R_Rect_row1, cv::NORM_L2);

        cv::Mat R_Rect_row2 = R_Rect_row0.cross(R_Rect_row1);
        R_Rect_row2 /= cv::norm(R_Rect_row2, cv::NORM_L2);

        R_Rect_row0.copyTo(m_R_Rect.row(0));
        R_Rect_row1.copyTo(m_R_Rect.row(1));
        R_Rect_row2.copyTo(m_R_Rect.row(2));
    }

    void PlanarRect::cal_P_Rect()
    {
        cv::Mat Trans_Rect_B(3, 4, CV_32F);
        m_R_Rect.colRange(0, 3).copyTo(Trans_Rect_B.colRange(0, 3)); // StartCol is inclusive while EndCol is exclusive
        cv::Mat t_Orig_B = -m_R_Rect * m_Img_Orig_B.get_PC();
        t_Orig_B.col(0).copyTo(Trans_Rect_B.col(3));

        cv::Mat Trans_Rect_M(3, 4, CV_32F);
        m_R_Rect.colRange(0, 3).copyTo(Trans_Rect_M.colRange(0, 3)); // StartCol is inclusive while EndCol is exclusive
        cv::Mat t_Orig_M = -m_R_Rect * m_Img_Orig_M.get_PC();
        t_Orig_M.col(0).copyTo(Trans_Rect_M.col(3));

        m_P_Rect_B = m_K_Rect_B * Trans_Rect_B;
        m_P_Rect_M = m_K_Rect_M * Trans_Rect_M;
    }

    void PlanarRect::cal_H()
    {
        //---Calculate the Homography---
        m_Homo_B = m_P_Rect_B * m_Img_Orig_B.get_P_mtx().inv(cv::DECOMP_SVD);
        m_Homo_M = m_P_Rect_M * m_Img_Orig_M.get_P_mtx().inv(cv::DECOMP_SVD);
        //---Another Homography Transformation used in Fusiello's Code. -> I do not like it because it is not reasonable
        /*
        //H_B = P_Rect_B.colRange(0, 3) * m_Img_Orig_B.get_P_mtx().colRange(0, 3).inv();
        //H_M = P_Rect_M.colRange(0, 3) * m_Img_Orig_M.get_P_mtx().colRange(0, 3).inv();
        */
        //---

    }

    //---Assistant Functions: Center Rectified Images---
    void PlanarRect::imp_CenterRectImg()
    {
        cv::Mat CenterB_Orig = cv::Mat::ones(3, 1, CV_32F);
        CenterB_Orig.ptr<float>(0)[0] = m_Img_Orig_B.get_Img().size().width / 2;
        CenterB_Orig.ptr<float>(1)[0] = m_Img_Orig_B.get_Img().size().height / 2;
        cv::Mat CenterM_Orig = cv::Mat::ones(3, 1, CV_32F);
        CenterM_Orig.ptr<float>(0)[0] = m_Img_Orig_M.get_Img().size().width / 2;
        CenterM_Orig.ptr<float>(1)[0] = m_Img_Orig_M.get_Img().size().height / 2;

        cv::Mat CenterB_Orig2Rect = m_Homo_B * CenterB_Orig;
        CenterB_Orig2Rect /= CenterB_Orig2Rect.ptr<float>(2)[0];
        cv::Mat CenterM_Orig2Rect = m_Homo_M * CenterM_Orig;
        CenterM_Orig2Rect /= CenterM_Orig2Rect.ptr<float>(2)[0];

        cv::Mat Drift_B = CenterB_Orig - CenterB_Orig2Rect;
        cv::Mat Drift_M = CenterM_Orig - CenterM_Orig2Rect;
        float Drift_y = (Drift_B.ptr<float>(1)[0] + Drift_M.ptr<float>(1)[0]) / 2;
        Drift_B.ptr<float>(1)[0] = Drift_y;
        Drift_M.ptr<float>(1)[0] = Drift_y;

        m_K_Rect_B.ptr<float>(0)[2] += Drift_B.ptr<float>(0)[0];
        m_K_Rect_B.ptr<float>(1)[2] += Drift_B.ptr<float>(1)[0];
        m_K_Rect_M.ptr<float>(0)[2] += Drift_M.ptr<float>(0)[0];
        m_K_Rect_M.ptr<float>(1)[2] += Drift_M.ptr<float>(1)[0];
    }

    //---Assistant Functions: Generate Rectified Images---
    void PlanarRect::determ_RectImgSize()
    {
        //---Select Img Corner in Originals---
        cv::Mat ImgCnr_UL_OrigB = cv::Mat::ones(3, 1, CV_32F);
        ImgCnr_UL_OrigB.ptr<float>(0)[0] = 0;
        ImgCnr_UL_OrigB.ptr<float>(1)[0] = 0;
        cv::Mat ImgCnr_UR_OrigB = cv::Mat::ones(3, 1, CV_32F);
        ImgCnr_UR_OrigB.ptr<float>(0)[0] = m_Img_Orig_B.get_Img().size().width;
        ImgCnr_UR_OrigB.ptr<float>(1)[0] = 0;
        cv::Mat ImgCnr_DL_OrigB = cv::Mat::ones(3, 1, CV_32F);
        ImgCnr_DL_OrigB.ptr<float>(0)[0] = 0;
        ImgCnr_DL_OrigB.ptr<float>(1)[0] = m_Img_Orig_B.get_Img().size().height;
        cv::Mat ImgCnr_DR_OrigB = cv::Mat::ones(3, 1, CV_32F);
        ImgCnr_DR_OrigB.ptr<float>(0)[0] = m_Img_Orig_B.get_Img().size().width;
        ImgCnr_DR_OrigB.ptr<float>(1)[0] = m_Img_Orig_B.get_Img().size().height;

        cv::Mat ImgCnr_UL_OrigM = cv::Mat::ones(3, 1, CV_32F);
        ImgCnr_UL_OrigM.ptr<float>(0)[0] = 0;
        ImgCnr_UL_OrigM.ptr<float>(1)[0] = 0;
        cv::Mat ImgCnr_UR_OrigM = cv::Mat::ones(3, 1, CV_32F);
        ImgCnr_UR_OrigM.ptr<float>(0)[0] = m_Img_Orig_M.get_Img().size().width;
        ImgCnr_UR_OrigM.ptr<float>(1)[0] = 0;
        cv::Mat ImgCnr_DL_OrigM = cv::Mat::ones(3, 1, CV_32F);
        ImgCnr_DL_OrigM.ptr<float>(0)[0] = 0;
        ImgCnr_DL_OrigM.ptr<float>(1)[0] = m_Img_Orig_M.get_Img().size().height;
        cv::Mat ImgCnr_DR_OrigM = cv::Mat::ones(3, 1, CV_32F);
        ImgCnr_DR_OrigM.ptr<float>(0)[0] = m_Img_Orig_M.get_Img().size().width;
        ImgCnr_DR_OrigM.ptr<float>(1)[0] = m_Img_Orig_M.get_Img().size().height;

        //---Transform Img Corner from Originals to Rectified---
        cv::Mat ImgCnr_UL_Orig2Rect_B = m_Homo_B * ImgCnr_UL_OrigB;
        ImgCnr_UL_Orig2Rect_B /= ImgCnr_UL_Orig2Rect_B.ptr<float>(2)[0];
        cv::Mat ImgCnr_UR_Orig2Rect_B = m_Homo_B * ImgCnr_UR_OrigB;
        ImgCnr_UR_Orig2Rect_B /= ImgCnr_UR_Orig2Rect_B.ptr<float>(2)[0];
        cv::Mat ImgCnr_DL_Orig2Rect_B = m_Homo_B * ImgCnr_DL_OrigB;
        ImgCnr_DL_Orig2Rect_B /= ImgCnr_DL_Orig2Rect_B.ptr<float>(2)[0];
        cv::Mat ImgCnr_DR_Orig2Rect_B = m_Homo_B * ImgCnr_DR_OrigB;
        ImgCnr_DR_Orig2Rect_B /= ImgCnr_DR_Orig2Rect_B.ptr<float>(2)[0];

        cv::Mat ImgCnr_UL_Orig2Rect_M = m_Homo_M * ImgCnr_UL_OrigM;
        ImgCnr_UL_Orig2Rect_M /= ImgCnr_UL_Orig2Rect_M.ptr<float>(2)[0];
        cv::Mat ImgCnr_UR_Orig2Rect_M = m_Homo_M * ImgCnr_UR_OrigM;
        ImgCnr_UR_Orig2Rect_M /= ImgCnr_UR_Orig2Rect_M.ptr<float>(2)[0];
        cv::Mat ImgCnr_DL_Orig2Rect_M = m_Homo_M * ImgCnr_DL_OrigM;
        ImgCnr_DL_Orig2Rect_M /= ImgCnr_DL_Orig2Rect_M.ptr<float>(2)[0];
        cv::Mat ImgCnr_DR_Orig2Rect_M = m_Homo_M * ImgCnr_DR_OrigM;
        ImgCnr_DR_Orig2Rect_M /= ImgCnr_DR_Orig2Rect_M.ptr<float>(2)[0];

        //---Determine the Boundary of Epipolar Image---
        std::vector<float> ImgCnr_Orig2Rect_B_x = { ImgCnr_UL_Orig2Rect_B.ptr<float>(0)[0], ImgCnr_UR_Orig2Rect_B.ptr<float>(0)[0], ImgCnr_DL_Orig2Rect_B.ptr<float>(0)[0], ImgCnr_DR_Orig2Rect_B.ptr<float>(0)[0] };
        std::vector<float> ImgCnr_Orig2Rect_B_y = { ImgCnr_UL_Orig2Rect_B.ptr<float>(1)[0], ImgCnr_UR_Orig2Rect_B.ptr<float>(1)[0], ImgCnr_DL_Orig2Rect_B.ptr<float>(1)[0], ImgCnr_DR_Orig2Rect_B.ptr<float>(1)[0] };

        std::vector<float> ImgCnr_Orig2Rect_M_x = { ImgCnr_UL_Orig2Rect_M.ptr<float>(0)[0], ImgCnr_UR_Orig2Rect_M.ptr<float>(0)[0], ImgCnr_DL_Orig2Rect_M.ptr<float>(0)[0], ImgCnr_DR_Orig2Rect_M.ptr<float>(0)[0] };
        std::vector<float> ImgCnr_Orig2Rect_M_y = { ImgCnr_UL_Orig2Rect_M.ptr<float>(1)[0], ImgCnr_UR_Orig2Rect_M.ptr<float>(1)[0], ImgCnr_DL_Orig2Rect_M.ptr<float>(1)[0], ImgCnr_DR_Orig2Rect_M.ptr<float>(1)[0] };

        float ImgBd_RectB_x_min = *std::min_element(ImgCnr_Orig2Rect_B_x.begin(), ImgCnr_Orig2Rect_B_x.end());
        float ImgBd_RectB_y_min = *std::min_element(ImgCnr_Orig2Rect_B_y.begin(), ImgCnr_Orig2Rect_B_y.end());
        float ImgBd_RectB_x_max = *std::max_element(ImgCnr_Orig2Rect_B_x.begin(), ImgCnr_Orig2Rect_B_x.end());
        float ImgBd_RectB_y_max = *std::max_element(ImgCnr_Orig2Rect_B_y.begin(), ImgCnr_Orig2Rect_B_y.end());

        float ImgBd_RectM_x_min = *std::min_element(ImgCnr_Orig2Rect_M_x.begin(), ImgCnr_Orig2Rect_M_x.end());
        float ImgBd_RectM_y_min = *std::min_element(ImgCnr_Orig2Rect_M_y.begin(), ImgCnr_Orig2Rect_M_y.end());
        float ImgBd_RectM_x_max = *std::max_element(ImgCnr_Orig2Rect_M_x.begin(), ImgCnr_Orig2Rect_M_x.end());
        float ImgBd_RectM_y_max = *std::max_element(ImgCnr_Orig2Rect_M_y.begin(), ImgCnr_Orig2Rect_M_y.end());

        //---Determine the Size of Epipolar Imgs---
        int ImgBd_Rect_x_min = std::floor(std::min(ImgBd_RectB_x_min, ImgBd_RectM_x_min));
        int ImgBd_Rect_x_max = std::ceil(std::max(ImgBd_RectB_x_max, ImgBd_RectM_x_max));
        int ImgBd_Rect_y_min = std::floor(std::min(ImgBd_RectB_y_min, ImgBd_RectM_y_min));
        int ImgBd_Rect_y_max = std::ceil(std::max(ImgBd_RectB_y_max, ImgBd_RectM_y_max));

        m_ImgCnr_UL_Rect = cv::Point(ImgBd_Rect_x_min, ImgBd_Rect_y_min);
        m_ImgCnr_DR_Rect = cv::Point(ImgBd_Rect_x_max, ImgBd_Rect_y_max);
        m_ImgSize_Rect = cv::Size(m_ImgCnr_DR_Rect.x - m_ImgCnr_UL_Rect.x, m_ImgCnr_DR_Rect.y - m_ImgCnr_UL_Rect.y);


        //---Determine TextureDescriptor for GPU Output---
        Gfx::STextureDescriptor TextureDescriptor = {};

        TextureDescriptor.m_NumberOfPixelsU = m_ImgSize_Rect.width;
        TextureDescriptor.m_NumberOfPixelsV = m_ImgSize_Rect.height;
        TextureDescriptor.m_NumberOfPixelsW = 1;
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding = Gfx::CTexture::ShaderResource;
        TextureDescriptor.m_Access = Gfx::CTexture::EAccess::CPUWrite;
        TextureDescriptor.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
        TextureDescriptor.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextureDescriptor.m_Format = Gfx::CTexture::R8_UBYTE; // 1 channel with 8-bit 

        m_TexturePtr_RectImg = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);
    }

    void PlanarRect::genrt_RectImg()
    {
        //---Compute @ GPU---
        Gfx::Performance::BeginEvent("Planar Rectification");

        Gfx::STextureDescriptor TextureDescriptor = {};

        TextureDescriptor.m_NumberOfPixelsU = m_Img_Orig_B.get_Img().cols;
        TextureDescriptor.m_NumberOfPixelsV = m_Img_Orig_B.get_Img().rows;
        TextureDescriptor.m_NumberOfPixelsW = 1;
        TextureDescriptor.m_NumberOfMipMaps = 1;
        TextureDescriptor.m_NumberOfTextures = 1;
        TextureDescriptor.m_Binding = Gfx::CTexture::ShaderResource;
        TextureDescriptor.m_Access = Gfx::CTexture::EAccess::CPURead;
        TextureDescriptor.m_Usage = Gfx::CTexture::EUsage::GPUToCPU;
        TextureDescriptor.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextureDescriptor.m_Format = Gfx::CTexture::R8_UBYTE;
        TextureDescriptor.m_pPixels = m_Img_Orig_B.get_Img().data;

        m_TexturePtr_OrigImg = Gfx::TextureManager::CreateTexture2D(TextureDescriptor);

        Gfx::ContextManager::SetShaderCS(m_CSPtr_PlanarRecr);
        Gfx::ContextManager::SetImageTexture(0, m_TexturePtr_OrigImg);
        Gfx::ContextManager::SetImageTexture(1, m_TexturePtr_RectImg);
        Gfx::ContextManager::SetConstantBuffer(0, m_BufferPtr_Homography);

        Gfx::BufferManager::UploadBufferData(m_BufferPtr_Homography, &m_Homo_B, 0, sizeof(m_Homo_B));

        const int WorkGroupsX = DivUp(m_ImgSize_Rect.width, 16);
        const int WorkGroupsY = DivUp(m_ImgSize_Rect.height, 16);

        Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        Gfx::ContextManager::ResetShaderCS();

        cv::Mat GPUResult(m_TexturePtr_RectImg->GetNumberOfPixelsV(), m_TexturePtr_RectImg->GetNumberOfPixelsU(), CV_8UC1);
        Gfx::TextureManager::CopyTextureToCPU(m_TexturePtr_RectImg, reinterpret_cast<char*>(GPUResult.data));

        cv::imshow("GPU Result", GPUResult);

        Gfx::Performance::EndEvent();

        //---Compute @ CPU (Remove after finishing "Compute @ GPU")---

        //---Create Rectified Images---
        m_Img_Rect_B = cv::Mat(m_ImgSize_Rect, CV_8UC4);
        m_Img_Rect_M = cv::Mat(m_ImgSize_Rect, CV_8UC4);

        //---Build Look-Up Table: from Rectified to Originals---
        mapB_x_Rect2Orig = cv::Mat(m_ImgSize_Rect, CV_32FC1);
        mapB_y_Rect2Orig = cv::Mat(m_ImgSize_Rect, CV_32FC1);
        mapM_x_Rect2Orig = cv::Mat(m_ImgSize_Rect, CV_32FC1);
        mapM_y_Rect2Orig = cv::Mat(m_ImgSize_Rect, CV_32FC1);

        for (int idx_y = 0; idx_y < m_ImgSize_Rect.height; idx_y++)
        {
            for (int idx_x = 0; idx_x < m_ImgSize_Rect.width; idx_x++)
            {
                cv::Mat pix_Rect = cv::Mat::ones(3, 1, CV_32F);
                pix_Rect.ptr<float>(0)[0] = idx_x + m_ImgCnr_UL_Rect.x;
                pix_Rect.ptr<float>(1)[0] = idx_y + m_ImgCnr_UL_Rect.y;

                cv::Mat pixB_Rect2Orig = m_Homo_B.inv() * pix_Rect;
                pixB_Rect2Orig /= pixB_Rect2Orig.ptr<float>(2)[0];

                cv::Mat pixM_Rect2Orig = m_Homo_M.inv() * pix_Rect;
                pixM_Rect2Orig /= pixM_Rect2Orig.ptr<float>(2)[0];

                mapB_x_Rect2Orig.ptr<float>(idx_y)[idx_x] = pixB_Rect2Orig.ptr<float>(0)[0];
                mapB_y_Rect2Orig.ptr<float>(idx_y)[idx_x] = pixB_Rect2Orig.ptr<float>(1)[0];

                mapM_x_Rect2Orig.ptr<float>(idx_y)[idx_x] = pixM_Rect2Orig.ptr<float>(0)[0];
                mapM_y_Rect2Orig.ptr<float>(idx_y)[idx_x] = pixM_Rect2Orig.ptr<float>(1)[0];
            }
        }

        //---Derive Pixel Value of Rectified Images: Transform Pixels from Rectfied back to Origianls & Interpolation---
        cv::remap(m_Img_Orig_B.get_Img(), m_Img_Rect_B, mapB_x_Rect2Orig, mapB_y_Rect2Orig, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);
        cv::remap(m_Img_Orig_M.get_Img(), m_Img_Rect_M, mapM_x_Rect2Orig, mapM_y_Rect2Orig, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);


        //---Build Look-Up Table: from Originals to Rectified---
        mapB_x_Orig2Rect = cv::Mat(m_Img_Orig_B.get_Img().size(), CV_32FC1);
        mapB_y_Orig2Rect = cv::Mat(m_Img_Orig_B.get_Img().size(), CV_32FC1);
        mapM_x_Orig2Rect = cv::Mat(m_Img_Orig_M.get_Img().size(), CV_32FC1);
        mapM_y_Orig2Rect = cv::Mat(m_Img_Orig_M.get_Img().size(), CV_32FC1);

        for (int idx_y = 0; idx_y < m_Img_Orig_B.get_Img().size().height; idx_y++)
        {
            for (int idx_x = 0; idx_x < m_Img_Orig_B.get_Img().size().width; idx_x++)
            {
                cv::Mat pixB_Orig = cv::Mat::ones(3, 1, CV_32F);
                pixB_Orig.ptr<float>(0)[0] = idx_x;
                pixB_Orig.ptr<float>(1)[0] = idx_y;

                cv::Mat pixM_Orig = cv::Mat::ones(3, 1, CV_32F);
                pixM_Orig.ptr<float>(0)[0] = idx_x;
                pixM_Orig.ptr<float>(1)[0] = idx_y;

                cv::Mat pixB_Orig2Rect = m_Homo_B * pixB_Orig;
                pixB_Orig2Rect /= pixB_Orig2Rect.ptr<float>(2)[0];
                pixB_Orig2Rect.ptr<float>(0)[0] = pixB_Orig2Rect.ptr<float>(0)[0] - ImgBd_Rect_x_min;
                pixB_Orig2Rect.ptr<float>(1)[0] = pixB_Orig2Rect.ptr<float>(1)[0] - ImgBd_Rect_y_min;

                cv::Mat pixM_Orig2Rect = m_Homo_M * pixM_Orig;
                pixM_Orig2Rect /= pixM_Orig2Rect.ptr<float>(2)[0];
                pixM_Orig2Rect.ptr<float>(0)[0] = pixM_Orig2Rect.ptr<float>(0)[0] - ImgBd_Rect_x_min;
                pixM_Orig2Rect.ptr<float>(1)[0] = pixM_Orig2Rect.ptr<float>(1)[0] - ImgBd_Rect_y_min;

                mapB_x_Orig2Rect.ptr<float>(idx_y)[idx_x] = pixB_Orig2Rect.ptr<float>(0)[0];
                mapB_y_Orig2Rect.ptr<float>(idx_y)[idx_x] = pixB_Orig2Rect.ptr<float>(1)[0];

                mapM_x_Orig2Rect.ptr<float>(idx_y)[idx_x] = pixM_Orig2Rect.ptr<float>(0)[0];
                mapM_y_Orig2Rect.ptr<float>(idx_y)[idx_x] = pixM_Orig2Rect.ptr<float>(1)[0];
            }
        }
    }


    //=====OLD=====

    //---Generation of Rectified Img---
    

    //---Get Function---
    void PlanarRect::get_K_Rect(cv::Mat &CamB_Rect, cv::Mat &CamM_Rect)
    {
        CamB_Rect = m_K_Rect_B;
        CamM_Rect = m_K_Rect_M;
    }
    void PlanarRect::get_R_Rect(cv::Mat &Rot_Rect)
    {
        Rot_Rect = m_R_Rect;
    }

    void PlanarRect::get_RectImg(cv::Mat& Output_RectImgB, cv::Mat& Output_RectImgM)
    {
        Output_RectImgB = m_Img_Rect_B;
        Output_RectImgM = m_Img_Rect_M;
    }

    void PlanarRect::get_Transform_Orig2Rect(cv::Mat& LookUpTx_B_Orig2Rect, cv::Mat& LookUpTy_B_Orig2Rect, cv::Mat& LookUpTx_M_Orig2Rect, cv::Mat& LookUpTy_M_Orig2Rect)
    {
        LookUpTx_B_Orig2Rect = mapB_x_Orig2Rect;
        LookUpTy_B_Orig2Rect = mapB_y_Orig2Rect;
        LookUpTx_M_Orig2Rect = mapM_x_Orig2Rect;
        LookUpTy_M_Orig2Rect = mapM_y_Orig2Rect;
    }

} // FutoGmtCV
