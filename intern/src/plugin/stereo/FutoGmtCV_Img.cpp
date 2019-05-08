#include "plugin\stereo\stereo_precompiled.h"

#include "plugin\stereo\FutoGmtCV_Img.h"


namespace FutoGmtCV
{
    //---Constructors & Destructor---
    CFutoImg::CFutoImg()
    {
    }

    CFutoImg::CFutoImg(const std::vector<char>& Img_Input, const glm::ivec2& ImgSize, const int& Channel)
        : m_Image(Img_Input),
          m_ImgSize(ImgSize),
          m_Channel(Channel)
    {
    }

    CFutoImg::CFutoImg(const std::vector<char>& Img_Input, const glm::ivec2& ImgSize, const int& Channel, const glm::mat4x3& P)
        : m_Image(Img_Input),
          m_ImgSize(ImgSize),
          m_Channel(Channel),
          m_P_mtx(P)
    {
        // Derive K, R, T from P

    }
    CFutoImg::CFutoImg(const std::vector<char>& Img_Input, const glm::ivec2& ImgSize, const int& Channel, const glm::mat3& K, const glm::mat3& R, const glm::vec3& PC)
        : m_Image(Img_Input),
          m_ImgSize(ImgSize),
          m_Channel(Channel),
          m_Cam_mtx(K),
          m_Rot_mtx(R), 
          m_PC_vec(PC)
    {
        glm::mat4x3 Transform_mtx = glm::mat4x3(m_Rot_mtx[0], m_Rot_mtx[1], m_Rot_mtx[2], -m_Rot_mtx * m_PC_vec);

        m_P_mtx = m_Cam_mtx * Transform_mtx;
    }

    CFutoImg::~CFutoImg()
    {
    }

    //---Set Functions---
    void CFutoImg::set_Cam(glm::mat3& K_Input)
    {
        m_Cam_mtx = K_Input;
    }

    void CFutoImg::set_Rot(glm::mat3& R_Input)
    {
        m_Rot_mtx = R_Input;
    }

    void CFutoImg::set_PC(glm::vec3& PC_Input)
    {
        m_PC_vec = PC_Input;
    }


    void CFutoImg::set_PPM(glm::mat4x3& P_Input)
    {
        m_P_mtx = P_Input; // P_mtx = K_mtx * [Rot_mtx | Trans_vec] = K_mtx * [Rot_mtx | -Rot_mtx * PC_vec]
    }


    //---Get Function---
    const std::vector<char>& CFutoImg::get_Img() const
    {
        return m_Image;
    }

    const glm::ivec2& CFutoImg::get_ImgSize() const
    {
        return m_ImgSize;
    }

    const glm::mat3& CFutoImg::get_Cam() const
    {
        return m_Cam_mtx;
    }

    const glm::mat3& CFutoImg::get_Rot() const
    {
        return m_Rot_mtx;
    }

    const glm::vec3& CFutoImg::get_PC() const
    {
        return m_PC_vec;
    }

    const glm::mat4x3& CFutoImg::get_PPM() const
    {
        return m_P_mtx;
    }

} // FutoGmtCV
