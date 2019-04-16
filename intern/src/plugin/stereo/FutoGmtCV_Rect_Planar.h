
#pragma once

#include "engine/graphic/gfx_shader_manager.h" // Managing GPU Shader by Pointer
#include "engine/graphic/gfx_texture_manager.h" // Managing GPU Texture (Data) by Pointer
#include "engine/graphic/gfx_buffer_manager.h" // Managing GPU Buffer (Memory) by Pointer

#include "plugin\stereo\FutoGmtCV_Img.h"

#include "opencv2/opencv.hpp"

namespace FutoGmtCV
{
    class PlanarRect
    {
    //---Constructor & Destructor---
    public:
        PlanarRect();
        ~PlanarRect();

    //---Execution Functions---
    public:
        void imp_PlanarRect();


    //---Assistant Functions---
    private:
        void cal_K_Rect();
        void cal_R_Rect();
        void cal_P_Rect();
        void cal_H();

        void imp_CenterRectImg();

        void determ_RectImgSize();
        void genrt_RectImg();


    //---Members---
    private:
        //---Epipolar Image---
        FutoImg m_Img_Orig_B, m_Img_Orig_M;
        cv::Mat m_Img_Rect_B, m_Img_Rect_M;

        cv::Mat m_K_Rect_B, m_K_Rect_M; // Camera mtx of Rectified Images
        cv::Mat m_R_Rect; // Rotation mtx of Rectified Images (World -> Image)
        cv::Mat m_P_Rect_B, m_P_Rect_M; // Perspective Projection mtx of Rectified Images (World -> Image)
        cv::Mat m_Homo_B, m_Homo_M; // Homography (Original -> Rectified)

        cv::Size m_ImgSize_Rect;
        cv::Point m_ImgCnr_UL_Rect, m_ImgCnr_DR_Rect;

        //---GPU Managers---
        Gfx::CShaderPtr m_CSPtr_PlanarRecr;
        Gfx::CTexturePtr m_TexturePtr_OrigImg;
        Gfx::CTexturePtr m_TexturePtr_RectImg;
        Gfx::CBufferPtr m_BufferPtr_Homography;

    //=====OLD=====
        
        

    //---Get Function---
    public:
        void get_K_Rect(cv::Mat &CamB_Rect, cv::Mat &CamM_Rect);
        void get_R_Rect(cv::Mat &Rot_Rect);
        void get_RectImg(cv::Mat& Output_RectImgB, cv::Mat& Output_RectImgM);
        void get_Transform_Orig2Rect(cv::Mat& LookUpTx_B_Orig2Rect, cv::Mat& LookUpTy_B_Orig2Rect, cv::Mat& LookUpTx_M_Orig2Rect, cv::Mat& LookUpTy_M_Orig2Rect);

    //---Members---
    private:
        
        //---Transformation---
        cv::Mat mapB_x_Orig2Rect, mapB_y_Orig2Rect, mapM_x_Orig2Rect, mapM_y_Orig2Rect;
        cv::Mat mapB_x_Rect2Orig, mapB_y_Rect2Orig, mapM_x_Rect2Orig, mapM_y_Rect2Orig;


    };

} // FutoGmtCV
