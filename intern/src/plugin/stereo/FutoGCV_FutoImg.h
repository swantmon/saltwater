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

        SFutoImg(Gfx::CTexturePtr _Img_TexturePtr, const glm::ivec3& _ImgSize, const glm::mat3& _Camera, const glm::mat3& _Rotation, const glm::vec3& _Position)
            : m_ImgSize(_ImgSize),
              m_Camera(_Camera), 
              m_Rotation(_Rotation), 
              m_Position(_Position)
        {
            {
                Gfx::STextureDescriptor TextDesc_Img = {};
                TextDesc_Img.m_NumberOfPixelsU = _Img_TexturePtr->GetNumberOfPixelsU();
                TextDesc_Img.m_NumberOfPixelsV = _Img_TexturePtr->GetNumberOfPixelsV();
                TextDesc_Img.m_NumberOfPixelsW = _Img_TexturePtr->GetNumberOfPixelsW();
                TextDesc_Img.m_NumberOfMipMaps = _Img_TexturePtr->GetNumberOfMipLevels();
                TextDesc_Img.m_NumberOfTextures = _Img_TexturePtr->GetNumberOfTextures();
                TextDesc_Img.m_Binding = _Img_TexturePtr->GetBinding();
                TextDesc_Img.m_Access = _Img_TexturePtr->GetAccess();
                TextDesc_Img.m_Usage = _Img_TexturePtr->GetUsage();
                TextDesc_Img.m_Semantic = _Img_TexturePtr->GetSemantic();
                TextDesc_Img.m_Format = _Img_TexturePtr->GetFormat();

                m_Img_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextDesc_Img);
            }
            Gfx::TextureManager::CopyTexture(_Img_TexturePtr, m_Img_TexturePtr);

            m_PPM = glm::mat4x3(m_Rotation[0], m_Rotation[1], m_Rotation[2], -m_Rotation * m_Position);
            m_PPM = m_Camera * m_PPM;
        }

        SFutoImg(const SFutoImg& _FutoImg) // Copy Constructor: Input must be const and called by reference.
            : m_ImgSize(_FutoImg.m_ImgSize),
              m_Camera(_FutoImg.m_Camera),
              m_Rotation(_FutoImg.m_Rotation),
              m_Position(_FutoImg.m_Position),
              m_PPM(_FutoImg.m_PPM)
        {
            {
                Gfx::STextureDescriptor TextDesc_Img = {};
                TextDesc_Img.m_NumberOfPixelsU = _FutoImg.m_Img_TexturePtr->GetNumberOfPixelsU();
                TextDesc_Img.m_NumberOfPixelsV = _FutoImg.m_Img_TexturePtr->GetNumberOfPixelsV();
                TextDesc_Img.m_NumberOfPixelsW = _FutoImg.m_Img_TexturePtr->GetNumberOfPixelsW();
                TextDesc_Img.m_NumberOfMipMaps = _FutoImg.m_Img_TexturePtr->GetNumberOfMipLevels();
                TextDesc_Img.m_NumberOfTextures = _FutoImg.m_Img_TexturePtr->GetNumberOfTextures();
                TextDesc_Img.m_Binding = _FutoImg.m_Img_TexturePtr->GetBinding();
                TextDesc_Img.m_Access = _FutoImg.m_Img_TexturePtr->GetAccess();
                TextDesc_Img.m_Usage = _FutoImg.m_Img_TexturePtr->GetUsage();
                TextDesc_Img.m_Semantic = _FutoImg.m_Img_TexturePtr->GetSemantic();
                TextDesc_Img.m_Format = _FutoImg.m_Img_TexturePtr->GetFormat();

                m_Img_TexturePtr = Gfx::TextureManager::CreateTexture2D(TextDesc_Img);
            }
            Gfx::TextureManager::CopyTexture(_FutoImg.m_Img_TexturePtr, m_Img_TexturePtr);
        }

        ~SFutoImg()
        {
            m_Img_TexturePtr = nullptr;
        }

    };
} // namespace FutoGCV