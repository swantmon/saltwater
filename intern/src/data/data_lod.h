//
//  data_lod.h
//  data
//
//  Created by Tobias Schwandt on 21/10/15.
//  Copyright © 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "data/data_surface.h"

namespace Dt
{
    class CLOD
    {
    public:
        
        static const unsigned int s_NumberOfSurfaces = 16;

    public:

        CLOD();
        ~CLOD();

        void SetNumberOfSurfaces(unsigned int _NumberOfSurfaces);
        unsigned int GetNumberOfSurfaces() const;

        void SetSurface(unsigned int _Index, CSurface* _pSurface);
        CSurface* GetSurface(unsigned int _Index);
        const CSurface* GetSurface(unsigned int _Index) const;
        
    protected:
        
        unsigned int m_NumberOfSurfaces;
        CSurface*    m_Surfaces[s_NumberOfSurfaces];
    };
} // namespace Dt
