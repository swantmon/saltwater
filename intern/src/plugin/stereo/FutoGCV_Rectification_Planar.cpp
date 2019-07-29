#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\FutoGCV_Rectification_Planar.h"

#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_performance.h"

namespace // No specific namespace => Only allowed to use in this file.
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
    CPlanarRectification::CPlanarRectification(const glm::ivec2& _OrigImgSize, EEpiType _EpiType, const glm::ivec2 _EpiImgSize)
        : m_OrigImgSize(_OrigImgSize), 
          m_EpiType(_EpiType), 
          m_EpiImgSize(_EpiImgSize)
    {


        // *** OLD ***

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
        BufferDesc_Homo.m_NumberOfBytes = sizeof(SHomography);
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

        if (m_ImgSize_Rect == glm::ivec2(0))
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

            m_EpiImgB_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_RectImg);
            m_EpiImgM_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextureDescriptor_RectImg);
        }

        if (m_ImgSize_DownSample == glm::ivec2(0))
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

        m_EpiImgB_TexturePtr = nullptr;
        m_EpiImgM_TexturePtr = nullptr;

        m_HomographyB_BufferPtr = nullptr;
        m_HomographyM_BufferPtr = nullptr;
    }

    //---Execution Functions---
    void CPlanarRectification::ComputeEpiGeometry(const SFutoImg& _OrigImg_B, const SFutoImg& _OrigImg_M)
    {
        //---Step 1. Compute Orientations of Epipolar Images---
        ComputeEpiCamera(_OrigImg_B.m_Camera, _OrigImg_M.m_Camera);
        ComputeEpiRotation(_OrigImg_B.m_Position, _OrigImg_M.m_Position, _OrigImg_B.m_Rotation);
        ComputeEpiPosition(_OrigImg_B.m_Position, _OrigImg_M.m_Position);
        ComputeEpiPPM();

        ComputeHomography(_OrigImg_B.m_PPM, _OrigImg_M.m_PPM);

        //---Step 2. Center Epipolar Images---
        glm::vec2 CenterShift_B(0.0f), CenterShift_M(0.0f);

        ComputeCenterShift(CenterShift_B, 0);
        ComputeCenterShift(CenterShift_M, 1);

        float CenterShift_y = (CenterShift_B.y + CenterShift_M.y) / 2;
        CenterShift_B.y = CenterShift_y;
        CenterShift_M.y = CenterShift_y;

        ShiftEpiCenter(CenterShift_B, CenterShift_M);

        ComputeHomography(_OrigImg_B.m_PPM, _OrigImg_M.m_PPM); // Update Homography after shifting.

        //---Step 3. Calculate Boundary of Epipolar Images---
        ComputeEpiCorner(0);
        ComputeEpiCorner(1);

        DetermEpiImgSize();

        //---Step 4. Generate Epipolar Images---
        GenrtEpiImg(_OrigImg_B.m_Img_TexturePtr, 0);
        GenrtEpiImg(_OrigImg_M.m_Img_TexturePtr, 1);
    }

    //---Assistant Functions---
    void CPlanarRectification::ComputeEpiCamera(const glm::mat3& _OrigCamera_B, const glm::mat3& _OrigCamera_M)
    {
        m_EpiCamera_B = 0.5 * (_OrigCamera_B + _OrigCamera_M);
        m_EpiCamera_B[1].x = 0; // Let skew = 0

        m_EpiCamera_M = m_EpiCamera_B; 
    }

    void CPlanarRectification::ComputeEpiRotation(const glm::vec3 & _OrigPosition_B, const glm::vec3 & _OrigPosition_M, const glm::mat3 & _OrigRotation_B)
    {
        glm::vec3 EpiR_row0 = _OrigPosition_M - _OrigPosition_B;
        EpiR_row0 = (EpiR_row0.x + EpiR_row0.y + EpiR_row0.z) >= 0 ? EpiR_row0 : -EpiR_row0; // Keep EpiImg always along with positive baseline direction
        EpiR_row0 /= glm::l2Norm(EpiR_row0);

        glm::vec3 EpiR_row1 = glm::cross(glm::transpose(_OrigRotation_B)[2], EpiR_row0);
        EpiR_row1 /= glm::l2Norm(EpiR_row1);

        glm::vec3 EpiR_row2 = glm::cross(EpiR_row0, EpiR_row1);
        EpiR_row2 /= glm::l2Norm(EpiR_row2);

        m_EpiRotation = glm::transpose(glm::mat3(EpiR_row0, EpiR_row1, EpiR_row2));
    }

    void CPlanarRectification::ComputeEpiPosition(const glm::vec3 & _OrigPosition_B, const glm::vec3 & _OrigPosition_M)
    {
        m_EpiPosition_B = _OrigPosition_B;
        m_EpiPosition_M = _OrigPosition_M;
    }

    void CPlanarRectification::ComputeEpiPPM()
    {
        m_EpiPPM_B = m_EpiCamera_B * glm::mat4x3(m_EpiRotation[0], m_EpiRotation[1], m_EpiRotation[2], -m_EpiRotation * m_EpiPosition_B);
        m_EpiPPM_M = m_EpiCamera_M * glm::mat4x3(m_EpiRotation[0], m_EpiRotation[1], m_EpiRotation[2], -m_EpiRotation * m_EpiPosition_M);
    }

    void CPlanarRectification::ComputeHomography(const glm::mat4x3 & _OrigPPM_B, const glm::mat4x3 & _OrigPPM_M)
    {
        //---Calculate the Homography---
        // A simple way to calculate Homography (Also proposed by Fusiello). <= glm::inverse cannot apply on glm::mat4*3
        glm::mat3 H_B = glm::mat3(m_EpiPPM_B) * glm::inverse(glm::mat3(_OrigPPM_B));
        glm::mat3 H_M = glm::mat3(m_EpiPPM_M) * glm::inverse(glm::mat3(_OrigPPM_M));

        m_Homography_B.m_H_Orig2Epi = glm::mat4(H_B);
        m_Homography_B.m_H_Epi2Orig = glm::mat4(glm::inverse(H_B));

        m_Homography_M.m_H_Orig2Epi = glm::mat4(H_M);
        m_Homography_M.m_H_Epi2Orig = glm::mat4(glm::inverse(H_M));
    }

    void CPlanarRectification::ComputeCenterShift(glm::vec2& CenterDrift, const int Which_Img)
    {
        glm::mat3 H;
        switch (Which_Img)
        {
        case 0: H = glm::mat3(m_Homography_B.m_H_Orig2Epi);
            break;
        case 1: H = glm::mat3(m_Homography_M.m_H_Orig2Epi);
            break;
        }

        glm::vec3 Center_Orig(m_OrigImgSize.x / 2, m_OrigImgSize.y / 2, 1.0f);

        glm::vec3 Center_Orig2Epi = H * Center_Orig;
        Center_Orig2Epi /= Center_Orig2Epi.z;

        CenterDrift = glm::vec2(Center_Orig - Center_Orig2Epi);
    }

    void CPlanarRectification::ShiftEpiCenter(const glm::vec2 & Shift_B, const glm::vec2 & Shift_M)
    {
        m_EpiCamera_B[2].x += Shift_B.x;
        m_EpiCamera_B[2].y += Shift_B.y;
        m_EpiCamera_M[2].x += Shift_M.x;
        m_EpiCamera_M[2].y += Shift_M.y;
    }

    void CPlanarRectification::ComputeEpiCorner(const int Which_Img)
    {
        glm::mat3 H;
        switch (Which_Img)
        {
        case 0: H = glm::mat3(m_Homography_B.m_H_Orig2Epi);
            break;
        case 1: H = glm::mat3(m_Homography_M.m_H_Orig2Epi);
            break;
        }

        //---Select corners in original images---
        glm::ivec3 ImgCnr_Orig_UL(0, 0, 1), 
                   ImgCnr_Orig_UR(m_OrigImgSize.x - 1, 0, 1), 
                   ImgCnr_Orig_DL(0, m_OrigImgSize.y - 1, 1), 
                   ImgCnr_Orig_DR(m_OrigImgSize.x - 1, m_OrigImgSize.y - 1, 1);

        //---Transform corners from original to rectified---
        glm::vec3 ImgCnr_Orig2Epi_UL = H * ImgCnr_Orig_UL;
        ImgCnr_Orig2Epi_UL /= ImgCnr_Orig2Epi_UL.z;

        glm::vec3 ImgCnr_Orig2Epi_UR = H * ImgCnr_Orig_UR;
        ImgCnr_Orig2Epi_UR /= ImgCnr_Orig2Epi_UR.z;

        glm::vec3 ImgCnr_Orig2Epi_DL = H * ImgCnr_Orig_DL;
        ImgCnr_Orig2Epi_DL /= ImgCnr_Orig2Epi_DL.z;

        glm::vec3 ImgCnr_Orig2Epi_DR = H * ImgCnr_Orig_DR;
        ImgCnr_Orig2Epi_DR /= ImgCnr_Orig2Epi_DR.z;

        //---Determine the Co-Corners of Epipolar Image---
        glm::ivec2 ImgCnr_Epi_UL, ImgCnr_Epi_DR;

        ImgCnr_Epi_UL.x =
            static_cast<int>(floor(glm::min(glm::min(ImgCnr_Orig2Epi_UL.x, ImgCnr_Orig2Epi_UR.x), glm::min(ImgCnr_Orig2Epi_DL.x, ImgCnr_Orig2Epi_DR.x))));
        ImgCnr_Epi_UL.y =
            static_cast<int>(floor(glm::min(glm::min(ImgCnr_Orig2Epi_UL.y, ImgCnr_Orig2Epi_UR.y), glm::min(ImgCnr_Orig2Epi_DL.y, ImgCnr_Orig2Epi_DR.y))));
        ImgCnr_Epi_DR.x =
            static_cast<int>(ceil(glm::max(glm::max(ImgCnr_Orig2Epi_UL.x, ImgCnr_Orig2Epi_UR.x), glm::max(ImgCnr_Orig2Epi_DL.x, ImgCnr_Orig2Epi_DR.x))));
        ImgCnr_Epi_DR.y =
            static_cast<int>(ceil(glm::max(glm::max(ImgCnr_Orig2Epi_UL.y, ImgCnr_Orig2Epi_UR.y), glm::max(ImgCnr_Orig2Epi_DL.y, ImgCnr_Orig2Epi_DR.y))));

        switch (Which_Img)
        {
        case 0:
            m_Homography_B.m_EpiCorner_UL = ImgCnr_Epi_UL;
            m_Homography_B.m_EpiCorner_DR = ImgCnr_Epi_DR;
            break;
        case 1:
            m_Homography_M.m_EpiCorner_UL = ImgCnr_Epi_UL;
            m_Homography_M.m_EpiCorner_DR = ImgCnr_Epi_DR;
            break;
        }
    }

    void CPlanarRectification::DetermEpiImgSize()
    {
        glm::ivec2 EpiImgConerUL, EpiImgConerDR;

        EpiImgConerUL.x = m_Homography_B.m_EpiCorner_UL.x <= m_Homography_M.m_EpiCorner_UL.x ? m_Homography_B.m_EpiCorner_UL.x : m_Homography_M.m_EpiCorner_UL.x;
        EpiImgConerUL.y = m_Homography_B.m_EpiCorner_UL.y <= m_Homography_M.m_EpiCorner_UL.y ? m_Homography_B.m_EpiCorner_UL.y : m_Homography_M.m_EpiCorner_UL.y;
        EpiImgConerDR.x = m_Homography_B.m_EpiCorner_DR.x >= m_Homography_M.m_EpiCorner_DR.x ? m_Homography_B.m_EpiCorner_DR.x : m_Homography_M.m_EpiCorner_DR.x;
        EpiImgConerDR.y = m_Homography_B.m_EpiCorner_DR.y >= m_Homography_M.m_EpiCorner_DR.y ? m_Homography_B.m_EpiCorner_DR.y : m_Homography_M.m_EpiCorner_DR.y;

        m_Homography_B.m_EpiCorner_UL = EpiImgConerUL;
        m_Homography_B.m_EpiCorner_DR = EpiImgConerDR;

        m_Homography_M.m_EpiCorner_UL = EpiImgConerUL;
        m_Homography_M.m_EpiCorner_DR = EpiImgConerDR;

        glm::ivec2 EpiImgFrame = m_Homography_B.m_EpiCorner_DR - m_Homography_B.m_EpiCorner_UL;

        if (EpiImgFrame.x > 5000 || EpiImgFrame.y > 5000)
        {
            m_Is_LargeSize = true;
        }

        if (m_EpiType == SUBIMG)
        {
            glm::ivec2 SubShift = m_EpiImgSize - EpiImgFrame;
            SubShift /= 2;

            m_Homography_B.m_EpiCorner_UL -= SubShift;
            m_Homography_B.m_EpiCorner_DR -= SubShift;

            m_Homography_M.m_EpiCorner_UL -= SubShift;
            m_Homography_M.m_EpiCorner_DR -= SubShift;
        }
        else if (m_EpiType == DOWNSAMPLE)
        {

        }
        else
        {
            m_EpiImgSize = EpiImgFrame;

            //---Initialize Output Texture Manager---
            Gfx::STextureDescriptor TextDesc_EpiImg = {};

            TextDesc_EpiImg.m_NumberOfPixelsU = m_EpiImgSize.x;
            TextDesc_EpiImg.m_NumberOfPixelsV = m_EpiImgSize.y;
            TextDesc_EpiImg.m_NumberOfPixelsW = 1;
            TextDesc_EpiImg.m_NumberOfMipMaps = 1;
            TextDesc_EpiImg.m_NumberOfTextures = 1;
            TextDesc_EpiImg.m_Binding = Gfx::CTexture::ShaderResource;
            TextDesc_EpiImg.m_Access = Gfx::CTexture::EAccess::CPUWrite;
            TextDesc_EpiImg.m_Usage = Gfx::CTexture::EUsage::GPUReadWrite;
            TextDesc_EpiImg.m_Semantic = Gfx::CTexture::UndefinedSemantic;
            TextDesc_EpiImg.m_Format = Gfx::CTexture::R8_UBYTE; // 1 channels with 8-bit.

            m_EpiImgB_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextDesc_EpiImg);
            m_EpiImgM_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextDesc_EpiImg);
        }
    }

    void CPlanarRectification::GenrtEpiImg(Gfx::CTexturePtr _OrigImg_TexturePtr, const int Which_Img)
    {
        Gfx::ContextManager::SetShaderCS(m_PlanarRectCSPtr);

        Gfx::ContextManager::SetImageTexture(0, _OrigImg_TexturePtr);

        switch (Which_Img)
        {
        case 0:
            Gfx::BufferManager::UploadBufferData(m_HomographyB_BufferPtr, &m_Homography_B);
            Gfx::ContextManager::SetConstantBuffer(0, m_HomographyB_BufferPtr);

            Gfx::ContextManager::SetImageTexture(1, m_EpiImgB_TexturePtr);

            break;
        case 1:
            Gfx::BufferManager::UploadBufferData(m_HomographyM_BufferPtr, &m_Homography_M);
            Gfx::ContextManager::SetConstantBuffer(0, m_HomographyM_BufferPtr);

            Gfx::ContextManager::SetImageTexture(1, m_EpiImgM_TexturePtr);

            break;
        }

        //---GPU Computation Start---
        Gfx::Performance::BeginEvent("Planar Rectification");

        const int WorkGroupsX = DivUp(m_ImgSize_Rect.x, TileSize_2D);
        const int WorkGroupsY = DivUp(m_ImgSize_Rect.y, TileSize_2D);

        Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        Gfx::ContextManager::ResetShaderCS();

        Gfx::Performance::EndEvent();
        //---GPU Computation End---
    }


    // *** OLD ***

    void CPlanarRectification::return_Result(CFutoImg& RectImgB, CFutoImg& RectImgM, SHomography& Homo_B, SHomography& Homo_M)
    {
        const auto RectImgSize_1D = m_ImgSize_Rect.x * m_ImgSize_Rect.y;
        std::vector<char> RectImgB_Vector1D(RectImgSize_1D, 0), RectImgM_Vector1D(RectImgSize_1D, 0);
        Gfx::TextureManager::CopyTextureToCPU(m_EpiImgB_TexturePtr, reinterpret_cast<char*>(RectImgB_Vector1D.data()));
        Gfx::TextureManager::CopyTextureToCPU(m_EpiImgM_TexturePtr, reinterpret_cast<char*>(RectImgM_Vector1D.data()));


        RectImgB = CFutoImg(RectImgB_Vector1D, m_ImgSize_Rect, 1, m_EpiCamera_B, m_EpiRotation, m_EpiPosition_B);
        RectImgM = CFutoImg(RectImgM_Vector1D, m_ImgSize_Rect, 1, m_EpiCamera_M, m_EpiRotation, m_EpiPosition_M);

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
            Gfx::ContextManager::SetImageTexture(0, m_EpiImgB_TexturePtr);
            Gfx::ContextManager::SetImageTexture(1, m_RectImgB_DownSample_TexturePtr);

            break;

        case 1:
            Gfx::ContextManager::SetImageTexture(0, m_EpiImgM_TexturePtr);
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

} // FutoGmtCV
