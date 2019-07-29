
#pragma once

#include "engine/graphic/gfx_shader_manager.h" // To manage GPU Shader
#include "engine/graphic/gfx_texture_manager.h" // To manage GPU Texture (Data in GPU processing) 
#include "engine/graphic/gfx_buffer_manager.h" // To manage GPU Buffer (Memory in GPU) 

namespace FutoGCV
{
    enum EEpiType
    {
        NORMAL,
        SUBIMG,
        DOWNSAMPLE
    };

    struct SHomography // Always make sure whole structure is the multiple of 4*float
    {
        glm::mat4 m_H_Orig2Epi; // GPU memory transmission is based on 4. => Using mat3 will adress wrong memory.
        glm::mat4 m_H_Epi2Orig; // GPU memory transmission is based on 4. => Using mat3 will adress wrong memory.
        glm::ivec2 m_EpiCorner_UL; // pix_Epi = pix_Orig2Epi + CornerUL; <= pix_Orig2Epi = H * pix_Orig;
        glm::ivec2 m_EpiCorner_DR; // Makes Structure base on 4.
    };

    class CPlanarRectification
    {
    //---Constructors & Destructor---
    public:

        CPlanarRectification(const glm::ivec2& _OrigImgSiye, EEpiType _EpiType, const glm::ivec2 _EpiImgSize = glm::ivec2(0));

        ~CPlanarRectification();

    //---Execution Functions---
    public:

        void ComputeEpiGeometry(const SFutoImg& OrigImg_B, const SFutoImg& OrigImg_M);

    //---Assistant Functions---
    private:
        void ComputeEpiCamera(const glm::mat3& _OrigCamera_B, const glm::mat3& _OrigCamera_M);
        void ComputeEpiRotation(const glm::vec3& _OrigPosition_B, const glm::vec3& _OrigPosition_M, const glm::mat3& _OrigRotation_B);
        void ComputeEpiPosition(const glm::vec3& _OrigPosition_B, const glm::vec3& _OrigPosition_M);
        void ComputeEpiPPM();

        void ComputeHomography(const glm::mat4x3& _OrigPPM_B, const glm::mat4x3& _OrigPPM_M);

        void ComputeCenterShift(glm::vec2& CenterDrift, const int Which_Img = 0);
        void ShiftEpiCenter(const glm::vec2& Drift_B, const glm::vec2& Drift_M);

        void ComputeEpiCorner(const int Which_Img = 0);
        void DetermEpiImgSize();

        void GenrtEpiImg(Gfx::CTexturePtr _OrigImg_TexturePtr, const int Which_Img = 0);

    //---Members---
    private:

        EEpiType m_EpiType;

        glm::ivec2 m_OrigImgSize, m_EpiImgSize;

        glm::mat3 m_EpiCamera_B, m_EpiCamera_M; // Camera matrix of Epipolar Images
        glm::mat3 m_EpiRotation; // Rotation matrix of Epipolar Images (World -> Image)
        glm::vec3 m_EpiPosition_B, m_EpiPosition_M; // Projection Center of Epipolar Images
        glm::mat4x3 m_EpiPPM_B, m_EpiPPM_M; // Perspective Projection Matrix of Epipolar Images (World -> Image)

        SHomography m_Homography_B, m_Homography_M;

        Gfx::CShaderPtr m_PlanarRectCSPtr, m_DownSamplingCSPtr;

        Gfx::CTexturePtr m_OrigImgB_TexturePtr, m_OrigImgM_TexturePtr;
        Gfx::CTexturePtr m_EpiImgB_TexturePtr, m_EpiImgM_TexturePtr;



        Gfx::CTexturePtr m_RectImgB_DownSample_TexturePtr, m_RectImgM_DownSample_TexturePtr;

        Gfx::CBufferPtr m_HomographyB_BufferPtr, m_HomographyM_BufferPtr;


    // *** OLD ***

    //---Execution Functions---
    public:
        void execute(const CFutoImg& OrigImg_B, const CFutoImg& OrigImg_M);

        void return_Result(CFutoImg& RectImg_Curt, CFutoImg& RectImg_Last, SHomography& Homo_B, SHomography& Homo_M);

        void imp_DownSampling(CFutoImg& RectImg_DownSampling, const int Which_Img = 0);

        bool m_Is_LargeSize;


    //---Members---
    private:
        //---Image Data---
        CFutoImg m_Img_Rect_B, m_Img_Rect_M; // Rectified Images
        glm::ivec2 m_ImgSize_Rect, m_ImgSize_DownSample;
        bool m_Is_FixSize, m_Is_DownSample;

    };

} // namespace FutoGmtCV
