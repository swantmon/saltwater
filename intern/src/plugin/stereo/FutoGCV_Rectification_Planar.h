
#pragma once

#include "engine/graphic/gfx_shader_manager.h" // To manage GPU Shader
#include "engine/graphic/gfx_texture_manager.h" // To manage GPU Texture (Data in GPU processing) 
#include "engine/graphic/gfx_buffer_manager.h" // To manage GPU Buffer (Memory in GPU) 

#include "plugin\stereo\futogcv_futoimg.h"

namespace FutoGCV
{
    struct SHomography // Always make sure whole structure is the multiple of 4*float
    {
        glm::mat4 m_H_Orig2Epi; // GPU memory transmission is based on 4. => Using mat3 will adress wrong memory.
        glm::mat4 m_H_Epi2Orig; // GPU memory transmission is based on 4. => Using mat3 will adress wrong memory.
        glm::ivec2 m_EpiCorner_UL; // pix_Epi = pix_Orig2Epi + CornerUL; <= pix_Orig2Epi = H * pix_Orig;
        glm::ivec2 m_EpiCorner_DR; // Makes Structure base on 4.
    };

    class CPlanarRectification
    {

    //===== Constructors & Destructor =====

    public:

        CPlanarRectification(const glm::ivec2& _OrigImgSize);

        ~CPlanarRectification();

    //===== Execution Functions =====

    public:

        void execute(SFutoImg& EpiImg_B, Gfx::CBufferPtr HomoB_BufferPtr, 
                     SFutoImg& EpiImg_M, Gfx::CBufferPtr HomoM_BufferPtr, 
                     const SFutoImg& _OrigImg_B, const SFutoImg& _OrigImg_M);

    //===== Assistant Functions =====

    private:

        //---Compute Homography based on Epipolar Geometry---
        void ComputeEpiCamera(const glm::mat3& _OrigCamera_B, const glm::mat3& _OrigCamera_M);
        void ComputeEpiRotation(const glm::vec3& _OrigPosition_B, const glm::vec3& _OrigPosition_M, const glm::mat3& _OrigRotation_B);
        void ComputeEpiPosition(const glm::vec3& _OrigPosition_B, const glm::vec3& _OrigPosition_M);
        void ComputeEpiPPM();

        void ComputeHomography(const glm::mat4x3& _OrigPPM_B, const glm::mat4x3& _OrigPPM_M);

        //---Center Epipolar Images---
        void ComputeCenterShift(glm::vec2& CenterDrift, const int Which_Img = 0);
        void ShiftEpiCenter(const glm::vec2& Drift_B, const glm::vec2& Drift_M);

        //---Determine Epipolar Image Size---
        void ComputeEpiCorner(const int Which_Img = 0);
        void DetermEpiImgSize(Gfx::CTexturePtr EpiImg_B_TexturePtr, Gfx::CTexturePtr EpiImg_M_TexturePtr);

        //---Generate Epipolar Images---
        void HomoTransform(Gfx::CTexturePtr EpiImg_TexturePtr, Gfx::CBufferPtr Homo_BufferPtr, Gfx::CTexturePtr OrigImg_TexturePtr, const int Which_Img = 0);

    // ===== Members =====

    private:

        glm::ivec2 m_OrigImgSize, m_EpiImgSize;

        glm::mat3 m_EpiCamera_B, m_EpiCamera_M; // Camera matrix of Epipolar Images
        glm::mat3 m_EpiRotation; // Rotation matrix of Epipolar Images (World -> Image)
        glm::vec3 m_EpiPosition_B, m_EpiPosition_M; // Projection Center of Epipolar Images
        glm::mat4x3 m_EpiPPM_B, m_EpiPPM_M; // Perspective Projection Matrix of Epipolar Images (World -> Image)

        SHomography m_Homography_B, m_Homography_M;

        Gfx::CShaderPtr m_PlanarRectificationCSPtr;

        Gfx::CTexturePtr m_EpiImgB_TexturePtr, m_EpiImgM_TexturePtr;
    };

} // namespace FutoGmtCV
