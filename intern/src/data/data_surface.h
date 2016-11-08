//
//  data_surface.h
//  data
//
//  Created by Tobias Schwandt on 21/10/15.
//  Copyright © 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_vector2.h"
#include "base/base_vector3.h"

#include "data/data_material.h"

namespace Dt
{
    class CSurface
    {
    public:
        
        enum EElement
        {
            Position  =  0,
            Normal    =  1,
            Tangent   =  2,
            Color0    =  4,
            Color1    =  8,
            TexCoord0 = 16,
            TexCoord1 = 32,
        };

    public: 

        typedef unsigned int BIndice;

    public:

        CSurface();
        ~CSurface();

        void SetElements(unsigned int _Elements);
        unsigned int GetElements() const;

        void SetNumberOfIndices(unsigned int _NumberOfIndices);
        unsigned int GetNumberOfIndices() const;

        void SetNumberOfVertices(unsigned int _NumberOfVertices);
        unsigned int GetNumberOfVertices() const;

        void SetIndices(BIndice* _pIndices);
        BIndice* GetIndices();
        const BIndice* GetIndices() const;

        void SetPositions(Base::Float3* _pPositions);
        Base::Float3* GetPositions();
        const Base::Float3* GetPositions() const;

        void SetNormals(Base::Float3* _pNormals);
        Base::Float3* GetNormals();
        const Base::Float3* GetNormals() const;

        void SetTangents(Base::Float3* _pTangents);
        Base::Float3* GetTangents();
        const Base::Float3* GetTangents() const;

        void SetBitangents(Base::Float3* _pBitangents);
        Base::Float3* GetBitangents();
        const Base::Float3* GetBitangents() const;

        void SetTexCoords(Base::Float2* _pTexCoords);
        Base::Float2* GetTexCoords();
        const Base::Float2* GetTexCoords() const;

        void SetMaterial(CMaterial* _pMaterial);
        CMaterial* GetMaterial();
        const CMaterial* GetMaterial() const;
        
    protected:
        
        unsigned int m_Elements;
        
        unsigned int m_NumberOfIndices;
        unsigned int m_NumberOfVertices;
        
        BIndice* m_pIndices;
        Base::Float3* m_pPositions;
        Base::Float3* m_pNormals;
        Base::Float3* m_pTangents;
        Base::Float3* m_pBitangents;
        Base::Float2* m_pTexCoords;
        
        CMaterial* m_pMaterial;
    };
} // namespace Dt
