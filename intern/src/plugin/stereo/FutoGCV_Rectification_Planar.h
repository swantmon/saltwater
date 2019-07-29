
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
        void ComputeEpiCamera(const glm::mat3& K_Orig_B, const glm::mat3& K_Orig_M);
        void ComputeEpiRotation(const glm::vec3& PC_Orig_B, const glm::vec3& PC_Orig_M, const glm::mat3& R_Orig_B);
        void ComputeEpiPC(const glm::vec3& PC_Orig_B, const glm::vec3& PC_Orig_M);
        void ComputeEpiPPM();
        void ComputeHomography(const glm::mat4x3& P_Orig_B, const glm::mat4x3& P_Orig_M);


    //---Members---
    private:

        EEpiType m_EpiType;

        glm::ivec2 m_OrigImgSize, m_EpiImgSize;

        glm::mat3 m_EpiCamera_B, m_EpiCamera_M; // Camera matrix of Epipolar Images
        glm::mat3 m_EpiRotation; // Rotation matrix of Epipolar Images (World -> Image)
        glm::vec3 m_EpiPosition_B, m_EpiPosition_M; // Projection Center of Epipolar Images
        glm::mat4x3 m_EpiPPM_B, m_EpiPPM_M; // Perspective Projection Matrix of Epipolar Images (World -> Image)

    // *** OLD ***

    //---Execution Functions---
    public:
        void execute(const CFutoImg& OrigImg_B, const CFutoImg& OrigImg_M);

        void return_Result(CFutoImg& RectImg_Curt, CFutoImg& RectImg_Last, SHomography& Homo_B, SHomography& Homo_M);

        void imp_DownSampling(CFutoImg& RectImg_DownSampling, const int Which_Img = 0);

        bool m_Is_LargeSize;

    //---Assistant Functions---
    private:
        

        void cal_CenterShift(glm::vec2& CenterDrift, const glm::ivec2& ImgSize_Orig, const int Which_Img = 0);
        void imp_CenterShift_K(const glm::vec2& Drift_B, const glm::vec2& Drift_Mt);

        void cal_RectImgBound(const glm::ivec2& ImgSize_Orig, const int Which_Img = 0);
        void determ_RectFrame();
        void genrt_RectImg(const std::vector<char>& Img_Orig, const glm::ivec2& ImgSize_Orig, const int Which_Img = 0);

    //---Members---
    private:
        //---Image Data---
        CFutoImg m_Img_Rect_B, m_Img_Rect_M; // Rectified Images
        glm::ivec2 m_ImgSize_Rect, m_ImgSize_DownSample;
        bool m_Is_FixSize, m_Is_DownSample;

        //---Homography---
        SHomography m_Homography_B, m_Homography_M; 


        //---GLSL Managers---
        Gfx::CShaderPtr m_PlanarRectCSPtr, m_DownSamplingCSPtr;

        Gfx::CTexturePtr m_OrigImgB_TexturePtr, m_OrigImgM_TexturePtr;
        Gfx::CTexturePtr m_RectImgB_TexturePtr, m_RectImgM_TexturePtr;
        Gfx::CTexturePtr m_RectImgB_DownSample_TexturePtr, m_RectImgM_DownSample_TexturePtr;

        Gfx::CBufferPtr m_HomographyB_BufferPtr, m_HomographyM_BufferPtr;
    };

} // namespace FutoGmtCV
