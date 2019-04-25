#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\FutoGmtCV_Img.h"


namespace FutoGmtCV
{
    //---Constructors & Destructor---
    FutoImg::FutoImg()
    {
    }

    FutoImg::FutoImg(std::vector<char>& Img_Input, glm::ivec2& ImgSize)
        : m_Img(Img_Input),
          m_ImgSize(ImgSize)
    {
    }

    FutoImg::FutoImg(std::vector<char>& Img_Input, glm::ivec2& ImgSize, glm::mat4x3& P)
        : m_Img(Img_Input),
          m_ImgSize(ImgSize),
          m_P_mtx(P)
    {
        // Derive K, R, T from P

    }
    FutoImg::FutoImg(std::vector<char>& Img_Input, glm::ivec2& ImgSize, glm::mat3& K, glm::mat3& R, glm::vec3& PC)
        : m_Img(Img_Input),
          m_ImgSize(ImgSize),
          m_Cam_mtx(K),
          m_Rot_mtx(R), 
          m_PC_vec(PC)
    {
        glm::mat4x3 Transform_mtx = glm::mat4x3(m_Rot_mtx[0], m_Rot_mtx[1], m_Rot_mtx[2], -m_Rot_mtx * m_PC_vec);

        m_P_mtx = m_Cam_mtx * Transform_mtx;
    }

    FutoImg::~FutoImg()
    {
    }

    //---Set Functions---
    void FutoImg::set_Cam(glm::mat3& K_Input)
    {
        m_Cam_mtx = K_Input;
    }

    void FutoImg::set_Rot(glm::mat3& R_Input)
    {
        m_Rot_mtx = R_Input;
    }

    void FutoImg::set_PC(glm::vec3& PC_Input)
    {
        m_PC_vec = PC_Input;
    }


    void FutoImg::set_PPM(glm::mat4x3& P_Input)
    {
        m_P_mtx = P_Input; // P_mtx = K_mtx * [Rot_mtx | Trans_vec] = K_mtx * [Rot_mtx | -Rot_mtx * PC_vec]
    }


    //---Get Function---
    std::vector<char> FutoImg::get_Img() const
    {
        return m_Img;
    }

    glm::ivec2 FutoImg::get_ImgSize() const
    {
        return m_ImgSize;
    }

    glm::mat3 FutoImg::get_Cam() const
    {
        return m_Cam_mtx;
    }

    glm::mat3 FutoImg::get_Rot() const
    {
        return m_Rot_mtx;
    }

    glm::vec3 FutoImg::get_PC() const
    {
        return m_PC_vec;
    }

    glm::mat4x3 FutoImg::get_PPM() const
    {
        return m_P_mtx;
    }


    //===== OLD =====

    //---Photogrammetric Computer Vision---
    void FutoImg::imp_cvSGBM(cv::Mat& DispImg, const cv::Mat& RectImg_Base, const cv::Mat& RectImg_Match)
    {
        operPtr_cvSGBM = cv::StereoSGBM::create();
        int WinSize_SAD = 9;
        operPtr_cvSGBM->setBlockSize(WinSize_SAD);
        operPtr_cvSGBM->setP1(8 * WinSize_SAD * WinSize_SAD);
        operPtr_cvSGBM->setP2(32 * WinSize_SAD * WinSize_SAD);
        operPtr_cvSGBM->setPreFilterCap(8);
        operPtr_cvSGBM->setDisp12MaxDiff(1);
        operPtr_cvSGBM->setUniquenessRatio(10);
        operPtr_cvSGBM->setSpeckleWindowSize(100);
        operPtr_cvSGBM->setSpeckleRange(1);
        //operPtr_cvSGBM->setMode(cv::StereoSGBM::MODE_SGBM);

        operPtr_cvSGBM->compute(RectImg_Base, RectImg_Match, DispImg);
    }

} // FutoImg
