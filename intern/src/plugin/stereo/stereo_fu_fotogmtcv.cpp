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

    cv::Mat Fu_FotoGmtCV::PolarRect(Fu_FotoGmtCV Img_Match)
    {
        // Apply Polar Rectification proposed by Pollefeys et al. (1999)
        /*
        // Derive F-mtx by known P-mtx
            // E = B * R_1to2 = R_2to1 * B
            // F = inv(K2)' * E * inv(K)
            // F = [P2*PC1]_skewSym * P2 * inv(P1) = [epipole2]_skewSym * P2 * inv(P1)
        
        glm::mat3x4 P_ImgBase = glm::mat3x4(Rot_mtx, glm::vec3(Trans_vec));
        glm::mat3x4 P_ImgM = glm::mat3x4(Img_Match.Rot_mtx, glm::vec3(Img_Match.Trans_vec));
        glm::mat4x3 P_ImgBase_PsudoInv = glm::transpose(P_ImgBase) * glm::inverse(P_ImgBase * glm::transpose(P_ImgBase));
        
        glm::vec3 Epipole_ImgM = P_ImgM * Trans_vec; // Epipole of Image_Match
        glm::mat3 Epipole_ImgM_SkewSym = 
            glm::mat3(glm::vec3(0, Epipole_ImgM[2], -Epipole_ImgM[1]), glm::vec3(-Epipole_ImgM[2], 0, Epipole_ImgM[0]), glm::vec3(Epipole_ImgM[1], -Epipole_ImgM[0], 0));
        glm::mat3 F_mtx = Epipole_ImgM_SkewSym * P_ImgM * P_ImgBase_PsudoInv;
        // Derive H-mtx by known P-mtx

        DetermCoRegion();
        */
        return Img_Rect;
    }

    void Fu_FotoGmtCV::DetermCoRegion()
    {
        
    }

    void Fu_FotoGmtCV::set_Rot(glm::mat3 R)
    {
        Rot_mtx = R;
    }

    void Fu_FotoGmtCV::set_Trans(glm::vec3 T)
    {
        Trans_vec = T;
    }

    void Fu_FotoGmtCV::set_P(glm::mat4x3 P)
    {
        P_mtx = P;
    }

    void Fu_FotoGmtCV::ShowImg()
    {
        cv::imshow("Image", Img);
    }

} // Stereo
