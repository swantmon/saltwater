//
//  data_model.h
//  data
//
//  Created by Tobias Schwandt on 21/10/15.
//  Copyright © 2015 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_typedef.h"

#include "data/data_mesh.h"

#include <string>

namespace Dt
{
    class CModel
    {
    public:
        
        typedef std::vector<CMesh*> CMeshContainer;

    public:

        CModel();
        ~CModel();

        void SetModelname(const Base::Char* _pModelname);
        const Base::Char* GetModelname() const;

        unsigned int GetNumberOfMeshes() const;

        CMesh& GetMesh(unsigned int _ID);
        CMesh& GetMesh(unsigned int _ID) const;
        
    protected:
        
        std::string    m_Modelname;
        CMeshContainer m_Meshes;
    };
} // namespace Dt
