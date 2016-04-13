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
        
        unsigned int m_Elements;
        
        unsigned int m_NumberOfIndices;
        unsigned int m_NumberOfVertices;
        
        unsigned int* m_pIndices;
        Base::Float3* m_pPositions;
        Base::Float3* m_pNormals;
        Base::Float3* m_pTangents;
        Base::Float3* m_pBitangents;
        Base::Float2* m_pTexCoords;
        
        CMaterial* m_pDefaultMaterial;
    };
} // namespace Dt
