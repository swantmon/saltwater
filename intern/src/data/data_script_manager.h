//
//  data_script_manager.h
//  data
//
//  Created by Tobias Schwandt on 18/05/16.
//  Copyright (c) 2016 TU Ilmenau. All rights reserved.
//

#pragma once

#include "data/data_script_facet.h"

namespace Dt
{
namespace ScriptManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CScriptFacet* CreateScript();

    void Update();
} // namespace ScriptManager
} // namespace Dt
