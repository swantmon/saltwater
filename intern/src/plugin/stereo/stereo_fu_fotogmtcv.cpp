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
        glm::vec3 Epipole_ImgM = Img_Match.P_mtx * glm::vec4(Trans_vec, 1); // Epipole of Image_Match
        glm::mat3 Epipole_ImgM_SkewSym = 
            glm::mat3(glm::vec3(0, Epipole_ImgM[2], -Epipole_ImgM[1]), glm::vec3(-Epipole_ImgM[2], 0, Epipole_ImgM[0]), glm::vec3(Epipole_ImgM[1], -Epipole_ImgM[0], 0));
        
        glm::mat3 F_mtx = Epipole_ImgM_SkewSym * Img_Match.P_mtx * P_ImgB_PsudoInv;
        

        
        return Img_Rect;
    }

    void Fu_FotoGmtCV::determ_CoRegion()
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
