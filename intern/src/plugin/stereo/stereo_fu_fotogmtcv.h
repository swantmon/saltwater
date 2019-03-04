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
    public:
        Fu_FotoGmtCV();
        Fu_FotoGmtCV(const std::vector<char>&, int, int);
        ~Fu_FotoGmtCV();
    
    public:
        cv::Mat PolarRect();

    public:
        void setPmtx(glm::mat4x3 P);
        void ShowImg(); 

    private:
        cv::Mat FuImg; // Original Image in RGB or RGBA
        cv::Mat FuImg_Rect; // Rectified Image
        glm::mat3 K_mtx;
        glm::mat4x3 P_mtx;
    };
} // Stereo

#endif
