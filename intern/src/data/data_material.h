//
//  data_material.h
//  data
//
//  Created by Tobias Schwandt on 21/10/15.
//  Copyright © 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_string.h"
#include "base/base_typedef.h"
#include "base/base_vector3.h"
#include "base/base_vector4.h"

#include "data/data_texture_2d.h"

namespace Dt
{
    // TODO by tschwandt
    // remove reflectance texture

    class CMaterial
    {
    public:

        CMaterial();
        ~CMaterial();

        void SetMaterialname(const char* _pMaterialname);
        const char* GetMaterialname() const;

        void SetColorTexture(Dt::CTexture2D* _pColorTexture);
        Dt::CTexture2D* GetColorTexture();
        const Dt::CTexture2D* GetColorTexture() const;

        void SetNormalTexture(Dt::CTexture2D* _pNormalTexture);
        Dt::CTexture2D* GetNormalTexture();
        const Dt::CTexture2D* GetNormalTexture() const;

        void SetRoughnessTexture(Dt::CTexture2D* _pRoughnessTexture);
        Dt::CTexture2D* GetRoughnessTexture();
        const Dt::CTexture2D* GetRoughnessTexture() const;

        void SetReflectanceTexture(Dt::CTexture2D* _pReflectanceTexture);
        Dt::CTexture2D* GetReflectanceTexture();
        const Dt::CTexture2D* GetReflectanceTexture() const;

        void SetMetalTexture(Dt::CTexture2D* _pMetalTexture);
        Dt::CTexture2D* GetMetalTexture();
        const Dt::CTexture2D* GetMetalTexture() const;

        void SetAmbientOcclusionTexture(Dt::CTexture2D* _pAmbientOcclusionTexture);
        Dt::CTexture2D* GetAmbientOcclusionTexture();
        const Dt::CTexture2D* GetAmbientOcclusionTexture() const;

        void SetBumpTexture(Dt::CTexture2D* _pBumpTexture);
        Dt::CTexture2D* GetBumpTexture();
        const Dt::CTexture2D* GetBumpTexture() const;

        void SetColor(const Base::Float3& _rColor);
        const Base::Float3& GetColor() const;

        void SetTilingOffset(const Base::Float4& _rTilingOffset);
        const Base::Float4& GetTilingOffset() const;

        void SetRoughness(float _Roughness);
        float GetRoughness() const;

        void SetSmoothness(float _Smoothness);
        float GetSmoothness() const;

        void SetReflectance(float _Reflectance);
        float GetReflectance() const;

        void SetMetalness(float _Metalness);
        float GetMetalness() const;

    protected:
        
        Base::CharString m_Materialname;
        Dt::CTexture2D*  m_pColorTexture;
        Dt::CTexture2D*  m_pNormalTexture;
        Dt::CTexture2D*  m_pRoughnessTexture;
        Dt::CTexture2D*  m_pReflectanceMap;
        Dt::CTexture2D*  m_pMetalTexture;
        Dt::CTexture2D*  m_pAOTexture;
        Dt::CTexture2D*  m_pBumpTexture;
        Base::Float3     m_Color;
        Base::Float4     m_TilingOffset;
        float            m_Roughness;
        float            m_Reflectance;
        float            m_MetalMask;
    };
} // namespace Dt
