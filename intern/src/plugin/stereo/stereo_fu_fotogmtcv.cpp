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
        std::vector<cv::Point2f> EpiPoles(2);
        EpiPoles[0] = cv::Point2d(EpiPole_ImgB.at<float>(0, 0) / EpiPole_ImgB.at<float>(0, 2), EpiPole_ImgB.at<float>(0, 1) / EpiPole_ImgB.at<float>(0, 2));
        cv::Mat e2 = F_svd.u.col(2);
        cv::Point2f EpiPole_M = cv::Point2d(e2.at<float>(0, 0) / e2.at<float>(2, 0), e2.at<float>(1, 0) / e2.at<float>(2, 0));
        EpiPoles[1] = cv::Point2d(EpiPole_ImgM.at<float>(0, 0) / EpiPole_ImgM.at<float>(2, 0), EpiPole_ImgM.at<float>(1, 0) / EpiPole_ImgM.at<float>(2, 0));
        if ((EpiPoles[0].x * EpiPoles[1].x < 0) && (EpiPoles[0].y * EpiPoles[1].y < 0))
        {

            EpiPoles[1] *= -1;
        }

        determ_CoRegion(EpiPoles, Img.size(), F_mtx);

        return Img_Rect;
    }

    void Fu_FotoGmtCV::determ_CoRegion(const std::vector<cv::Point2f>& EpiPoles, const cv::Size ImgSize, const cv::Mat& F)
    {
        std::vector<cv::Point2f> ImgCorner_B, ImgCorner_M;
        get_ImgCorner(EpiPoles[0], ImgSize, ImgCorner_B);
        get_ImgCorner(EpiPoles[1], ImgSize, ImgCorner_M);

        float Rho_B_min, Rho_B_max, Rho_M_min, Rho_M_max;
        determ_RhoRange(EpiPoles[0], ImgSize, ImgCorner_B, Rho_B_min, Rho_B_max);
        determ_RhoRange(EpiPoles[1], ImgSize, ImgCorner_M, Rho_M_min, Rho_M_max);


    }

    void Fu_FotoGmtCV::get_ImgCorner(const cv::Point2f& EpiPole, const cv::Size ImgSize, std::vector<cv::Point2f>& ImgCorner)
    {
        if (EpiPole.y < 0)
        {
            if (EpiPole.x < 0)
            {
                ImgCorner.resize(2);
                ImgCorner[0] = cv::Point2f(ImgSize.width - 1, 0);
                ImgCorner[1] = cv::Point2f(0, ImgSize.height - 1);
            }
            else if (EpiPole.x < ImgSize.width - 1)
            {
                ImgCorner.resize(2);
                ImgCorner[0] = cv::Point2f(ImgSize.width - 1, 0);
                ImgCorner[1] = cv::Point2f(0, 0);
            }
            else
            {
                ImgCorner.resize(2);
                ImgCorner[0] = cv::Point2f(ImgSize.width - 1, ImgSize.height - 1);
                ImgCorner[1] = cv::Point2f(0, 0);
            }
        }
        else if (EpiPole.y <= ImgSize.height - 1)
        {
            if (EpiPole.x < 0)
            {
                ImgCorner.resize(2);
                ImgCorner[0] = cv::Point2f(0, 0);
                ImgCorner[1] = cv::Point2f(0, ImgSize.height - 1);
            }
            else if (EpiPole.x < ImgSize.width - 1)
            {
                ImgCorner.resize(4);
                ImgCorner[0] = cv::Point2f(0, 0);
                ImgCorner[1] = cv::Point2f(ImgSize.width - 1, 0);
                ImgCorner[2] = cv::Point2f(ImgSize.width - 1, ImgSize.height - 1);
                ImgCorner[3] = cv::Point2f(0, ImgSize.height - 1);
            }
            else
            {
                ImgCorner.resize(2);
                ImgCorner[0] = cv::Point2f(ImgSize.width - 1, ImgSize.height - 1);
                ImgCorner[1] = cv::Point2f(ImgSize.width - 1, 0);
            }
        }
        else
        {
            if (EpiPole.x < 0)
            {
                ImgCorner.resize(2);
                ImgCorner[0] = cv::Point2f(0, 0);
                ImgCorner[1] = cv::Point2f(ImgSize.width - 1, ImgSize.height - 1);
            }
            else if (EpiPole.x < ImgSize.width - 1)
            {
                ImgCorner.resize(2);
                ImgCorner[0] = cv::Point2f(0, ImgSize.height - 1);
                ImgCorner[1] = cv::Point2f(ImgSize.width - 1, ImgSize.height - 1);
            }
            else
            {
                ImgCorner.resize(2);
                ImgCorner[0] = cv::Point2f(0, ImgSize.height - 1);
                ImgCorner[1] = cv::Point2f(ImgSize.width - 1, 0);
            }
        }
    }

    void Fu_FotoGmtCV::determ_RhoRange(const cv::Point2f & EpiPole, const cv::Size ImgSize, std::vector<cv::Point2f> & ImgCorner, float & minRho, float & maxRho)
    {
        if (EpiPole.y < 0)
        { 
            if (EpiPole.x < 0) 
            {
                minRho = sqrt(EpiPole.x * EpiPole.x + EpiPole.y * EpiPole.y); // Point A
                maxRho = sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)); // Point D
            }
            else if (EpiPole.x <= ImgSize.width - 1) 
            {
                minRho = -EpiPole.y;
                maxRho = std::max(
                                  sqrt(EpiPole.x * EpiPole.x + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)), // Point C
                                  sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)) // Point D
                                 );
            }
            else { // Case 3
                minRho = sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + EpiPole.y * EpiPole.y); // Point B
                maxRho = sqrt(EpiPole.x * EpiPole.x + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)); // Point C
            }
        }
        else if (EpiPole.y <= ImgSize.height - 1) 
        { 
            if (EpiPole.x < 0) 
            { 
                minRho = -EpiPole.x;
                maxRho = std::max(
                                  sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)), // Point D
                                  sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + EpiPole.y * EpiPole.y) // Point B
                                 );
            }
            else if (EpiPole.x <= ImgSize.width - 1) 
            { 
                minRho = 0;
                maxRho = std::max(
                                  std::max(
                                           sqrt(EpiPole.x * EpiPole.x + EpiPole.y * EpiPole.y), // Point A
                                           sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + EpiPole.y * EpiPole.y) // Point B
                                          ),
                                  std::max(
                                           sqrt(EpiPole.x * EpiPole.x + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)), // Point C
                                           sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)) // Point D
                                          )
                                 );
            }
            else 
            { 
                minRho = EpiPole.x - (ImgSize.width - 1);
                maxRho = std::max(
                                  sqrt(EpiPole.x * EpiPole.x + EpiPole.y * EpiPole.y), // Point A
                                  sqrt(EpiPole.x * EpiPole.x + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)) // Point C
                                 );
            }
        }
        else 
        {
            if (EpiPole.x < 0) 
            {
                minRho = sqrt(EpiPole.x * EpiPole.x + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)); // Point C
                maxRho = sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + EpiPole.y * EpiPole.y); // Point B
            }
            else if (EpiPole.x <= ImgSize.width - 1) 
            { 
                minRho = EpiPole.y - (ImgSize.height - 1);
                maxRho = std::max(
                                  sqrt(EpiPole.x * EpiPole.x + EpiPole.y * EpiPole.y), // Point A
                                  sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + EpiPole.y * EpiPole.y) // Point B
                                 );
            }
            else 
            {
                minRho = sqrt(((ImgSize.width - 1) - EpiPole.x) * ((ImgSize.width - 1) - EpiPole.x) + ((ImgSize.height - 1) - EpiPole.y) * ((ImgSize.height - 1) - EpiPole.y)); // Point D
                maxRho = sqrt(EpiPole.x * EpiPole.x + EpiPole.y * EpiPole.y); // Point A
            }
        }
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
