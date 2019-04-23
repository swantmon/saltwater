#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\FutoGmtCV_Rect_Planar.h"

#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_performance.h"

namespace // No specific namespace => Only allowed to use in this page.
{
    //---Definition for GPU Parallel Processing---
    #define TileSize_2D 16

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
    }

    PlanarRect::PlanarRect(const cv::Size& OrigImgSize, const cv::Size& RectImgSize)
        : m_ImgSize_Orig(OrigImgSize),
          m_ImgSize_Rect(RectImgSize)
    {
        //---Initialize Shader Manager---
        std::stringstream DefineStream;

        DefineStream
            << "#define TILE_SIZE_2D " << TileSize_2D << " \n"; // 16 for work group size is suggested for 2D image (based on experience).

        std::string DefineString = DefineStream.str();

        m_CSPtr_PlanarRect = Gfx::ShaderManager::CompileCS("../../plugins/stereo/cs_Rect_Planar.glsl", "main", DefineString.c_str());

        //---Initialize Buffer Manager---
        Gfx::SBufferDescriptor BufferDesc = {};

        BufferDesc.m_Stride = 0;
        BufferDesc.m_Usage = Gfx::CBuffer::GPURead;
        BufferDesc.m_Binding = Gfx::CBuffer::ConstantBuffer;
        BufferDesc.m_Access = Gfx::CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(glm::mat4);
        BufferDesc.m_pBytes = nullptr;
        BufferDesc.m_pClassKey = 0;

        m_BufferPtr_Homography = Gfx::BufferManager::CreateBuffer(BufferDesc);

        //---Initialize Input Texture Manager---
        Gfx::STextureDescriptor TextureDescriptor_In = {};

        TextureDescriptor_In.m_NumberOfPixelsU = m_ImgSize_Orig.width;
        TextureDescriptor_In.m_NumberOfPixelsV = m_ImgSize_Orig.height;
        TextureDescriptor_In.m_NumberOfPixelsW = 1;
        TextureDescriptor_In.m_NumberOfMipMaps = 1;
        TextureDescriptor_In.m_NumberOfTextures = 1;
        TextureDescriptor_In.m_Binding = Gfx::CTexture::ShaderResource;
        TextureDescriptor_In.m_Access = Gfx::CTexture::EAccess::CPURead;
        TextureDescriptor_In.m_Usage = Gfx::CTexture::EUsage::GPUToCPU;
        TextureDescriptor_In.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextureDescriptor_In.m_Format = Gfx::CTexture::R8_UBYTE; // 1 channel with 8-bit. -> R8G8B8 = 3 channels with 8-bit.

        m_TexturePtr_OrigImg = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_In);

        //---Initialize Output Texture Manager---
        Gfx::STextureDescriptor TextureDescriptor_Out = {};

        TextureDescriptor_Out.m_NumberOfPixelsU = m_ImgSize_Rect.width;
        TextureDescriptor_Out.m_NumberOfPixelsV = m_ImgSize_Rect.height;
        TextureDescriptor_Out.m_NumberOfPixelsW = 1;
        TextureDescriptor_Out.m_NumberOfMipMaps = 1;
        TextureDescriptor_Out.m_NumberOfTextures = 1;
        TextureDescriptor_Out.m_Binding = Gfx::CTexture::ShaderResource;
        TextureDescriptor_Out.m_Access = Gfx::CTexture::EAccess::CPUWrite;
        TextureDescriptor_Out.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
        TextureDescriptor_Out.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextureDescriptor_Out.m_Format = Gfx::CTexture::R8_UBYTE; // 1 channel with 8-bit. -> R8G8B8 = 3 channels with 8-bit.

        m_TexturePtr_RectImg = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_Out);

        //---Initialize Rectified Image @ CPU---
        m_Img_Rect_B = cv::Mat::zeros(m_ImgSize_Rect, CV_8UC1);
        m_Img_Rect_M = cv::Mat::zeros(m_ImgSize_Rect, CV_8UC1);

        //---Initialize Look-Up Table @ CPU---
        m_Table_Bx_Orig2Rect = cv::Mat::zeros(m_ImgSize_Orig, CV_8UC1);
        m_Table_By_Orig2Rect = cv::Mat::zeros(m_ImgSize_Orig, CV_8UC1);
        m_Table_Mx_Orig2Rect = cv::Mat::zeros(m_ImgSize_Orig, CV_8UC1);
        m_Table_My_Orig2Rect = cv::Mat::zeros(m_ImgSize_Orig, CV_8UC1);
        m_Table_Bx_Rect2Orig = cv::Mat::zeros(m_ImgSize_Rect, CV_8UC1);
        m_Table_By_Rect2Orig = cv::Mat::zeros(m_ImgSize_Rect, CV_8UC1);
        m_Table_Mx_Rect2Orig = cv::Mat::zeros(m_ImgSize_Rect, CV_8UC1);
        m_Table_My_Rect2Orig = cv::Mat::zeros(m_ImgSize_Rect, CV_8UC1);
    }


    PlanarRect::~PlanarRect()
    {
        //---Release Manager---
        m_CSPtr_PlanarRect = nullptr;
        m_TexturePtr_OrigImg = nullptr;
        m_TexturePtr_RectImg = nullptr;
    }

    //---Execution Functions---
    void PlanarRect::execute(FutoImg& Img_Rect_B, FutoImg& Img_Rect_M, const FutoImg& Img_Orig_B, const FutoImg& Img_Orig_M)
    {
        //---Step 0. Check the size of Original Image Size---
        assert(Img_Orig_B.get_Img().size() == m_ImgSize_Orig); 
        assert(Img_Orig_M.get_Img().size() == m_ImgSize_Orig); 

        //---Step 1. Calculate Orientations of Rectified Images & Homography from Original to Rectified---
        cal_K_Rect(Img_Orig_B.get_Cam(), Img_Orig_M.get_Cam());
        cal_R_Rect(Img_Orig_B.get_PC(), Img_Orig_M.get_PC(), Img_Orig_B.get_Rot());
        cal_PC_Rect(Img_Orig_B.get_PC(), Img_Orig_M.get_PC());
        cal_P_Rect();

        cal_H(Img_Orig_B.get_PPM(), Img_Orig_M.get_PPM());

        //---Step 2. Center Rectified Images---
        cv::Mat Drift_B = cv::Mat::zeros(3, 1, CV_32F);
        cv::Mat Drift_M = cv::Mat::zeros(3, 1, CV_32F);

        cal_Drift(Drift_B, Img_Orig_B.get_Img().size());
        cal_Drift(Drift_M, Img_Orig_M.get_Img().size());
        float Drift_y = (Drift_B.ptr<float>(1)[0] + Drift_M.ptr<float>(1)[0]) / 2;
        Drift_B.ptr<float>(1)[0] = Drift_y;
        Drift_M.ptr<float>(1)[0] = Drift_y;

        imp_Drift_K(Drift_B, Drift_M);

        cal_H(Img_Orig_B.get_PPM(), Img_Orig_M.get_PPM()); // Update Homography because Rectified Camera mtx has changed.

        //---Generate Rectified Images & Look-Up Table (Original <-> Rectified)---
        cv::Point ImgCnr_RectB_UL, ImgCnr_RectB_DR, ImgCnr_RectM_UL, ImgCnr_RectM_DR;

        cal_RectImgBound(ImgCnr_RectB_UL, ImgCnr_RectB_DR, Img_Orig_B.get_Img().size(), 0);
        cal_RectImgBound(ImgCnr_RectM_UL, ImgCnr_RectM_DR, Img_Orig_M.get_Img().size(), 1);

        determ_RectImgCnr(ImgCnr_RectB_UL, ImgCnr_RectB_DR, ImgCnr_RectM_UL, ImgCnr_RectM_DR);

        //---Step 3. Build Look-Up Table for Pixel-Wise Transformation from Rectified to Original---
        genrt_RectImg(Img_Orig_B.get_Img(), 0);
        genrt_RectImg(Img_Orig_M.get_Img(), 1);

        get_RectImg(Img_Rect_B, 0);
        get_RectImg(Img_Rect_M, 1);
    }

    //---Assistant Functions: Compute Orientations---
    void PlanarRect::cal_K_Rect(const cv::Mat& K_Orig_B, const cv::Mat& K_Orig_M)
    {
        m_K_Rect_B = 0.5 * (K_Orig_B + K_Orig_M);
        m_K_Rect_B.ptr<float>(0)[1] = 0; // Let skew = 0
        m_K_Rect_M = 0.5 * (K_Orig_B + K_Orig_M);
        m_K_Rect_M.ptr<float>(0)[1] = 0; // Let skew = 0
    }

    void PlanarRect::cal_R_Rect(const cv::Mat& PC_Orig_B, const cv::Mat& PC_Orig_M, const cv::Mat& R_Orig_B)
    {
        m_R_Rect = cv::Mat::eye(3, 3, CV_32F);

        cv::Mat R_Rect_row0 = PC_Orig_M - PC_Orig_B;
        if (cv::sum(R_Rect_row0)[0] < 0)
        {
            R_Rect_row0 *= -1; // Make the rotation always left2right and up2down.
        }

        R_Rect_row0 /= cv::norm(R_Rect_row0, cv::NORM_L2);
        cv::transpose(R_Rect_row0, R_Rect_row0);

        cv::Mat R_Rect_row1 = R_Orig_B.row(2).cross(R_Rect_row0);
        R_Rect_row1 /= cv::norm(R_Rect_row1, cv::NORM_L2);

        cv::Mat R_Rect_row2 = R_Rect_row0.cross(R_Rect_row1);
        R_Rect_row2 /= cv::norm(R_Rect_row2, cv::NORM_L2);

        R_Rect_row0.copyTo(m_R_Rect.row(0));
        R_Rect_row1.copyTo(m_R_Rect.row(1));
        R_Rect_row2.copyTo(m_R_Rect.row(2));
    }

    void PlanarRect::cal_PC_Rect(const cv::Mat& PC_Orig_B, const cv::Mat& PC_Orig_M)
    {
        m_PC_Rect_B = PC_Orig_B;
        m_PC_Rect_M = PC_Orig_M;
    }

    void PlanarRect::cal_P_Rect()
    {
        cv::Mat Trans_Rect_B(3, 4, CV_32F);
        m_R_Rect.colRange(0, 3).copyTo(Trans_Rect_B.colRange(0, 3)); // StartCol is inclusive while EndCol is exclusive
        cv::Mat t_Orig_B = -m_R_Rect * m_PC_Rect_B;
        t_Orig_B.col(0).copyTo(Trans_Rect_B.col(3));

        cv::Mat Trans_Rect_M(3, 4, CV_32F);
        m_R_Rect.colRange(0, 3).copyTo(Trans_Rect_M.colRange(0, 3)); // StartCol is inclusive while EndCol is exclusive
        cv::Mat t_Orig_M = -m_R_Rect * m_PC_Rect_M;
        t_Orig_M.col(0).copyTo(Trans_Rect_M.col(3));

        m_P_Rect_B = m_K_Rect_B * Trans_Rect_B;
        m_P_Rect_M = m_K_Rect_M * Trans_Rect_M;
    }

    void PlanarRect::cal_H(const cv::Mat& P_Orig_B, const cv::Mat& P_Orig_M)
    {
        //---Calculate the Homography---
        m_Homo_B = m_P_Rect_B * P_Orig_B.inv(cv::DECOMP_SVD);
        m_Homo_M = m_P_Rect_M * P_Orig_M.inv(cv::DECOMP_SVD);
        //---Another Homography Transformation used in Fusiello's Code. -> Simplfying to enhance efficiency.
        /*
        //H_B = P_Rect_B.colRange(0, 3) * m_Img_Orig_B.get_P_mtx().colRange(0, 3).inv();
        //H_M = P_Rect_M.colRange(0, 3) * m_Img_Orig_M.get_P_mtx().colRange(0, 3).inv();
        */
        //---

    }

    //---Assistant Functions: Center Rectified Images---
    void PlanarRect::cal_Drift(cv::Mat& CenterDrift, const cv::Size& ImgSize_Orig)
    {
        cv::Mat Center_Orig = cv::Mat::ones(3, 1, CV_32F);
        Center_Orig.ptr<float>(0)[0] = ImgSize_Orig.width / 2;
        Center_Orig.ptr<float>(1)[0] = ImgSize_Orig.height / 2;

        cv::Mat Center_Orig2Rect = m_Homo_B * Center_Orig;
        Center_Orig2Rect /= Center_Orig2Rect.ptr<float>(2)[0];

        CenterDrift = Center_Orig - Center_Orig2Rect;
    }

    void PlanarRect::imp_Drift_K(const cv::Mat& Drift_B, const cv::Mat& Drift_M)
    {
        m_K_Rect_B.ptr<float>(0)[2] += Drift_B.ptr<float>(0)[0];
        m_K_Rect_B.ptr<float>(1)[2] += Drift_B.ptr<float>(1)[0];
        m_K_Rect_M.ptr<float>(0)[2] += Drift_M.ptr<float>(0)[0];
        m_K_Rect_M.ptr<float>(1)[2] += Drift_M.ptr<float>(1)[0];
    }

    //---Assistant Functions: Generate Rectified Images---
    void PlanarRect::cal_RectImgBound(cv::Point& ImgCnr_Rect_UL, cv::Point& ImgCnr_Rect_DR, const cv::Size& ImgSize_Orig, const int Which_Img)
    {
        cv::Mat H;
        switch (Which_Img)
        {
        case 0: H = m_Homo_B;
            break;
        case 1: H = m_Homo_M;
            break;
        }

        //---Select corners in original images---
        cv::Mat ImgCnr_Orig_UL = cv::Mat::zeros(3, 1, CV_32F);
        ImgCnr_Orig_UL.ptr<float>(2)[0] = 1;
        cv::Mat ImgCnr_Orig_UR = cv::Mat::ones(3, 1, CV_32F);
        ImgCnr_Orig_UR.ptr<float>(0)[0] = ImgSize_Orig.width;
        ImgCnr_Orig_UR.ptr<float>(1)[0] = 0;
        cv::Mat ImgCnr_Orig_DL = cv::Mat::ones(3, 1, CV_32F);
        ImgCnr_Orig_DL.ptr<float>(0)[0] = 0;
        ImgCnr_Orig_DL.ptr<float>(1)[0] = ImgSize_Orig.height;
        cv::Mat ImgCnr_Orig_DR = cv::Mat::ones(3, 1, CV_32F);
        ImgCnr_Orig_DR.ptr<float>(0)[0] = ImgSize_Orig.width;
        ImgCnr_Orig_DR.ptr<float>(1)[0] = ImgSize_Orig.height;

        //---Transform corners from original to rectified---
        cv::Mat ImgCnr_Orig2Rect_UL = H * ImgCnr_Orig_UL;
        ImgCnr_Orig2Rect_UL /= ImgCnr_Orig2Rect_UL.ptr<float>(2)[0];
        cv::Mat ImgCnr_Orig2Rect_UR = H * ImgCnr_Orig_UR;
        ImgCnr_Orig2Rect_UR /= ImgCnr_Orig2Rect_UR.ptr<float>(2)[0];
        cv::Mat ImgCnr_Orig2Rect_DL = H * ImgCnr_Orig_DL;
        ImgCnr_Orig2Rect_DL /= ImgCnr_Orig2Rect_DL.ptr<float>(2)[0];
        cv::Mat ImgCnr_Orig2Rect_DR = H * ImgCnr_Orig_DR;
        ImgCnr_Orig2Rect_DR /= ImgCnr_Orig2Rect_DR.ptr<float>(2)[0];

        //---Determine the Boundary of Epipolar Image---
        ImgCnr_Rect_UL.x = 
            std::floor(std::min(std::min(ImgCnr_Orig2Rect_UL.ptr<float>(0)[0], ImgCnr_Orig2Rect_UR.ptr<float>(0)[0]), std::min(ImgCnr_Orig2Rect_DL.ptr<float>(0)[0], ImgCnr_Orig2Rect_DR.ptr<float>(0)[0])));
        ImgCnr_Rect_UL.y =
            std::floor(std::min(std::min(ImgCnr_Orig2Rect_UL.ptr<float>(1)[0], ImgCnr_Orig2Rect_UR.ptr<float>(1)[0]), std::min(ImgCnr_Orig2Rect_DL.ptr<float>(1)[0], ImgCnr_Orig2Rect_DR.ptr<float>(1)[0])));
        ImgCnr_Rect_DR.x =
            std::ceil(std::max(std::max(ImgCnr_Orig2Rect_UL.ptr<float>(0)[0], ImgCnr_Orig2Rect_UR.ptr<float>(0)[0]), std::max(ImgCnr_Orig2Rect_DL.ptr<float>(0)[0], ImgCnr_Orig2Rect_DR.ptr<float>(0)[0])));
        ImgCnr_Rect_DR.y =
            std::ceil(std::max(std::max(ImgCnr_Orig2Rect_UL.ptr<float>(1)[0], ImgCnr_Orig2Rect_UR.ptr<float>(1)[0]), std::max(ImgCnr_Orig2Rect_DL.ptr<float>(1)[0], ImgCnr_Orig2Rect_DR.ptr<float>(1)[0])));
    }

    void PlanarRect::determ_RectImgCnr(const cv::Point& ImgCnr_RectB_UL, const cv::Point& ImgCnr_RectB_DR, const cv::Point& ImgCnr_RectM_UL, const cv::Point& ImgCnr_RectM_DR)
    {
        //---Calculate the common boundary of both rectified images---
        m_ImgCnr_Rect_UL.x = std::min(ImgCnr_RectB_UL.x, ImgCnr_RectM_UL.x);
        m_ImgCnr_Rect_UL.y = std::min(ImgCnr_RectB_UL.y, ImgCnr_RectM_UL.y);
        m_ImgCnr_Rect_DR.x = std::max(ImgCnr_RectB_DR.x, ImgCnr_RectM_DR.x);
        m_ImgCnr_Rect_DR.y = std::max(ImgCnr_RectB_DR.y, ImgCnr_RectM_DR.y);
    }

    void PlanarRect::genrt_RectImg(const cv::Mat& Img_Orig, const int Which_Img)
    {
        cv::Mat H(3, 3, CV_32F);
        switch (Which_Img)
        {
        case 0: H = m_Homo_B;
            break;
        case 1: H = m_Homo_M;
            break;
        }

        //---Compute @ GPU---

        Gfx::Performance::BeginEvent("Planar Rectification");

        //---Put OrigImg into Input Texture---
        Base::AABB2UInt TargetRect;
        TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_ImgSize_Orig.width, m_ImgSize_Orig.height));
        Gfx::TextureManager::CopyToTexture2D(m_TexturePtr_OrigImg, TargetRect, m_ImgSize_Orig.width, Img_Orig.data);

        //---Put Homography into Buffer---
        Gfx::BufferManager::UploadBufferData(m_BufferPtr_Homography, H.data, 0, sizeof(float) * H.rows * H.cols);

        //---Connecting Managers (@CPU) & GLSL (@GPU)---
        Gfx::ContextManager::SetShaderCS(m_CSPtr_PlanarRect);
        Gfx::ContextManager::SetImageTexture(0, m_TexturePtr_OrigImg);
        Gfx::ContextManager::SetImageTexture(1, m_TexturePtr_RectImg);
        Gfx::ContextManager::SetConstantBuffer(0, m_BufferPtr_Homography);

        //---Start GPU Parallel Processing---
        const int WorkGroupsX = DivUp(m_ImgSize_Rect.width, TileSize_2D);
        const int WorkGroupsY = DivUp(m_ImgSize_Rect.height, TileSize_2D);

        Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        Gfx::ContextManager::ResetShaderCS();

        cv::Mat Img_Rect(m_TexturePtr_RectImg->GetNumberOfPixelsV(), m_TexturePtr_RectImg->GetNumberOfPixelsU(), CV_8UC1);
        Gfx::TextureManager::CopyTextureToCPU(m_TexturePtr_RectImg, reinterpret_cast<char*>(Img_Rect.data));

        cv::imshow("RectImg by GPU", Img_Rect);

        Gfx::Performance::EndEvent();


        //---Compute @ CPU (Remove after finishing "Compute @ GPU")---

        //---Create Rectified Images---
        cv::Mat Img_Rect(m_ImgSize_Rect, CV_8UC1);

        //---Build Look-Up Table: from Rectified to Originals---
        cv::Mat map_Rect2Orig_x(m_ImgSize_Rect, CV_32FC1);
        cv::Mat map_Rect2Orig_y(m_ImgSize_Rect, CV_32FC1);

        for (int idx_y = 0; idx_y < m_ImgSize_Rect.height; idx_y++)
        {
            for (int idx_x = 0; idx_x < m_ImgSize_Rect.width; idx_x++)
            {
                cv::Mat pix_Rect = cv::Mat::ones(3, 1, CV_32F);
                pix_Rect.ptr<float>(0)[0] = idx_x + m_ImgCnr_Rect_UL.x;
                pix_Rect.ptr<float>(1)[0] = idx_y + m_ImgCnr_Rect_UL.y;

                cv::Mat pix_Rect2Orig = H.inv() * pix_Rect;
                pix_Rect2Orig /= pix_Rect2Orig.ptr<float>(2)[0];

                map_Rect2Orig_x.ptr<float>(idx_y)[idx_x] = pix_Rect2Orig.ptr<float>(0)[0];
                map_Rect2Orig_y.ptr<float>(idx_y)[idx_x] = pix_Rect2Orig.ptr<float>(1)[0];
            }
        }

        //---Derive Pixel Value of Rectified Images: Transform Pixels from Rectfied back to Origianls & Interpolation---
        cv::remap(Img_Orig, Img_Rect, map_Rect2Orig_x, map_Rect2Orig_y, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);

        switch (Which_Img)
        {
        case 0:
            Img_Rect.copyTo(m_Img_Rect_B);
            break;
        case 1:
            Img_Rect.copyTo(m_Img_Rect_M);
            break;
        }
    }

    //---Assistant Functions: Return Rectified Images---
    void PlanarRect::get_RectImg(FutoImg& Img_Rect, const int Which_Img)
    {
        switch (Which_Img)
        {
        case 0: 
            Img_Rect = FutoImg(m_Img_Rect_B, m_K_Rect_B, m_R_Rect, m_PC_Rect_B);
            break;
        case 1: 
            Img_Rect = FutoImg(m_Img_Rect_M, m_K_Rect_M, m_R_Rect, m_PC_Rect_M);
            break;
        }
    }

} // FutoGmtCV
