//
//  data_model.h
//  data
//
//  Created by Tobias Schwandt on 21/10/15.
//  Copyright © 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_aabb3.h"
#include "base/base_string.h"
#include "base/base_typedef.h"

#include "data/data_lod.h"

namespace Dt
{
    class CModel
    {
    public:
        
        static const unsigned int s_NumberOfLODs = 4;

    public:

        CModel();
        ~CModel();

        void SetModelname(const char* _pModelname);
        const char* GetModelname() const;

        void SetNumberOfLODs(unsigned int _NumberOfLODs);
        unsigned int GetNumberOfLODs() const;

        void SetLOD(unsigned int _Index, CLOD* _pLOD);
        CLOD* GetLOD(unsigned int _Index);
        const CLOD* GetLOD(unsigned int _Index) const;
        
    protected:
        
        Base::CharString m_Modelname;
        unsigned int     m_NumberOfLODs;
        CLOD*            m_LODs[s_NumberOfLODs];
    };
} // namespace Dt
