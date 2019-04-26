
#pragma once

#include "engine/graphic/gfx_shader_manager.h" // To manage GPU Shader
#include "engine/graphic/gfx_texture_manager.h" // To manage GPU Texture (Data in GPU processing) 
#include "engine/graphic/gfx_buffer_manager.h" // To manage GPU Buffer (Memory in GPU) 

#include "plugin\stereo\FutoGmtCV_Img.h"

#include "opencv2/opencv.hpp"

namespace FutoGmtCV
{
    struct SHomographyTransform // Always make sure whole structure is the multiple of 4*float
    {
        glm::mat4 m_H_Orig2Rect; // Transmition unit of layout(std140) is 4*float. => mat3 is only 3*3*float, which may transmit wrong memory address.
        glm::mat4 m_H_Rect2Orig;
        glm::ivec2 m_RectImgConer_UL; // pix_Rect = pix_Orig2Rect + Shift; <= pix_Orig2Rect = H * pix_Orig;
        glm::ivec2 m_RectImgConer_DR;
    };

    class CRectification_Planar
    {
    //---Constructor & Destructor---
    public:
        CRectification_Planar();
        CRectification_Planar(const glm::ivec2& OrigImgSize, const glm::ivec2& RectImgSize);
        ~CRectification_Planar();

    //---Execution Functions---
    public:
        void execute(FutoImg& Img_Rect_B, FutoImg& Img_Rect_M, const FutoImg& Img_Orig_B, const FutoImg& Img_Orig_M);

    //---Assistant Functions---
    private:
        void cal_K_Rect(const glm::mat3& K_Orig_B, const glm::mat3& K_Orig_M);
        void cal_R_Rect(const glm::vec3& PC_Orig_B, const glm::vec3& PC_Orig_M, const glm::mat3& R_Orig_B);
        void cal_PC_Rect(const glm::vec3& PC_Orig_B, const glm::vec3& PC_Orig_M);
        void cal_P_Rect();
        void cal_H(const glm::mat4x3& P_Orig_B, const glm::mat4x3& P_Orig_M);

        void cal_CenterShift(glm::vec2& CenterDrift, const glm::ivec2& ImgSize_Orig, const int Which_Img = 0);
        void imp_CenterShift_K(const glm::vec2& Drift_B, const glm::vec2& Drift_Mt);

        void cal_RectImgBound(const glm::ivec2& ImgSize_Orig, const int Which_Img = 0);
        void genrt_RectImg(const std::vector<char>& Img_Orig, const int Which_Img = 0);

        void get_RectImg(FutoImg& Img_Rect, const int Which_Img = 0);

    //---Members---
    private:
        //---Rectified Image---
        std::vector<char> m_Img_Rect_B, m_Img_Rect_M; // Rectified Images
        glm::ivec2 m_ImgSize_Orig, m_ImgSize_Rect;
        glm::ivec2 m_ImgCnr_Rect_UL, m_ImgCnr_Rect_DR;

        //---Homography---
        SHomographyTransform m_Homo_B, m_Homo_M; // Homography (Original -> Rectified)

        //---Orientations of Rectified Images---
        glm::mat3 m_K_Rect_B, m_K_Rect_M; // Camera mtx of Rectified Images
        glm::mat3 m_R_Rect; // Rotation mtx of Rectified Images (World -> Image)
        glm::vec3 m_PC_Rect_B, m_PC_Rect_M; // Projection Center of Rectified Images
        glm::mat4x3 m_P_Rect_B, m_P_Rect_M; // Perspective Projection mtx of Rectified Images (World -> Image)

        //---GLSL Managers---
        Gfx::CShaderPtr m_PlanarRectCSPtr;
        Gfx::CTexturePtr m_OrigImgTexturePtr;
        Gfx::CTexturePtr m_RectImgTexturePtr;
        Gfx::CBufferPtr m_HomographyBufferPtr;
    };

} // FutoGmtCV
