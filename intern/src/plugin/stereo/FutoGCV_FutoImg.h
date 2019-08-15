#pragma once

#include "engine/graphic/gfx_texture_manager.h"

#include "base/base_include_glm.h" // Some warnings appears when directly #include "glm" in Engine


namespace FutoGCV
{
    struct SFutoImg
    {
        //---Member---
        Gfx::CTexturePtr m_Img_TexturePtr; // 2D Texture

        glm::ivec3 m_ImgSize; // Width, Height, Channel

        glm::mat3 m_Camera; // Interior Orientations of Camera.
        glm::mat3 m_Rotation; // Rotation from World to Camera.
        glm::vec3 m_Position; // Position of Camera in World.
        glm::mat4x3 m_PPM; // Perspective Projection Matrix

        //---Constructors & Destructor---
        SFutoImg()
        {
        }

        ~SFutoImg()
        {
            m_Img_TexturePtr = nullptr;
        }

        void SetOrientation(const glm::ivec3& _ImgSize, const glm::mat3& _Camera, const glm::mat3& _Rotation, const glm::vec3& _Position)
        {
            m_ImgSize = _ImgSize;
            m_Camera = _Camera;
            m_Rotation = _Rotation;
            m_Position = _Position;

            m_PPM = glm::mat4x3(m_Rotation[0], m_Rotation[1], m_Rotation[2], -m_Rotation * m_Position);
            m_PPM = m_Camera * m_PPM;
        }
    };
} // namespace FutoGCV