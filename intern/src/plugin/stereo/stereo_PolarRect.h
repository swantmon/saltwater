#pragma once

#ifndef PolarRect_H
#define PolarRect_H

#include <vector>

#include "base/base_include_glm.h" // Some warnings appears when directly #include glm 

#include "opencv2/opencv.hpp"

namespace Stereo
{
    class PolarRect
    {
    //---Constructor & Destructor---
    public:
        PolarRect();
        ~PolarRect();
    //---Main Function---
    private:
        void determ_CoRegion(const std::vector<cv::Point2f>& epipoles, const cv::Size imgDimensions, const cv::Mat & F);

    //---Assist Function---
    private:
        void getExternalPoints(const cv::Point2d& epipole, const cv::Size imgDimensions, std::vector<cv::Point2f>& externalPoints);
        void determ_RhoRange(const cv::Point2d& epipole, const cv::Size imgDimensions, const std::vector<cv::Point2f>& externalPoints, double& minRho, double& maxRho);

    //---Member---
    private:
        double m_minRho1, m_maxRho1, m_minRho2, m_maxRho2;
    };

}
#endif
