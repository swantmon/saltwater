#pragma once

#ifndef Fu_FotoGmtCV_H
#define Fu_FotoGmtCV_H

#include <vector>

#include "base/base_include_glm.h" // Some warnings appears when directly #include glm 

#include "opencv2/opencv.hpp"

namespace Stereo
{
    class Fu_FotoGmtCV
    {
    //---Constructor & Destructor---
    public:
        Fu_FotoGmtCV();
        Fu_FotoGmtCV(const std::vector<char>&, int ImgW, int ImgH);
        ~Fu_FotoGmtCV();
    //---Epipolarization: Polar Rectification-----
    public:
        cv::Mat PolarRect(Fu_FotoGmtCV Img_Match);
        void DetermCoRegion();

    public:
        void set_Rot(glm::mat3 R);
        void set_Trans(glm::vec3 T);
        void set_P(glm::mat4x3 P);
        void ShowImg(); 

    private:
        cv::Mat Img; // Original Image in RGB or RGBA
        cv::Mat Img_Rect; // Rectified Image
        glm::mat3 K_mtx;
        glm::mat3 Rot_mtx; // Rotations from Mapping frame to Image frame
        glm::vec3 Trans_vec; // Translations in Mapping frame
        glm::mat4x3 P_mtx; // P-matrix = [Rot_mtx | -Rot_mtx*Trans_vec]
            //glm is column-major, which makes glm::mat4*3 represents real 3*4 matrix
            // Verification: glm::mat3x2 * glm::mat2x3 = glm::mat2
                /*
                glm::mat3x2 A = glm::mat3x2(glm::vec2(1, 2), glm::vec2(2, 3), glm::vec2(1, 2));
                glm::mat2x3 B = glm::mat2x3(glm::vec3(0, 1, 2), glm::vec3(2, 1, 0));
                glm::mat2 Ans = A * B;
                */
    };
} // Stereo

#endif
