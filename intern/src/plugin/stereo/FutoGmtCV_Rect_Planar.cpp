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

    struct SHomographyTransform // Always make sure whole structure is the multiple of 4*float
    {
        glm::mat4 m_Homography; // Transmition unit of layout(std140) is 4*float. => mat3 is only 3*3*float, which may transmit wrong memory address.
        glm::mat4 m_InvHomography;
        glm::ivec2 m_Shift_Orig2Rect; // pix_Rect = pix_Orig2Rect + Shift; <= pix_Orig2Rect = H * pix_Orig;
        glm::ivec2 m_Padding;
    };
}

namespace FutoGmtCV
{
    //---Constructors & Destructor---
    Rectification_Planar::Rectification_Planar()
    {
    }

    Rectification_Planar::Rectification_Planar(const glm::ivec2& OrigImgSize, const glm::ivec2& RectImgSize)
        : m_ImgSize_Orig(OrigImgSize),
          m_ImgSize_Rect(RectImgSize)
    {
        //---Initialize Shader Manager---
        std::stringstream DefineStream;
        DefineStream
            << "#define TILE_SIZE_2D " << TileSize_2D << " \n"; // 16 for work group size is suggested for 2D image (based on experience).
        std::string DefineString = DefineStream.str();

        m_PlanarRectCSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/cs_Rect_Planar.glsl", "main", DefineString.c_str());

        //---Initialize Buffer Manager---
        Gfx::SBufferDescriptor BufferDesc = {};

        BufferDesc.m_Stride = 0;
        BufferDesc.m_Usage = Gfx::CBuffer::GPURead;
        BufferDesc.m_Binding = Gfx::CBuffer::ConstantBuffer;
        BufferDesc.m_Access = Gfx::CBuffer::CPUWrite;
        BufferDesc.m_NumberOfBytes = sizeof(SHomographyTransform);
        BufferDesc.m_pBytes = nullptr;
        BufferDesc.m_pClassKey = 0;

        m_HomographyBufferPtr = Gfx::BufferManager::CreateBuffer(BufferDesc);

        //---Initialize Input Texture Manager---
        Gfx::STextureDescriptor TextureDescriptor_In = {};

        TextureDescriptor_In.m_NumberOfPixelsU = m_ImgSize_Orig.x;
        TextureDescriptor_In.m_NumberOfPixelsV = m_ImgSize_Orig.y;
        TextureDescriptor_In.m_NumberOfPixelsW = 1;
        TextureDescriptor_In.m_NumberOfMipMaps = 1;
        TextureDescriptor_In.m_NumberOfTextures = 1;
        TextureDescriptor_In.m_Binding = Gfx::CTexture::ShaderResource;
        TextureDescriptor_In.m_Access = Gfx::CTexture::EAccess::CPURead;
        TextureDescriptor_In.m_Usage = Gfx::CTexture::EUsage::GPUToCPU;
        TextureDescriptor_In.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextureDescriptor_In.m_Format = Gfx::CTexture::R8G8B8A8_BYTE; // 4 channels and each channel is 8-bit.

        m_OrigImgTexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_In);

        //---Initialize Output Texture Manager---
        Gfx::STextureDescriptor TextureDescriptor_Out = {};

        TextureDescriptor_Out.m_NumberOfPixelsU = m_ImgSize_Rect.x;
        TextureDescriptor_Out.m_NumberOfPixelsV = m_ImgSize_Rect.y;
        TextureDescriptor_Out.m_NumberOfPixelsW = 1;
        TextureDescriptor_Out.m_NumberOfMipMaps = 1;
        TextureDescriptor_Out.m_NumberOfTextures = 1;
        TextureDescriptor_Out.m_Binding = Gfx::CTexture::ShaderResource;
        TextureDescriptor_Out.m_Access = Gfx::CTexture::EAccess::CPUWrite;
        TextureDescriptor_Out.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
        TextureDescriptor_Out.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextureDescriptor_Out.m_Format = Gfx::CTexture::R8G8B8A8_BYTE; // 4 channels and each channel is 8-bit.

        m_RectImgTexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_Out);

        //---Initialize Rectified Image @ CPU---
        m_Img_Rect_B = cv::Mat::zeros(m_ImgSize_Rect, CV_8UC1);
        m_Img_Rect_M = cv::Mat::zeros(m_ImgSize_Rect, CV_8UC1);
    }


    Rectification_Planar::~Rectification_Planar()
    {
        //---Release Manager---
        m_PlanarRectCSPtr = nullptr;
        m_OrigImgTexturePtr = nullptr;
        m_RectImgTexturePtr = nullptr;
    }

    //---Execution Functions---
    void Rectification_Planar::execute(FutoImg& Img_Rect_B, FutoImg& Img_Rect_M, const FutoImg& Img_Orig_B, const FutoImg& Img_Orig_M)
    {
        //---Step 0. Check the size of Original Image Size---
        assert(Img_Orig_B.get_ImgSize().x == m_ImgSize_Orig.x && Img_Orig_B.get_ImgSize().y == m_ImgSize_Orig.y);
        assert(Img_Orig_M.get_ImgSize().x == m_ImgSize_Orig.x && Img_Orig_M.get_ImgSize().y == m_ImgSize_Orig.y);

        //---Step 1. Calculate Orientations of Rectified Images & Homography from Original to Rectified---
        cal_K_Rect(Img_Orig_B.get_Cam(), Img_Orig_M.get_Cam());
        cal_R_Rect(Img_Orig_B.get_PC(), Img_Orig_M.get_PC(), Img_Orig_B.get_Rot());
        cal_PC_Rect(Img_Orig_B.get_PC(), Img_Orig_M.get_PC());
        cal_P_Rect();

        cal_H(Img_Orig_B.get_PPM(), Img_Orig_M.get_PPM());

        //---Step 2. Center Rectified Images---
        glm::vec3 CenterShift_B(0.0f);
        glm::vec3 CenterShift_M(0.0f);

        cal_CneterShift(CenterShift_B, Img_Orig_B.get_ImgSize());
        cal_CneterShift(CenterShift_M, Img_Orig_M.get_ImgSize());
        float CenterShift_y = (CenterShift_B.y + CenterShift_M.y) / 2;
        CenterShift_B.y = CenterShift_y;
        CenterShift_M.y = CenterShift_y;

        imp_CenterShift_K(CenterShift_B, CenterShift_M);

        cal_H(Img_Orig_B.get_PPM(), Img_Orig_M.get_PPM()); // Update Homography because Rectified Camera mtx has changed.

        //---Step 3. Calculate the Corners of Rectified Images---
        glm::ivec2 ImgCnr_RectB_UL, ImgCnr_RectB_DR, ImgCnr_RectM_UL, ImgCnr_RectM_DR;

        cal_RectImgBound(ImgCnr_RectB_UL, ImgCnr_RectB_DR, Img_Orig_B.get_Img().size(), 0);
        cal_RectImgBound(ImgCnr_RectM_UL, ImgCnr_RectM_DR, Img_Orig_M.get_Img().size(), 1);

        determ_RectImgCnr(ImgCnr_RectB_UL, ImgCnr_RectB_DR, ImgCnr_RectM_UL, ImgCnr_RectM_DR);

        //---Step 4. Generate Rectified Images---
        genrt_RectImg(Img_Orig_B.get_Img(), 0);
        genrt_RectImg(Img_Orig_M.get_Img(), 1);

        get_RectImg(Img_Rect_B, 0);
        get_RectImg(Img_Rect_M, 1);
    }

    //---Assistant Functions: Compute Orientations---
    void Rectification_Planar::cal_K_Rect(const glm::mat3& K_Orig_B, const glm::mat3& K_Orig_M)
    {
        m_K_Rect_B = 0.5 * (K_Orig_B + K_Orig_M);
        m_K_Rect_B[1].x = 0; // Let skew = 0
        m_K_Rect_M = m_K_Rect_B; // Camera mtx of both rectified images are the same.
    }

    void Rectification_Planar::cal_R_Rect(const glm::vec3& PC_Orig_B, const glm::vec3& PC_Orig_M, const glm::mat3& R_Orig_B)
    {
        glm::mat3 R_Orig_row = glm::transpose(R_Orig_B);

        glm::vec3 R_Rect_row0 = PC_Orig_M - PC_Orig_B;
        if ((R_Rect_row0.x + R_Rect_row0.y + R_Rect_row0.z) < 0)
        {
            R_Rect_row0 *= -1; // Make the rotation always left2right and up2down.
        }

        R_Rect_row0 /= glm::l2Norm(R_Rect_row0);

        glm::vec3 R_Rect_row1 = glm::cross(R_Orig_row[1], R_Rect_row0);
        R_Rect_row1 /= glm::l2Norm(R_Rect_row1);

        glm::vec3 R_Rect_row2 = glm::cross(R_Rect_row0, R_Rect_row1);
        R_Rect_row2 /= glm::l2Norm(R_Rect_row2);

        m_R_Rect = glm::mat3(R_Rect_row0, R_Rect_row1, R_Rect_row2);
        m_R_Rect = glm::transpose(m_R_Rect);
    }

    void Rectification_Planar::cal_PC_Rect(const glm::vec3& PC_Orig_B, const glm::vec3& PC_Orig_M)
    {
        m_PC_Rect_B = PC_Orig_B;
        m_PC_Rect_M = PC_Orig_M;
    }

    void Rectification_Planar::cal_P_Rect()
    {
        m_P_Rect_B = glm::mat4x3(m_R_Rect[0], m_R_Rect[1], m_R_Rect[2], -m_R_Rect * m_PC_Rect_B);
        m_P_Rect_B *= m_K_Rect_B;

        m_P_Rect_M = glm::mat4x3(m_R_Rect[0], m_R_Rect[1], m_R_Rect[2], -m_R_Rect * m_PC_Rect_M);
        m_P_Rect_M *= m_K_Rect_M;
    }

    void Rectification_Planar::cal_H(const glm::mat4x3& P_Orig_B, const glm::mat4x3& P_Orig_M)
    {
        //---Calculate the Homography---
        // A simple way to calculate Homography (Also proposed by Fusiello). <= glm::inverse cannot apply on glm::mat4*3
        m_Homo_B = glm::mat3(m_P_Rect_B) * glm::inverse(glm::mat3(P_Orig_B));
        m_Homo_M = glm::mat3(m_P_Rect_M) * glm::inverse(glm::mat3(P_Orig_M));
    }

    //---Assistant Functions: Center Rectified Images---
    void Rectification_Planar::cal_CneterShift(glm::vec3& CenterDrift, const glm::ivec2& ImgSize_Orig)
    {
        glm::vec3 Center_Orig(ImgSize_Orig.x/2, ImgSize_Orig.y/2, 1.0f);

        glm::vec3 Center_Orig2Rect = m_Homo_B * Center_Orig;
        Center_Orig2Rect /= Center_Orig2Rect.z;

        CenterDrift = Center_Orig - Center_Orig2Rect;
    }

    void Rectification_Planar::imp_CenterShift_K(const glm::vec3& Drift_B, const glm::vec3& Drift_M)
    {
        m_K_Rect_B[2].x += Drift_B.x;
        m_K_Rect_B[2].y += Drift_B.y;
        m_K_Rect_M[2].x += Drift_M.x;
        m_K_Rect_M[2].y += Drift_M.y;
    }

    //---Assistant Functions: Generate Rectified Images---
    void Rectification_Planar::cal_RectImgBound(glm::ivec2& ImgCnr_Rect_UL, glm::ivec2& ImgCnr_Rect_DR, const glm::ivec2& ImgSize_Orig, const int Which_Img)
    {
        glm::mat3 H;
        switch (Which_Img)
        {
        case 0: H = m_Homo_B;
            break;
        case 1: H = m_Homo_M;
            break;
        }

        //---Select corners in original images---
        glm::ivec3 ImgCnr_Orig_UL(0, 0, 1);
        glm::ivec3 ImgCnr_Orig_UR(ImgSize_Orig.x, 0, 1);
        glm::ivec3 ImgCnr_Orig_DL(0, ImgSize_Orig.y, 1);
        glm::ivec3 ImgCnr_Orig_DR(ImgSize_Orig.x, ImgSize_Orig.y, 1);

        //---Transform corners from original to rectified---
        glm::vec3 ImgCnr_Orig2Rect_UL = H * ImgCnr_Orig_UL;
        ImgCnr_Orig2Rect_UL /= ImgCnr_Orig2Rect_UL.z;
        glm::vec3 ImgCnr_Orig2Rect_UR = H * ImgCnr_Orig_UR;
        ImgCnr_Orig2Rect_UR /= ImgCnr_Orig2Rect_UR.z;
        glm::vec3 ImgCnr_Orig2Rect_DL = H * ImgCnr_Orig_DL;
        ImgCnr_Orig2Rect_DL /= ImgCnr_Orig2Rect_DL.z;
        glm::vec3 ImgCnr_Orig2Rect_DR = H * ImgCnr_Orig_DR;
        ImgCnr_Orig2Rect_DR /= ImgCnr_Orig2Rect_DR.z;

        //---Determine the Boundary of Epipolar Image---
        ImgCnr_Rect_UL.x = 
            std::floor(std::min(std::min(ImgCnr_Orig2Rect_UL.x, ImgCnr_Orig2Rect_UR.x), std::min(ImgCnr_Orig2Rect_DL.x, ImgCnr_Orig2Rect_DR.x)));
        ImgCnr_Rect_UL.y =
            std::floor(std::min(std::min(ImgCnr_Orig2Rect_UL.y, ImgCnr_Orig2Rect_UR.y), std::min(ImgCnr_Orig2Rect_DL.y, ImgCnr_Orig2Rect_DR.y)));
        ImgCnr_Rect_DR.x =
            std::ceil(std::max(std::max(ImgCnr_Orig2Rect_UL.x, ImgCnr_Orig2Rect_UR.x), std::max(ImgCnr_Orig2Rect_DL.x, ImgCnr_Orig2Rect_DR.x)));
        ImgCnr_Rect_DR.y =
            std::ceil(std::max(std::max(ImgCnr_Orig2Rect_UL.y, ImgCnr_Orig2Rect_UR.y), std::max(ImgCnr_Orig2Rect_DL.y, ImgCnr_Orig2Rect_DR.y)));
    }

    void Rectification_Planar::determ_RectImgCnr(const glm::ivec2& ImgCnr_RectB_UL, const glm::ivec2& ImgCnr_RectB_DR, const glm::ivec2& ImgCnr_RectM_UL, const glm::ivec2& ImgCnr_RectM_DR)
    {
        //---Calculate the common boundary of both rectified images---
        m_ImgCnr_Rect_UL.x = std::min(ImgCnr_RectB_UL.x, ImgCnr_RectM_UL.x);
        m_ImgCnr_Rect_UL.y = std::min(ImgCnr_RectB_UL.y, ImgCnr_RectM_UL.y);
        m_ImgCnr_Rect_DR.x = std::max(ImgCnr_RectB_DR.x, ImgCnr_RectM_DR.x);
        m_ImgCnr_Rect_DR.y = std::max(ImgCnr_RectB_DR.y, ImgCnr_RectM_DR.y);
    }

    void Rectification_Planar::genrt_RectImg(const cv::Mat& Img_Orig, const int Which_Img)
    {
        glm::mat3 H(0.0);
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
        TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(m_ImgSize_Orig.xh, m_ImgSize_Orig.y));
        Gfx::TextureManager::CopyToTexture2D(m_OrigImgTexturePtr, TargetRect, m_ImgSize_Orig.x, Img_Orig.data);

        //---Put Homography into Buffer---
        SHomographyTransform HomographyBuffer;

        HomographyBuffer.m_Homography = glm::mat4(H);
        HomographyBuffer.m_InvHomography = glm::mat4(glm::inverse(H));
        HomographyBuffer.m_Shift_Orig2Rect.x = m_ImgCnr_Rect_UL.x;
        HomographyBuffer.m_Shift_Orig2Rect.y = m_ImgCnr_Rect_UL.y;


        Gfx::BufferManager::UploadBufferData(m_HomographyBufferPtr, &HomographyBuffer);

        //---Connecting Managers (@CPU) & GLSL (@GPU)---
        Gfx::ContextManager::SetShaderCS(m_PlanarRectCSPtr);
        Gfx::ContextManager::SetImageTexture(0, m_OrigImgTexturePtr);
        Gfx::ContextManager::SetImageTexture(1, m_RectImgTexturePtr);
        Gfx::ContextManager::SetConstantBuffer(0, m_HomographyBufferPtr);

        //---Start GPU Parallel Processing---
        const int WorkGroupsX = DivUp(m_ImgSize_Rect.x, TileSize_2D);
        const int WorkGroupsY = DivUp(m_ImgSize_Rect.y, TileSize_2D);

        Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        Gfx::ContextManager::ResetShaderCS();

        Gfx::Performance::EndEvent();

        cv::Mat Img_Rect(m_RectImgTexturePtr->GetNumberOfPixelsV(), m_RectImgTexturePtr->GetNumberOfPixelsU(), CV_8UC1);
        Gfx::TextureManager::CopyTextureToCPU(m_RectImgTexturePtr, reinterpret_cast<char*>(Img_Rect.data));

        switch (Which_Img)
        {
        case 0:
            Img_Rect.copyTo(m_Img_Rect_B);
            break;
        case 1:
            Img_Rect.copyTo(m_Img_Rect_M);
            break;
        }


        //---Compute @ CPU (Remove after finishing "Compute @ GPU")---
        /*
        //---Create Rectified Images---
        //cv::Mat Img_Rect(m_ImgSize_Rect, CV_8UC1);

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
        */
    }

    //---Assistant Functions: Return Rectified Images---
    void Rectification_Planar::get_RectImg(FutoImg& Img_Rect, const int Which_Img)
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
