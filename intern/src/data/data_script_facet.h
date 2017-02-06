//
//  data_script_facet.h
//  data
//
//  Created by Tobias Schwandt on 19/05/16.
//  Copyright (c) 2016 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_typedef.h"
#include "base/base_vector2.h"
#include "base/base_vector3.h"

#include <string>

namespace Dt
{
    class CScriptFacet
    {

    public:

        void SetScriptFile(const Base::Char* _pScriptFile);
        const Base::Char* GetScriptFile();

    public:

        CScriptFacet();
        ~CScriptFacet();

    private:
        
        std::string m_ScriptFile;
    };
} // namespace Dt