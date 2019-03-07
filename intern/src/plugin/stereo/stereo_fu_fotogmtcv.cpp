#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\stereo_fu_fotogmtcv.h"


namespace Stereo
{
    Fu_FotoGmtCV::Fu_FotoGmtCV()
    {
    }
    
    Fu_FotoGmtCV::Fu_FotoGmtCV(const std::vector<char>& Img_Input, int ImgW, int ImgH)
    {
        // Latter apply switch for different types of images
        cv::Mat Img_Input_cv(cv::Size(ImgW, ImgH), CV_8UC4); // 2D Matrix(x*y) with (8-bit unsigned character) + (4 bands)
        memcpy(Img_Input_cv.data, Img_Input.data(), Img_Input.size());
        cv::cvtColor(Img_Input_cv, Img_Input_cv, cv::COLOR_BGRA2RGBA); // Transform image from BGRA (default of OpenCV) to RGB
        Img = Img_Input_cv; // 1D vector stores 2D image in row-oriented
    }
    Fu_FotoGmtCV::~Fu_FotoGmtCV()
    {
    }

    //---Polar Rectification---
    cv::Mat Fu_FotoGmtCV::PolarRect(Fu_FotoGmtCV Img_Match) // Apply Polar Rectification proposed by Pollefeys et al. (1999)
    {
        //---Derive F-mtx by known P-mtx---
            // E = B * R_1to2 = R_2to1 * B
            // F = inv(K2)' * E * inv(K)
            // F = [epipole2]_skewSym * P2 * inv(P1) = [P2*PC1]_skewSym * P2 * inv(P1)
        
        cv::Mat P_ImgB_PsudoInv;
        cv::invert(P_mtx, P_ImgB_PsudoInv, cv::DECOMP_SVD);
        cv::Mat PC = cv::Mat(4, 1, CV_32F);
        PC.at<float>(0, 0) = Trans_vec.at<float>(0, 0);
        PC.at<float>(1, 0) = Trans_vec.at<float>(1, 0);
        PC.at<float>(2, 0) = Trans_vec.at<float>(2, 0);
        PC.at<float>(3, 0) = 1;
        cv::Mat EpiPole_ImgM = Img_Match.P_mtx * PC; // Epipole of Image_Match
        float eMx = (EpiPole_ImgM.at<float>(0, 0) / EpiPole_ImgM.at<float>(2, 0));
        float eMy = (EpiPole_ImgM.at<float>(1, 0) / EpiPole_ImgM.at<float>(2, 0));
        cv::Mat Epipole_ImgM_SkewSym = cv::Mat::zeros(cv::Size(3, 3), CV_32F);
        Epipole_ImgM_SkewSym.at<float>(0, 1) = -EpiPole_ImgM.at<float>(2, 0);
        Epipole_ImgM_SkewSym.at<float>(0, 2) = EpiPole_ImgM.at<float>(1, 0);
        Epipole_ImgM_SkewSym.at<float>(1, 0) = EpiPole_ImgM.at<float>(2, 0);
        Epipole_ImgM_SkewSym.at<float>(1, 2) = -EpiPole_ImgM.at<float>(0, 0);
        Epipole_ImgM_SkewSym.at<float>(2, 0) = -EpiPole_ImgM.at<float>(1, 0);
        Epipole_ImgM_SkewSym.at<float>(2, 1) = EpiPole_ImgM.at<float>(0, 0);
        
        cv::Mat F_mtx = Epipole_ImgM_SkewSym * Img_Match.P_mtx * P_ImgB_PsudoInv;
        
        cv::SVD F_svd(F_mtx);
        cv::Mat EpiPole_ImgB = F_svd.vt.row(2);
        std::vector<cv::Point2f> EpiPole;
        EpiPole[0] = cv::Point2d(EpiPole_ImgB.at<float>(0, 0) / EpiPole_ImgB.at<float>(0, 2), EpiPole_ImgB.at<float>(0, 1) / EpiPole_ImgB.at<float>(0, 2));
        // cv::Mat e2 = F_svd.u.col(2);
        // cv::Point2f EpiPole_M = cv::Point2d(e2.at<float>(0, 0) / e2.at<float>(2, 0), e2.at<float>(1, 0) / e2.at<float>(2, 0));
        EpiPole[1] = cv::Point2d(EpiPole_ImgM.at<float>(0, 0) / EpiPole_ImgM.at<float>(2, 0), EpiPole_ImgM.at<float>(1, 0) / EpiPole_ImgM.at<float>(2, 0));
        if ((EpiPole[0].x * EpiPole[1].x < 0) && (EpiPole[0].y * EpiPole[1].y < 0))
        {

            EpiPole[1] *= -1;
        }
        
        return Img_Rect;
    }

    void Fu_FotoGmtCV::determ_CoRegion(const std::vector<cv::Point2f>& EpiPoles, const cv::Size ImgSize, const cv::Mat& F)
    {
        
    }

    //---Orientation & Transformation---


    //---Set Functions---
    void Fu_FotoGmtCV::set_Cam(glm::mat3& glmK)
    {
        cv::Mat cvK = cv::Mat(3, 3, CV_32F);
        glm2cv(&cvK, glm::transpose(glmK));
        
        K_mtx = cvK;
    }

    void Fu_FotoGmtCV::set_Rot(glm::mat3 glmR)
    {
        cv::Mat cvR = cv::Mat(3, 3, CV_32F);
        glm2cv(&cvR, glm::transpose(glmR));
        
        Rot_mtx = cvR;
    }

    void Fu_FotoGmtCV::set_Trans(glm::vec3 glmT)
    {
        cv::Mat cvT = cv::Mat(3, 1, CV_32F);
        glm2cv(&cvT, glmT);
        
        Trans_vec = cvT;
    }

    void Fu_FotoGmtCV::set_P(glm::mat3x4 glmP)
    {
        cv::Mat cvP = cv::Mat(3, 4, CV_32F);
        glm2cv(&cvP, glmP);

        P_mtx = cvP;
    }

    //---Show Function---
    void Fu_FotoGmtCV::show_Img()
    {
        cv::imshow("Image", Img);
    }

    //---Type Transform---
    void Fu_FotoGmtCV::glm2cv(cv::Mat* cvmat, const glm::mat3& glmmat)
    {
        memcpy(cvmat->data, glm::value_ptr(glmmat), 9 * sizeof(float));
    }

    void Fu_FotoGmtCV::glm2cv(cv::Mat* cvmat, const glm::vec3& glmmat)
    {
        memcpy(cvmat->data, glm::value_ptr(glmmat), 3 * sizeof(float));
    }

    void Fu_FotoGmtCV::glm2cv(cv::Mat* cvmat, const glm::mat3x4& glmmat)
    {
        memcpy(cvmat->data, glm::value_ptr(glmmat), 12 * sizeof(float));
    }

} // Stereo
