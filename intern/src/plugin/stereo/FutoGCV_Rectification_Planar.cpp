#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\FutoGCV_Rectification_Planar.h"

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

namespace FutoGCV
{
    //---Constructors & Destructor---
    CPlanarRectification::CPlanarRectification()
    {
    }

    CPlanarRectification::CPlanarRectification(const glm::uvec2& ImgSize_Orig, const glm::uvec2& ImgSize_Rect, const glm::uvec2& ImgSize_DownSample)
        : m_ImgSize_Rect(ImgSize_Rect),
          m_ImgSize_DownSample(ImgSize_DownSample)
    {
        //---Initialize Shader Manager---
        std::stringstream DefineStream;
        DefineStream
            << "#define TILE_SIZE_2D " << TileSize_2D << " \n"; // 16 for work group size is suggested for 2D image (based on experience).
        std::string DefineString = DefineStream.str();

        m_PlanarRectCSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/cs_Rectification_Planar.glsl", "main", DefineString.c_str());

        m_DownSamplingCSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/cs_Down_Sampling.glsl", "main", DefineString.c_str());

        //---Initialize Buffer Manager---
        Gfx::SBufferDescriptor BufferDesc_Homo = {};

        BufferDesc_Homo.m_Stride = 0;
        BufferDesc_Homo.m_Usage = Gfx::CBuffer::GPURead;
        BufferDesc_Homo.m_Binding = Gfx::CBuffer::ConstantBuffer;
        BufferDesc_Homo.m_Access = Gfx::CBuffer::CPUWrite;
        BufferDesc_Homo.m_NumberOfBytes = sizeof(SHomographyTransform);
        BufferDesc_Homo.m_pBytes = nullptr;
        BufferDesc_Homo.m_pClassKey = 0;

        m_HomographyB_BufferPtr = Gfx::BufferManager::CreateBuffer(BufferDesc_Homo);
        m_HomographyM_BufferPtr = Gfx::BufferManager::CreateBuffer(BufferDesc_Homo);

        //---Initialize Texture Manager for Original Image---
        Gfx::STextureDescriptor TextureDescriptor_OrigImg = {};

        TextureDescriptor_OrigImg.m_NumberOfPixelsU = ImgSize_Orig.x;
        TextureDescriptor_OrigImg.m_NumberOfPixelsV = ImgSize_Orig.y;
        TextureDescriptor_OrigImg.m_NumberOfPixelsW = 1;
        TextureDescriptor_OrigImg.m_NumberOfMipMaps = 1;
        TextureDescriptor_OrigImg.m_NumberOfTextures = 1;
        TextureDescriptor_OrigImg.m_Binding = Gfx::CTexture::ShaderResource;
        TextureDescriptor_OrigImg.m_Access = Gfx::CTexture::EAccess::CPURead;
        TextureDescriptor_OrigImg.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
        TextureDescriptor_OrigImg.m_Semantic = Gfx::CTexture::UndefinedSemantic;
        TextureDescriptor_OrigImg.m_Format = Gfx::CTexture::R8G8B8A8_UBYTE; // 4 channels and each channel is 8-bit.

        m_OrigImgB_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_OrigImg);
        m_OrigImgM_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_OrigImg);

        if (m_ImgSize_Rect == glm::uvec2(0))
        {
            m_Is_FixSize = false;
        }
        else
        {
            m_Is_FixSize = true;

            //---Initialize Texture Manager for Rectified Image---
            Gfx::STextureDescriptor TextureDescriptor_RectImg = TextureDescriptor_OrigImg;

            TextureDescriptor_RectImg.m_NumberOfPixelsU = m_ImgSize_Rect.x;
            TextureDescriptor_RectImg.m_NumberOfPixelsV = m_ImgSize_Rect.y;
            TextureDescriptor_RectImg.m_Format = Gfx::CTexture::R8_UBYTE; // 1 channels and each channel is 8-bit.

            m_RectImgB_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_RectImg);
            m_RectImgM_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_RectImg);
        }

        if (m_ImgSize_DownSample == glm::uvec2(0))
        {
            m_Is_DownSample = false;
        }
        else
        {
            m_Is_DownSample = true;

            Gfx::STextureDescriptor TextureDescriptor_DownSampling = {};

            TextureDescriptor_DownSampling.m_NumberOfPixelsU = m_ImgSize_DownSample.x;
            TextureDescriptor_DownSampling.m_NumberOfPixelsV = m_ImgSize_DownSample.y;
            TextureDescriptor_DownSampling.m_NumberOfPixelsW = 1;
            TextureDescriptor_DownSampling.m_NumberOfMipMaps = 1;
            TextureDescriptor_DownSampling.m_NumberOfTextures = 1;
            TextureDescriptor_DownSampling.m_Binding = Gfx::CTexture::ShaderResource;
            TextureDescriptor_DownSampling.m_Access = Gfx::CTexture::EAccess::CPURead;
            TextureDescriptor_DownSampling.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
            TextureDescriptor_DownSampling.m_Semantic = Gfx::CTexture::UndefinedSemantic;
            TextureDescriptor_DownSampling.m_Format = Gfx::CTexture::R8_UBYTE; // Single channel with 8-bit.

            m_RectImgB_DownSample_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_DownSampling);
            m_RectImgM_DownSample_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_DownSampling);
        }
    }

    CPlanarRectification::~CPlanarRectification()
    {
        //---Release Manager---
        m_PlanarRectCSPtr = nullptr;

        m_OrigImgB_TexturePtr = nullptr;
        m_OrigImgM_TexturePtr = nullptr;

        m_RectImgB_TexturePtr = nullptr;
        m_RectImgM_TexturePtr = nullptr;

        m_HomographyB_BufferPtr = nullptr;
        m_HomographyM_BufferPtr = nullptr;
    }

    //---Execution Functions---
    void CPlanarRectification::execute(const CFutoImg& OrigImg_B, const CFutoImg& OrigImg_M)
    {
        m_Is_LargeSize = false;

        //---Step 1. Calculate Orientations of Rectified Images & Homography from Original to Rectified---
        cal_K_Rect(OrigImg_B.get_Cam(), OrigImg_M.get_Cam());
        cal_R_Rect(OrigImg_B.get_PC(), OrigImg_M.get_PC(), OrigImg_B.get_Rot());
        cal_PC_Rect(OrigImg_B.get_PC(), OrigImg_M.get_PC());
        cal_P_Rect();

        cal_H(OrigImg_B.get_PPM(), OrigImg_M.get_PPM());

        //---Step 2. Center Rectified Images---

        glm::vec2 CenterShift_B(0.0f);
        glm::vec2 CenterShift_M(0.0f);

        cal_CenterShift(CenterShift_B, OrigImg_B.get_ImgSize(), 0);
        cal_CenterShift(CenterShift_M, OrigImg_M.get_ImgSize(), 1);

        float CenterShift_y = (CenterShift_B.y + CenterShift_M.y) / 2;
        CenterShift_B.y = CenterShift_y;
        CenterShift_M.y = CenterShift_y;

        imp_CenterShift_K(CenterShift_B, CenterShift_M);

        cal_H(OrigImg_B.get_PPM(), OrigImg_M.get_PPM()); // Update Homography because Rectified Camera mtx has changed.

        //---Step 3. Calculate the Corners of Rectified Images---
        cal_RectImgBound(OrigImg_B.get_ImgSize(), 0);
        cal_RectImgBound(OrigImg_M.get_ImgSize(), 1);

        determ_RectFrame();

        //---Step 4. Generate Rectified Images---
        genrt_RectImg(OrigImg_B.get_Img(), OrigImg_B.get_ImgSize(), 0);
        genrt_RectImg(OrigImg_M.get_Img(), OrigImg_M.get_ImgSize(), 1);
    }

    void CPlanarRectification::return_Result(CFutoImg& RectImgB, CFutoImg& RectImgM, SHomographyTransform& Homo_B, SHomographyTransform& Homo_M)
    {
        const int RectImgSize_1D = m_ImgSize_Rect.x * m_ImgSize_Rect.y;
        std::vector<char> RectImgB_Vector1D(RectImgSize_1D, 0), RectImgM_Vector1D(RectImgSize_1D, 0);
        Gfx::TextureManager::CopyTextureToCPU(m_RectImgB_TexturePtr, reinterpret_cast<char*>(RectImgB_Vector1D.data()));
        Gfx::TextureManager::CopyTextureToCPU(m_RectImgM_TexturePtr, reinterpret_cast<char*>(RectImgM_Vector1D.data()));


        RectImgB = CFutoImg(RectImgB_Vector1D, m_ImgSize_Rect, 1, m_K_Rect_B, m_R_Rect, m_PC_Rect_B);
        RectImgM = CFutoImg(RectImgM_Vector1D, m_ImgSize_Rect, 1, m_K_Rect_M, m_R_Rect, m_PC_Rect_M);

        Homo_B = m_Homography_B;
        Homo_M = m_Homography_M;
    }

    void CPlanarRectification::imp_DownSampling(CFutoImg& RectImg_DownSampling, const int Which_Img)
    {
        //---GPU Computation Start---
        Gfx::Performance::BeginEvent("Down-Sampling Rectified Image");

        Gfx::ContextManager::SetShaderCS(m_DownSamplingCSPtr);

        switch (Which_Img)
        {
        case 0:
            Gfx::ContextManager::SetImageTexture(0, m_RectImgB_TexturePtr);
            Gfx::ContextManager::SetImageTexture(1, m_RectImgB_DownSample_TexturePtr);

            break;

        case 1:
            Gfx::ContextManager::SetImageTexture(0, m_RectImgM_TexturePtr);
            Gfx::ContextManager::SetImageTexture(1, m_RectImgM_DownSample_TexturePtr);

            break;
        }

        //---Start GPU Parallel Processing---
        const int WorkGroupsX = DivUp(m_ImgSize_DownSample.x, TileSize_2D);
        const int WorkGroupsY = DivUp(m_ImgSize_DownSample.y, TileSize_2D);

        Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        Gfx::ContextManager::ResetShaderCS();

        Gfx::Performance::EndEvent();
        //---GPU Computation End---

        //---Return Down-Sampled Rectified Images---
        const int RectImgSize_1D = m_ImgSize_DownSample.x * m_ImgSize_DownSample.y;
        std::vector<char> RectImg_Vector1D(RectImgSize_1D, 0);

        switch (Which_Img)
        {
        case 0:
            Gfx::TextureManager::CopyTextureToCPU(m_RectImgB_DownSample_TexturePtr, reinterpret_cast<char*>(RectImg_Vector1D.data()));

            break;

        case 1:
            Gfx::TextureManager::CopyTextureToCPU(m_RectImgM_DownSample_TexturePtr, reinterpret_cast<char*>(RectImg_Vector1D.data()));

            break;
        }

        RectImg_DownSampling = CFutoImg(RectImg_Vector1D, m_ImgSize_DownSample, 1);
    }

    //---Assistant Functions: Compute Orientations---
    void CPlanarRectification::cal_K_Rect(const glm::mat3& K_Orig_B, const glm::mat3& K_Orig_M)
    {
        m_K_Rect_B = 0.5 * (K_Orig_B + K_Orig_M);
        m_K_Rect_B[1].x = 0; // Let skew = 0
        m_K_Rect_M = m_K_Rect_B; // Camera mtx of both rectified images are the same.
    }

    void CPlanarRectification::cal_R_Rect(const glm::vec3& PC_Orig_B, const glm::vec3& PC_Orig_M, const glm::mat3& R_Orig_B)
    {
        glm::vec3 R_Rect_row0 = PC_Orig_M - PC_Orig_B;
        R_Rect_row0 = (R_Rect_row0.x + R_Rect_row0.y + R_Rect_row0.z) >= 0 ? R_Rect_row0 : -R_Rect_row0; // Keep RectImg always along with positive baseline direction
        R_Rect_row0 /= glm::l2Norm(R_Rect_row0);

        glm::vec3 R_Rect_row1 = glm::cross(glm::transpose(R_Orig_B)[2], R_Rect_row0);
        R_Rect_row1 /= glm::l2Norm(R_Rect_row1);

        glm::vec3 R_Rect_row2 = glm::cross(R_Rect_row0, R_Rect_row1);
        R_Rect_row2 /= glm::l2Norm(R_Rect_row2);

        m_R_Rect = glm::transpose(glm::mat3(R_Rect_row0, R_Rect_row1, R_Rect_row2));
    }

    void CPlanarRectification::cal_PC_Rect(const glm::vec3& PC_Orig_B, const glm::vec3& PC_Orig_M)
    {
        m_PC_Rect_B = PC_Orig_B;
        m_PC_Rect_M = PC_Orig_M;
    }

    void CPlanarRectification::cal_P_Rect()
    {
        m_P_Rect_B = m_K_Rect_B * glm::mat4x3(m_R_Rect[0], m_R_Rect[1], m_R_Rect[2], -m_R_Rect * m_PC_Rect_B);
        m_P_Rect_M = m_K_Rect_M * glm::mat4x3(m_R_Rect[0], m_R_Rect[1], m_R_Rect[2], -m_R_Rect * m_PC_Rect_M);
    }

    void CPlanarRectification::cal_H(const glm::mat4x3& P_Orig_B, const glm::mat4x3& P_Orig_M)
    {
        //---Calculate the Homography---
        // A simple way to calculate Homography (Also proposed by Fusiello). <= glm::inverse cannot apply on glm::mat4*3
        glm::mat3 H_B = glm::mat3(m_P_Rect_B) * glm::inverse(glm::mat3(P_Orig_B));
        glm::mat3 H_M = glm::mat3(m_P_Rect_M) * glm::inverse(glm::mat3(P_Orig_M));

        m_Homography_B.m_H_Orig2Rect = glm::mat4(H_B);
        m_Homography_B.m_H_Rect2Orig = glm::mat4(glm::inverse(H_B));

        m_Homography_M.m_H_Orig2Rect = glm::mat4(H_M);
        m_Homography_M.m_H_Rect2Orig = glm::mat4(glm::inverse(H_M));
    }

    //---Assistant Functions: Center Rectified Images---
    void CPlanarRectification::cal_CenterShift(glm::vec2& CenterDrift, const glm::ivec2& ImgSize_Orig, const int Which_Img)
    {
        glm::mat3 H;
        switch (Which_Img)
        {
        case 0: H = glm::mat3(m_Homography_B.m_H_Orig2Rect);
            break;
        case 1: H = glm::mat3(m_Homography_M.m_H_Orig2Rect);
            break;
        }
        
        glm::vec3 Center_Orig(ImgSize_Orig.x/2, ImgSize_Orig.y/2, 1.0f);

        glm::vec3 Center_Orig2Rect = H * Center_Orig;
        Center_Orig2Rect /= Center_Orig2Rect.z;

        CenterDrift = glm::vec2(Center_Orig - Center_Orig2Rect);
    }

    void CPlanarRectification::imp_CenterShift_K(const glm::vec2& Shift_B, const glm::vec2& Shift_M)
    {
        m_K_Rect_B[2].x += Shift_B.x;
        m_K_Rect_B[2].y += Shift_B.y;
        m_K_Rect_M[2].x += Shift_M.x;
        m_K_Rect_M[2].y += Shift_M.y;
    }

    //---Assistant Functions: Generate Rectified Images---
    void CPlanarRectification::cal_RectImgBound(const glm::ivec2& ImgSize_Orig, const int Which_Img)
    {
        glm::mat3 H;
        switch (Which_Img)
        {
        case 0: H = glm::mat3(m_Homography_B.m_H_Orig2Rect);
            break;
        case 1: H = glm::mat3(m_Homography_M.m_H_Orig2Rect);
            break;
        }

        //---Select corners in original images---
        glm::ivec3 ImgCnr_Orig_UL(0, 0, 1);
        glm::ivec3 ImgCnr_Orig_UR(ImgSize_Orig.x - 1, 0, 1);
        glm::ivec3 ImgCnr_Orig_DL(0, ImgSize_Orig.y - 1, 1);
        glm::ivec3 ImgCnr_Orig_DR(ImgSize_Orig.x - 1, ImgSize_Orig.y - 1, 1);

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
        glm::ivec2 ImgCnr_Rect_UL, ImgCnr_Rect_DR;

        ImgCnr_Rect_UL.x = 
            std::floor(std::min(std::min(ImgCnr_Orig2Rect_UL.x, ImgCnr_Orig2Rect_UR.x), std::min(ImgCnr_Orig2Rect_DL.x, ImgCnr_Orig2Rect_DR.x)));
        ImgCnr_Rect_UL.y =
            std::floor(std::min(std::min(ImgCnr_Orig2Rect_UL.y, ImgCnr_Orig2Rect_UR.y), std::min(ImgCnr_Orig2Rect_DL.y, ImgCnr_Orig2Rect_DR.y)));
        ImgCnr_Rect_DR.x =
            std::ceil(std::max(std::max(ImgCnr_Orig2Rect_UL.x, ImgCnr_Orig2Rect_UR.x), std::max(ImgCnr_Orig2Rect_DL.x, ImgCnr_Orig2Rect_DR.x)));
        ImgCnr_Rect_DR.y =
            std::ceil(std::max(std::max(ImgCnr_Orig2Rect_UL.y, ImgCnr_Orig2Rect_UR.y), std::max(ImgCnr_Orig2Rect_DL.y, ImgCnr_Orig2Rect_DR.y)));

        switch (Which_Img)
        {
        case 0: 
            m_Homography_B.m_RectImgConer_UL = ImgCnr_Rect_UL;
            m_Homography_B.m_RectImgConer_DR = ImgCnr_Rect_DR;
            break;
        case 1: 
            m_Homography_M.m_RectImgConer_UL = ImgCnr_Rect_UL;
            m_Homography_M.m_RectImgConer_DR = ImgCnr_Rect_DR;
            break;
        }
    }

    void CPlanarRectification::determ_RectFrame()
    {
        glm::ivec2 RectImgConerUL, RectImgConerDR;

        RectImgConerUL.x = m_Homography_B.m_RectImgConer_UL.x <= m_Homography_M.m_RectImgConer_UL.x ? m_Homography_B.m_RectImgConer_UL.x : m_Homography_M.m_RectImgConer_UL.x;
        RectImgConerUL.y = m_Homography_B.m_RectImgConer_UL.y <= m_Homography_M.m_RectImgConer_UL.y ? m_Homography_B.m_RectImgConer_UL.y : m_Homography_M.m_RectImgConer_UL.y;
        RectImgConerDR.x = m_Homography_B.m_RectImgConer_DR.x >= m_Homography_M.m_RectImgConer_DR.x ? m_Homography_B.m_RectImgConer_DR.x : m_Homography_M.m_RectImgConer_DR.x;
        RectImgConerDR.y = m_Homography_B.m_RectImgConer_DR.y >= m_Homography_M.m_RectImgConer_DR.y ? m_Homography_B.m_RectImgConer_DR.y : m_Homography_M.m_RectImgConer_DR.y;

        m_Homography_B.m_RectImgConer_UL = RectImgConerUL;
        m_Homography_B.m_RectImgConer_DR = RectImgConerDR;

        m_Homography_M.m_RectImgConer_UL = RectImgConerUL;
        m_Homography_M.m_RectImgConer_DR = RectImgConerDR;

        glm::uvec2 RectPlane = m_Homography_B.m_RectImgConer_DR - m_Homography_B.m_RectImgConer_UL;

        if (RectPlane.x > 5000 || RectPlane.y > 5000)
        {
            m_Is_LargeSize = true;
        }

        if (m_Is_FixSize)
        {
            glm::ivec2 AddShift = m_ImgSize_Rect - RectPlane;
            AddShift /= 2;

            m_Homography_B.m_RectImgConer_UL -= AddShift;
            m_Homography_B.m_RectImgConer_DR -= AddShift;

            m_Homography_M.m_RectImgConer_UL -= AddShift;
            m_Homography_M.m_RectImgConer_DR -= AddShift;
        }
        else
        {
            m_ImgSize_Rect = RectPlane;

            //---Initialize Output Texture Manager---
            Gfx::STextureDescriptor TextureDescriptor_RectImg = {};

            TextureDescriptor_RectImg.m_NumberOfPixelsU = m_ImgSize_Rect.x;
            TextureDescriptor_RectImg.m_NumberOfPixelsV = m_ImgSize_Rect.y;
            TextureDescriptor_RectImg.m_NumberOfPixelsW = 1;
            TextureDescriptor_RectImg.m_NumberOfMipMaps = 1;
            TextureDescriptor_RectImg.m_NumberOfTextures = 1;
            TextureDescriptor_RectImg.m_Binding = Gfx::CTexture::ShaderResource;
            TextureDescriptor_RectImg.m_Access = Gfx::CTexture::EAccess::CPUWrite;
            TextureDescriptor_RectImg.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
            TextureDescriptor_RectImg.m_Semantic = Gfx::CTexture::UndefinedSemantic;
            TextureDescriptor_RectImg.m_Format = Gfx::CTexture::R8_UBYTE; // 1 channels with 8-bit.

            m_RectImgB_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_RectImg);
            m_RectImgM_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_RectImg);
        }
    }

    void CPlanarRectification::genrt_RectImg(const std::vector<char>& Img_Orig, const glm::ivec2& ImgSize_Orig, const int Which_Img)
    {
        //---GPU Computation Start---
        Gfx::Performance::BeginEvent("Planar Rectification");

        //---Put Homography to Buffer & OrigImg to Texture---
        Base::AABB2UInt TargetRect;

        switch (Which_Img)
        {
        case 0: 
            Gfx::BufferManager::UploadBufferData(m_HomographyB_BufferPtr, &m_Homography_B);

            TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(ImgSize_Orig.x, ImgSize_Orig.y));
            Gfx::TextureManager::CopyToTexture2D(m_OrigImgB_TexturePtr, TargetRect, ImgSize_Orig.x, static_cast<const void*>(Img_Orig.data()));

            Gfx::ContextManager::SetShaderCS(m_PlanarRectCSPtr);
            Gfx::ContextManager::SetImageTexture(0, m_OrigImgB_TexturePtr);
            Gfx::ContextManager::SetImageTexture(1, m_RectImgB_TexturePtr);
            Gfx::ContextManager::SetConstantBuffer(0, m_HomographyB_BufferPtr);

            break;
        case 1: 
            Gfx::BufferManager::UploadBufferData(m_HomographyM_BufferPtr, &m_Homography_M);

            TargetRect = Base::AABB2UInt(glm::uvec2(0, 0), glm::uvec2(ImgSize_Orig.x, ImgSize_Orig.y));
            Gfx::TextureManager::CopyToTexture2D(m_OrigImgM_TexturePtr, TargetRect, ImgSize_Orig.x, static_cast<const void*>(Img_Orig.data()));

            Gfx::ContextManager::SetShaderCS(m_PlanarRectCSPtr);
            Gfx::ContextManager::SetImageTexture(0, m_OrigImgM_TexturePtr);
            Gfx::ContextManager::SetImageTexture(1, m_RectImgM_TexturePtr);
            Gfx::ContextManager::SetConstantBuffer(0, m_HomographyM_BufferPtr);

            break;
        }

        //---Start GPU Parallel Processing---
        const int WorkGroupsX = DivUp(m_ImgSize_Rect.x, TileSize_2D);
        const int WorkGroupsY = DivUp(m_ImgSize_Rect.y, TileSize_2D);

        Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        Gfx::ContextManager::ResetShaderCS();

        Gfx::Performance::EndEvent();
        //---GPU Computation End---
    }
} // FutoGmtCV
