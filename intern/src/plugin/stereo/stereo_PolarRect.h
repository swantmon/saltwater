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
        PolarRect(const cv::Mat& Img_B, const cv::Mat& Img_M);
        ~PolarRect();
    
    //---Main Function---
    public:
        void compute(const cv::Mat& F);
        void genrt_RectImg(int interpolation = cv::INTER_CUBIC);
        void get_RectImg(cv::Mat& RectImg_B, cv::Mat& RectImg_M);
    private:
        void cal_CoRegion(const std::vector<cv::Point2f>& EpiPoles, const cv::Size ImgSize, const cv::Mat& F);
            // Determine the Common Region = Determine the size of Epipolar Image
        void get_TransformImgPts(const cv::Size& imgSize, const cv::Point2d EpiPole1, const cv::Point2d EpiPole2, const cv::Mat& F);
            // Derive the Image Points along Epipolar Line
        void do_Transform(const cv::Mat& img1, const cv::Mat& img2, const cv::Point2d epipole1, const cv::Point2d epipole2, const cv::Mat& F);
            // Polar Transformation

    //---Assist Function---
    private:
        void get_ExternPts(const cv::Point2d& EpiPole, const cv::Size ImgSize, std::vector<cv::Point2f>& ImgPt_Extern);
        void cal_RhoRange(const cv::Point2d& EpiPole, const cv::Size ImgSize, const std::vector<cv::Point2f>& ImgPt_Extern, float& minRho, float& maxRho);

        
        cv::Vec3f get_ImgLn_from_ImgPt(const cv::Point2d& ImgPt1, const cv::Point2d& ImgPt2);
        void computeEpilines(const std::vector<cv::Point2f> & points, const uint32_t &whichImage, const cv::Mat & F, const std::vector <cv::Vec3f> & oldlines, std::vector <cv::Vec3f> & newLines);
            // Derive Epipolar Line on Image according to Image Point on the other Image.
        void getNewPointAndLineSingleImage(const cv::Point2d epipole1, const cv::Point2d epipole2, const cv::Size & imgDimensions, const cv::Mat & F, const uint32_t & whichImage, const cv::Point2d & pOld1, const cv::Point2d & pOld2, 
                                           cv::Vec3f & prevLine, cv::Point2d & pNew1, cv::Vec3f & newLine1, cv::Point2d & pNew2, cv::Vec3f & newLine2);
        void getNewEpiline(const cv::Point2d epipole1, const cv::Point2d epipole2, const cv::Size & imgDimensions, const cv::Mat & F, const cv::Point2d pOld1, const cv::Point2d pOld2,
                           cv::Vec3f prevLine1, cv::Vec3f prevLine2, cv::Point2d & pNew1, cv::Point2d & pNew2, cv::Vec3f & newLine1, cv::Vec3f & newLine2);
        void transformLine(const cv::Point2d& epipole, const cv::Point2d& p2, const cv::Mat& inputImage, const uint32_t & thetaIdx, const double &minRho, const double & maxRho, cv::Mat& mapX, cv::Mat& mapY, cv::Mat& inverseMapX, cv::Mat& inverseMapY);
            // Transformation between Cartisian & Polar

        bool lineIntersectsRect(const cv::Vec3d & line, const cv::Size & imgDimensions, cv::Point2d * intersection = NULL);
            // The Intersection(交集) of Epipolar lines on Image -> To derive External Epipolar Line.
        bool lineIntersectsSegment(const cv::Vec3d & line, const cv::Point2d & p1, const cv::Point2d & p2, cv::Point2d * intersection = NULL);
        cv::Point2d getBorderIntersection(const cv::Point2d & epipole, const cv::Vec3d & line, const cv::Size & imgDimensions, const cv::Point2d * lastPoint = NULL);
            // The Intersection of External Epipolar Line and Image Border.
        void getBorderIntersections(const cv::Point2d& epipole, const cv::Vec3d& line, const cv::Size& imgDimensions, std::vector<cv::Point2d>& intersections);
        cv::Point2d getNearestIntersection(const cv::Point2d & oldEpipole, const cv::Point2d & newEpipole, const cv::Vec3d & line, const cv::Point2d & oldPoint, const cv::Size & imgDimensions);

        //---Basic Computation---
        void get_Epipoles(const cv::Mat& F, cv::Point2f& epipole1, cv::Point2f& epipole2);
        bool Is_InsideImg(cv::Point2d ImgPt, cv::Size ImgSize);
        bool Is_TheRightPoint(const cv::Point2d & epipole, const cv::Point2d & intersection, const cv::Vec3d & line, const cv::Point2d * lastPoint);

    //---Member---
    private:
        cv::Mat ImgB_Orig, ImgM_Orig, ImgB_Rect, ImgM_Rect; // Original & Rectified Images

        cv::Point2d m_b1, m_b2, m_e1, m_e2; // The Beginning & Ending Point of Common Region in Image_Base & Image_Match.
        cv::Vec3f m_line1B, m_line1E, m_line2B, m_line2E; // Beginning & Ending of Epipolar Line in Image_Base & Image_Match
        float m_Rho_ImgB_min, m_Rho_ImgB_max, m_Rho_ImgM_min, m_Rho_ImgM_max; // Min & Max distance from EpiPole to Image Plane.

        std::vector<cv::Point2d> m_ThetaPts_ImgB, m_ThetaPts_ImgM; // Rows in Epipolar Images.

        cv::Mat m_mapX1, m_mapY1, m_mapX2, m_mapY2; // Look-up Table from Rectified to Original (To get pixel value from Original).
        cv::Mat m_inverseMapX1, m_inverseMapY1, m_inverseMapX2, m_inverseMapY2; // Look-up Table from Original to Rectified.

        double m_stepSize; // ???
    };

}
#endif
