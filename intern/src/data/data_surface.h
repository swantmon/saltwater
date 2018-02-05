//
//  data_surface.h
//  data
//
//  Created by Tobias Schwandt on 21/10/15.
//  Copyright © 2015 TU Ilmenau. All rights reserved.
//

#pragma once

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

        void SetPositions(glm::vec3* _pPositions);
        glm::vec3* GetPositions();
        const glm::vec3* GetPositions() const;

        void SetNormals(glm::vec3* _pNormals);
        glm::vec3* GetNormals();
        const glm::vec3* GetNormals() const;

        void SetTangents(glm::vec3* _pTangents);
        glm::vec3* GetTangents();
        const glm::vec3* GetTangents() const;

        void SetBitangents(glm::vec3* _pBitangents);
        glm::vec3* GetBitangents();
        const glm::vec3* GetBitangents() const;

        void SetTexCoords(glm::vec2* _pTexCoords);
        glm::vec2* GetTexCoords();
        const glm::vec2* GetTexCoords() const;

        void SetMaterial(CMaterial* _pMaterial);
        CMaterial* GetMaterial();
        const CMaterial* GetMaterial() const;
        
    protected:
        
        unsigned int m_Elements;
        
        unsigned int m_NumberOfIndices;
        unsigned int m_NumberOfVertices;
        
        BIndice* m_pIndices;
        glm::vec3* m_pPositions;
        glm::vec3* m_pNormals;
        glm::vec3* m_pTangents;
        glm::vec3* m_pBitangents;
        glm::vec2* m_pTexCoords;
        
        CMaterial* m_pMaterial;
    };
} // namespace Dt
