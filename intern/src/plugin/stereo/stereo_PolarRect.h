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
        void getTransformationPoints(const cv::Size& imgDimensions, const cv::Point2d epipole1, const cv::Point2d epipole2, const cv::Mat& F);
        void doTransformation(const cv::Mat& img1, const cv::Mat& img2, const cv::Point2d epipole1, const cv::Point2d epipole2, const cv::Mat& F);

    //---Assist Function---
    private:
        void getExternalPoints(const cv::Point2d& EpiPole, const cv::Size ImgSize, std::vector<cv::Point2f>& ImgPt_Extern);
        void determ_RhoRange(const cv::Point2d& EpiPole, const cv::Size ImgSize, const std::vector<cv::Point2f>& ImgPt_Extern, double& minRho, double& maxRho);

        cv::Vec3f get_ImgLn_from_ImgPt(const cv::Point2d& ImgPt1, const cv::Point2d& ImgPt2);
        void computeEpilines(const std::vector<cv::Point2f> & points, const uint32_t &whichImage, const cv::Mat & F, const std::vector <cv::Vec3f> & oldlines, std::vector <cv::Vec3f> & newLines);
        void getNewPointAndLineSingleImage(const cv::Point2d epipole1, const cv::Point2d epipole2, const cv::Size & imgDimensions, const cv::Mat & F, const uint32_t & whichImage, const cv::Point2d & pOld1, const cv::Point2d & pOld2, 
                                           cv::Vec3f & prevLine, cv::Point2d & pNew1, cv::Vec3f & newLine1, cv::Point2d & pNew2, cv::Vec3f & newLine2);
        void getNewEpiline(const cv::Point2d epipole1, const cv::Point2d epipole2, const cv::Size & imgDimensions, const cv::Mat & F, const cv::Point2d pOld1, const cv::Point2d pOld2,
                           cv::Vec3f prevLine1, cv::Vec3f prevLine2, cv::Point2d & pNew1, cv::Point2d & pNew2, cv::Vec3f & newLine1, cv::Vec3f & newLine2);
        void transformLine(const cv::Point2d& epipole, const cv::Point2d& p2, const cv::Mat& inputImage, const uint32_t & thetaIdx, const double &minRho, const double & maxRho, cv::Mat& mapX, cv::Mat& mapY, cv::Mat& inverseMapX, cv::Mat& inverseMapY);

        bool lineIntersectsRect(const cv::Vec3d & line, const cv::Size & imgDimensions, cv::Point2d * intersection = NULL);
        bool lineIntersectsSegment(const cv::Vec3d & line, const cv::Point2d & p1, const cv::Point2d & p2, cv::Point2d * intersection = NULL);
        cv::Point2d getBorderIntersection(const cv::Point2d & epipole, const cv::Vec3d & line, const cv::Size & imgDimensions, const cv::Point2d * lastPoint = NULL);
        void getBorderIntersections(const cv::Point2d& epipole, const cv::Vec3d& line, const cv::Size& imgDimensions, std::vector<cv::Point2d>& intersections);
        cv::Point2d getNearestIntersection(const cv::Point2d & oldEpipole, const cv::Point2d & newEpipole, const cv::Vec3d & line, const cv::Point2d & oldPoint, const cv::Size & imgDimensions);

    //---Basic Operation Function
    private:
        bool Is_InsideImg(cv::Point2d ImgPt, cv::Size ImgSize);
        bool Is_TheRightPoint(const cv::Point2d & epipole, const cv::Point2d & intersection, const cv::Vec3d & line, const cv::Point2d * lastPoint);

    //---Member---
    private:
        cv::Vec3f m_line1B, m_line1E, m_line2B, m_line2E; // Beginning & Ending of Image Line
        cv::Point2d m_b1, m_b2, m_e1, m_e2; // ??? Border of Epipolar Image ???
        double m_minRho1, m_maxRho1, m_minRho2, m_maxRho2; // Min & Max distance from EpiPole to Image Plane.

        cv::Mat m_mapX1, m_mapY1, m_mapX2, m_mapY2;
        cv::Mat m_inverseMapX1, m_inverseMapY1, m_inverseMapX2, m_inverseMapY2;

        double m_stepSize; // ???
        std::vector<cv::Point2d> m_thetaPoints1, m_thetaPoints2; // ???
    };

}
#endif
