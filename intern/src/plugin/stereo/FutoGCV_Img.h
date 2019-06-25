
#pragma once

#include "C:/saltwater/intern/src/base/base_include_glm.h" // Some warnings appears when directly #include "glm" in Engine

#include <vector>

namespace FutoGCV
{
    class CFutoImg
    {
    //---Constructor & Destructor---
    public:
        CFutoImg();
        CFutoImg(const std::vector<char>& Img_Input, const glm::ivec2& ImgSize, const int& Channel);
        CFutoImg(const std::vector<char>& Img_Input, const glm::ivec2& ImgSize, const int& Channel, const glm::mat4x3& P);
        CFutoImg(const std::vector<char>& Img_Input, const glm::ivec2& ImgSize, const int& Channel, const glm::mat3& K, const glm::mat3& R, const glm::vec3& PC);
        ~CFutoImg();

    //---Set Functions---
    public:
        void set_Cam(glm::mat3& K);
        void set_Rot(glm::mat3& R);
        void set_PC(glm::vec3& T);
        void set_PPM(glm::mat4x3& P_Input);

    //---Get Function---
    public:
        const std::vector<char>& get_Img() const;
        const glm::ivec2& get_ImgSize() const;
        const glm::mat3& get_Cam() const;
        const glm::mat3& get_Rot() const;
        const glm::vec3& get_PC() const;
        const glm::mat4x3& get_PPM() const;
        
    //---Members---
    private:
        std::vector<char> m_Image; // Image data
        glm::ivec2 m_ImgSize; // Image Width & Image Height
        int m_Channel; // Number of Channel of Image
        glm::mat3 m_Cam_mtx; // Camera Matrix in pixel. Origin is upper-left. x-axis is row-direction and y-axis is column-direction
        glm::mat3 m_Rot_mtx; // Rotations from Mapping frame to Image frame
        glm::vec3 m_PC_vec; // Projection Center in Mapping frame
        glm::mat4x3 m_P_mtx; // Perspective Projection Matrix = K_mtx * [Rot_mtx | -Rot_mtx*PC_vec]
    };
} // FutoGmtCV

