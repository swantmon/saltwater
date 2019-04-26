
#pragma once

#include "base/base_include_glm.h" // Some warnings appears when directly #include glm 
#include "opencv2/opencv.hpp"
#include "libsgm.h"

namespace FutoGmtCV
{
    class FutoImg
    {
    //---Constructor & Destructor---
    public:
        FutoImg();
        FutoImg(const std::vector<char>& Img_Input, const glm::ivec2& ImgSize);
        FutoImg(const std::vector<char>& Img_Input, const glm::ivec2& ImgSize, const glm::mat4x3& P);
        FutoImg(const std::vector<char>& Img_Input, const glm::ivec2& ImgSize, const glm::mat3& K, const glm::mat3& R, const glm::vec3& PC);
        ~FutoImg();

    //---Set Functions---
    public:
        void set_Cam(glm::mat3& K);
        void set_Rot(glm::mat3& R);
        void set_PC(glm::vec3& T);
        void set_PPM(glm::mat4x3& P_Input);

    //---Get Function---
    public:
        std::vector<char> get_Img() const;
        glm::ivec2 get_ImgSize() const;
        glm::mat3 get_Cam() const;
        glm::mat3 get_Rot() const;
        glm::vec3 get_PC() const;
        glm::mat4x3 get_PPM() const;
        
    //---Members---
    private:
        std::vector<char> m_Img; // Image data in RGBA
        glm::ivec2 m_ImgSize;
        glm::mat3 m_Cam_mtx; // Camera Matrix in pixel. Origin is upper-left. x-axis is row-direction and y-axis is column-direction
        glm::mat3 m_Rot_mtx; // Rotations from Mapping frame to Image frame
        glm::vec3 m_PC_vec; // Projection Center in Mapping frame
        glm::mat4x3 m_P_mtx; // Perspective Projection Matrix = K_mtx * [Rot_mtx | -Rot_mtx*PC_vec]

    //===OLD===

    //---Photogrammetric Computer Vision---
    public:

        //---Stereo Matching---
        void imp_cvSGBM(cv::Mat& DispImg, const cv::Mat& RectImg_Base, const cv::Mat& RectImg_Match);
    
    //---Operator Object---
    private:
        //---Stereo Matching---
        cv::Ptr<cv::StereoSGBM> operPtr_cvSGBM;


    };
} // FutoGmtCV

