#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\FutoGCV_Rectification_Planar.h"

#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_performance.h"

namespace // No specific namespace => Only allowed to use in this file.
{
    //---Definition for GPU Parallel Processing---
    #define TileSize_2D 16 // 16 is suggested work group size for 2D image (based on experience).

    int DivUp(int TotalShaderCount, int WorkGroupSize)
    {
        return (TotalShaderCount + WorkGroupSize - 1) / WorkGroupSize;
    }
}

namespace FutoGCV
{
    //===== Constructors & Destructor =====

    CPlanarRectification::CPlanarRectification(const glm::ivec2& _OrigImgSize)
        : m_OrigImgSize(_OrigImgSize) 
    {
        //---Initialize Shader Manager---
        std::stringstream DefineStream;
        DefineStream
            << "#define TILE_SIZE_2D " << TileSize_2D << " \n"; 
        std::string DefineString = DefineStream.str();

        m_PlanarRectificationCSPtr = Gfx::ShaderManager::CompileCS("../../plugins/stereo/cs_rectification_planar.glsl", "main", DefineString.c_str());
    }

    CPlanarRectification::~CPlanarRectification()
    {
        //---Release Manager---
        m_PlanarRectificationCSPtr = nullptr;
    }

    //===== Execution Functions =====

    void CPlanarRectification::execute(SFutoImg& EpiImg_B, Gfx::CBufferPtr HomoB_BufferPtr, 
                                       SFutoImg& EpiImg_M, Gfx::CBufferPtr HomoM_BufferPtr, 
                                       const SFutoImg& _OrigImg_B, const SFutoImg& _OrigImg_M)
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

        //---Step 3. Determine Epipolar Image Frame---
        ComputeEpiCorner(0);
        ComputeEpiCorner(1);

        DetermEpiImgSize(EpiImg_B.m_Img_TexturePtr, EpiImg_M.m_Img_TexturePtr);

        //---Step 4. Generate Epipolar Images---
        HomoTransform(EpiImg_B.m_Img_TexturePtr, HomoB_BufferPtr, _OrigImg_B.m_Img_TexturePtr, 0);
        HomoTransform(EpiImg_M.m_Img_TexturePtr, HomoM_BufferPtr, _OrigImg_M.m_Img_TexturePtr, 1);
    }

    //===== Assistant Functions =====

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
        glm::ivec3 CnrUL_Orig(0, 0, 1), 
                   CnrUR_Orig(m_OrigImgSize.x - 1, 0, 1), 
                   CnrDL_Orig(0, m_OrigImgSize.y - 1, 1), 
                   CnrDR_Orig(m_OrigImgSize.x - 1, m_OrigImgSize.y - 1, 1);

        //---Transform corners from original to rectified---
        glm::vec3 CnrUL_Orig2Epi = H * CnrUL_Orig;
        CnrUL_Orig2Epi /= CnrUL_Orig2Epi.z;

        glm::vec3 CnrUR_Orig2Epi = H * CnrUR_Orig;
        CnrUR_Orig2Epi /= CnrUR_Orig2Epi.z;

        glm::vec3 CnrDL_Orig2Epi = H * CnrDL_Orig;
        CnrDL_Orig2Epi /= CnrDL_Orig2Epi.z;

        glm::vec3 CnrDR_Orig2Epi = H * CnrDR_Orig;
        CnrDR_Orig2Epi /= CnrDR_Orig2Epi.z;

        //---Determine the Co-Corners of Epipolar Image---
        glm::ivec2 CnrUL_Epi, CnrDR_Epi;

        CnrUL_Epi.x =
            static_cast<int>(floor(glm::min(glm::min(CnrUL_Orig2Epi.x, CnrUR_Orig2Epi.x), glm::min(CnrDL_Orig2Epi.x, CnrDR_Orig2Epi.x))));
        CnrUL_Epi.y =
            static_cast<int>(floor(glm::min(glm::min(CnrUL_Orig2Epi.y, CnrUR_Orig2Epi.y), glm::min(CnrDL_Orig2Epi.y, CnrDR_Orig2Epi.y))));
        CnrDR_Epi.x =
            static_cast<int>(ceil(glm::max(glm::max(CnrUL_Orig2Epi.x, CnrUR_Orig2Epi.x), glm::max(CnrDL_Orig2Epi.x, CnrDR_Orig2Epi.x))));
        CnrDR_Epi.y =
            static_cast<int>(ceil(glm::max(glm::max(CnrUL_Orig2Epi.y, CnrUR_Orig2Epi.y), glm::max(CnrDL_Orig2Epi.y, CnrDR_Orig2Epi.y))));

        switch (Which_Img)
        {
        case 0:
            m_Homography_B.m_EpiCorner_UL = CnrUL_Epi;
            m_Homography_B.m_EpiCorner_DR = CnrDR_Epi;
            break;
        case 1:
            m_Homography_M.m_EpiCorner_UL = CnrUL_Epi;
            m_Homography_M.m_EpiCorner_DR = CnrDR_Epi;
            break;
        }
    }

    void CPlanarRectification::DetermEpiImgSize(Gfx::CTexturePtr EpiImg_B_TexturePtr, Gfx::CTexturePtr EpiImg_M_TexturePtr)
    {
        glm::ivec2 EpiFrameUL, EpiFrameDR;

        EpiFrameUL.x = m_Homography_B.m_EpiCorner_UL.x <= m_Homography_M.m_EpiCorner_UL.x ? m_Homography_B.m_EpiCorner_UL.x : m_Homography_M.m_EpiCorner_UL.x;
        EpiFrameUL.y = m_Homography_B.m_EpiCorner_UL.y <= m_Homography_M.m_EpiCorner_UL.y ? m_Homography_B.m_EpiCorner_UL.y : m_Homography_M.m_EpiCorner_UL.y;
        EpiFrameDR.x = m_Homography_B.m_EpiCorner_DR.x >= m_Homography_M.m_EpiCorner_DR.x ? m_Homography_B.m_EpiCorner_DR.x : m_Homography_M.m_EpiCorner_DR.x;
        EpiFrameDR.y = m_Homography_B.m_EpiCorner_DR.y >= m_Homography_M.m_EpiCorner_DR.y ? m_Homography_B.m_EpiCorner_DR.y : m_Homography_M.m_EpiCorner_DR.y;

        m_Homography_B.m_EpiCorner_UL = EpiFrameUL;
        m_Homography_B.m_EpiCorner_DR = EpiFrameDR;

        m_Homography_M.m_EpiCorner_UL = EpiFrameUL;
        m_Homography_M.m_EpiCorner_DR = EpiFrameDR;

        m_EpiImgSize = m_Homography_B.m_EpiCorner_DR - m_Homography_B.m_EpiCorner_UL;

        if (m_EpiImgSize.x > 5000 || m_EpiImgSize.y > 5000)
        {
            return;
        }

        //---Initialize EpiImg Texture Manager---
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

        EpiImg_B_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextDesc_EpiImg);
        EpiImg_M_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextDesc_EpiImg);
    }

    void CPlanarRectification::HomoTransform(Gfx::CTexturePtr EpiImg_TexturePtr, Gfx::CBufferPtr Homo_BufferPtr, Gfx::CTexturePtr OrigImg_TexturePtr, const int Which_Img)
    {
        Gfx::ContextManager::SetShaderCS(m_PlanarRectificationCSPtr);

        switch (Which_Img)
        {
        case 0:
            Gfx::BufferManager::UploadBufferData(Homo_BufferPtr, &m_Homography_B);

            break;

        case 1:
            Gfx::BufferManager::UploadBufferData(Homo_BufferPtr, &m_Homography_M);

            break;
        }

        Gfx::ContextManager::SetConstantBuffer(0, Homo_BufferPtr);

        Gfx::ContextManager::SetImageTexture(0, OrigImg_TexturePtr);
        Gfx::ContextManager::SetImageTexture(1, EpiImg_TexturePtr);

        //---GPU Computation Start---
        Gfx::Performance::BeginEvent("Planar Rectification");

        const int WorkGroupsX = DivUp(m_EpiImgSize.x, TileSize_2D);
        const int WorkGroupsY = DivUp(m_EpiImgSize.y, TileSize_2D);

        Gfx::ContextManager::Dispatch(WorkGroupsX, WorkGroupsY, 1);

        Gfx::ContextManager::ResetShaderCS();

        Gfx::Performance::EndEvent();
        //---GPU Computation End---
    }

} // FutoGmtCV
