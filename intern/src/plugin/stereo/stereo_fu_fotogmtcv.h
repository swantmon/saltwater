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

    private:
        void determ_CoRegion(const std::vector<cv::Point2f>& EpiPoles, const cv::Size ImgSize, const cv::Mat& F);
        void get_ExternPt(const cv::Point2f & EpiPole, const cv::Size ImgSize, std::vector<cv::Point2f> & ExternPt);
        void determ_RhoRange(const cv::Point2f & EpiPole, const cv::Size ImgSize, std::vector<cv::Point2f> & ImgCorner, float & minRho, float & maxRho);
        void get_ImgLn_from_ImgPt(const cv::Point2f & ImgPt1, const cv::Point2f & ImgPt2, cv::Vec3f ImgLn);
        void cal_EpiLn(const std::vector<cv::Point2f>& ImgPts, const uint whichImg, const cv::Mat F_mtx, const std::vector<cv::Vec3f>& ImgLn_old, std::vector<cv::Vec3f>& ImgLn_new);
        bool LnIntersectRect(const cv::Vec3d & ImgLn, const cv::Size & ImgSize, cv::Point2d * intersection);
        bool LnIntersectsSegment(const cv::Vec3d & ImgLn, const cv::Point2d & ImgPtB, const cv::Point2d & ImgPtM, cv::Point2d * intersection);
        cv::Point2d get_BorderIntersect(const cv::Point2f& Epipole, const cv::Vec3f& ImgLn, const cv::Size& ImgSize, const cv::Point2f* ImgPt_Last);

    //---Is Function---
    public:
        bool is_InsideImg(cv::Point2f ImgPt, cv::Size ImgSize);

    //---Set Functions---
    public:
        void set_Cam(glm::mat3& K);
        void set_Rot(glm::mat3 R);
        void set_Trans(glm::vec3 T);
        void set_P(glm::mat3x4 P);
    
    //---Show Functions---
    public:
        void show_Img(); 

    //---Type Transform---
    private:
        void glm2cv(cv::Mat* cvmat, const glm::mat3& glmmat);
        void glm2cv(cv::Mat* cvmat, const glm::vec3& glmmat);
        void glm2cv(cv::Mat* cvmat, const glm::mat3x4& glmmat);

    //---Members---
    private:
        cv::Mat Img; // Original Image in RGB or RGBA
        cv::Mat Img_Rect; // Rectified Image
        cv::Mat K_mtx;
        cv::Mat Rot_mtx; // Rotations from Mapping frame to Image frame
        cv::Mat Trans_vec; // Translations in Mapping frame
        cv::Mat P_mtx; // P-matrix = [Rot_mtx | -Rot_mtx*Trans_vec]
    };
} // Stereo

#endif
