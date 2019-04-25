
#pragma once

#include "engine/graphic/gfx_shader_manager.h" // To manage GPU Shader
#include "engine/graphic/gfx_texture_manager.h" // To manage GPU Texture (Data in GPU processing) 
#include "engine/graphic/gfx_buffer_manager.h" // To manage GPU Buffer (Memory in GPU) 

#include "plugin\stereo\FutoGmtCV_Img.h"

#include "opencv2/opencv.hpp"

namespace FutoGmtCV
{
    class Rectification_Planar
    {
    //---Constructor & Destructor---
    public:
        Rectification_Planar();
        Rectification_Planar(const cv::Size& OrigImgSize, const cv::Size& RectImgSize);
        ~Rectification_Planar();

    //---Execution Functions---
    public:
        void execute(FutoImg& Img_Rect_B, FutoImg& Img_Rect_M, const FutoImg& Img_Orig_B, const FutoImg& Img_Orig_M);

    //---Assistant Functions---
    private:
        void cal_K_Rect(const cv::Mat& K_Orig_B, const cv::Mat& K_Orig_M);
        void cal_R_Rect(const cv::Mat& PC_Orig_B, const cv::Mat& PC_Orig_M, const cv::Mat& R_Orig_B);
        void cal_PC_Rect(const cv::Mat& PC_Orig_B, const cv::Mat& PC_Orig_M);
        void cal_P_Rect();
        void cal_H(const cv::Mat& P_Orig_B, const cv::Mat& P_Orig_M);

        void cal_CneterShift(cv::Mat& CenterDrift, const cv::Size& ImgSize_Ori);
        void imp_CenterShift_K(const cv::Mat& Drift_B, const cv::Mat& Drift_Mt);

        void cal_RectImgBound(cv::Point& ImgCnr_Rect_UL, cv::Point& ImgCnr_Rect_DR, const cv::Size& ImgSize_Orig, const int Which_Img = 0);
        void determ_RectImgCnr(const cv::Point& ImgCnr_RectB_UL, const cv::Point& ImgCnr_RectB_DR, const cv::Point& ImgCnr_RectM_UL, const cv::Point& ImgCnr_RectM_DR);
        void genrt_RectImg(const cv::Mat& Img_Orig, const int Which_Img = 0);

        void get_RectImg(FutoImg& Img_Rect, const int Which_Img = 0);

    //---Members---
    private:
        //---Rectified Image---
        cv::Mat m_Img_Rect_B, m_Img_Rect_M; // Rectified Images
        cv::Size m_ImgSize_Orig, m_ImgSize_Rect;
        cv::Point m_ImgCnr_Rect_UL, m_ImgCnr_Rect_DR;

        //---Orientations---
        cv::Mat m_K_Rect_B, m_K_Rect_M; // Camera mtx of Rectified Images
        cv::Mat m_R_Rect; // Rotation mtx of Rectified Images (World -> Image)
        cv::Mat m_PC_Rect_B, m_PC_Rect_M; // Projection Center of Rectified Images
        cv::Mat m_P_Rect_B, m_P_Rect_M; // Perspective Projection mtx of Rectified Images (World -> Image)
        cv::Mat m_Homo_B, m_Homo_M; // Homography (Original -> Rectified)

        //---GLSL Managers---
        Gfx::CShaderPtr m_PlanarRectCSPtr;
        Gfx::CTexturePtr m_OrigImgTexturePtr;
        Gfx::CTexturePtr m_RectImgTexturePtr;
        Gfx::CBufferPtr m_HomographyBufferPtr;
    };

} // FutoGmtCV
