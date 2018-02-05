//
//  data_material.h
//  data
//
//  Created by Tobias Schwandt on 21/10/15.
//  Copyright © 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_lib_glm.h"
#include "base/base_typedef.h"

#include "data/data_texture_2d.h"

#include <string>

namespace Dt
{
    // TODO by tschwandt
    // - Materialname necessary; is filename not enough?

    class CMaterial
    {
    public:

        enum EDirtyFlags
        {
            DirtyCreate  = 0x01,
            DirtyData    = 0x02,
            DirtyTexture = 0x04,
            DirtyDestroy = 0x08,
        };

    public:

        CMaterial();
        ~CMaterial();

        void SetMaterialname(const std::string& _rMaterialname);
        const std::string& GetMaterialname() const;

        void SetFilename(const std::string& _rFilename);
        const std::string& GetFileName() const;

        void SetColorTexture(Dt::CTexture2D* _pColorTexture);
        Dt::CTexture2D* GetColorTexture();
        const Dt::CTexture2D* GetColorTexture() const;

        void SetNormalTexture(Dt::CTexture2D* _pNormalTexture);
        Dt::CTexture2D* GetNormalTexture();
        const Dt::CTexture2D* GetNormalTexture() const;

        void SetRoughnessTexture(Dt::CTexture2D* _pRoughnessTexture);
        Dt::CTexture2D* GetRoughnessTexture();
        const Dt::CTexture2D* GetRoughnessTexture() const;

        void SetMetalTexture(Dt::CTexture2D* _pMetalTexture);
        Dt::CTexture2D* GetMetalTexture();
        const Dt::CTexture2D* GetMetalTexture() const;

        void SetAmbientOcclusionTexture(Dt::CTexture2D* _pAmbientOcclusionTexture);
        Dt::CTexture2D* GetAmbientOcclusionTexture();
        const Dt::CTexture2D* GetAmbientOcclusionTexture() const;

        void SetBumpTexture(Dt::CTexture2D* _pBumpTexture);
        Dt::CTexture2D* GetBumpTexture();
        const Dt::CTexture2D* GetBumpTexture() const;

        void SetColor(const glm::vec3& _rColor);
        const glm::vec3& GetColor() const;

        void SetTilingOffset(const glm::vec4& _rTilingOffset);
        const glm::vec4& GetTilingOffset() const;

        void SetRoughness(float _Roughness);
        float GetRoughness() const;

        void SetSmoothness(float _Smoothness);
        float GetSmoothness() const;

        void SetReflectance(float _Reflectance);
        float GetReflectance() const;

        void SetMetalness(float _Metalness);
        float GetMetalness() const;

        void SetDisplacement(float _Displacement);
        float GetDisplacement() const;

        unsigned int GetHash() const;

        unsigned int GetDirtyFlags() const;

        Base::U64 GetDirtyTime() const;

    protected:
        
        std::string     m_Materialname;
        std::string     m_FileName;
        Dt::CTexture2D* m_pColorTexture;
        Dt::CTexture2D* m_pNormalTexture;
        Dt::CTexture2D* m_pRoughnessTexture;
        Dt::CTexture2D* m_pMetalTexture;
        Dt::CTexture2D* m_pAOTexture;
        Dt::CTexture2D* m_pBumpTexture;
        glm::vec3    m_Color;
        glm::vec4    m_TilingOffset;
        float           m_Roughness;
        float           m_Reflectance;
        float           m_MetalMask;
        float           m_Displacement;

        unsigned int    m_Hash;
        unsigned int    m_DirtyFlags;
        Base::U64       m_DirtyTime;
    };
} // namespace Dt
