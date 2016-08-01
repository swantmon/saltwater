//
//  data_fx_manager.h
//  data
//
//  Created by Tobias Schwandt on 29/03/16.
//  Copyright (c) 2016 TU Ilmenau. All rights reserved.
//

#pragma once

#include "data/data_fx_facet.h"

namespace Dt
{
namespace FXManager
{
    void OnStart();
    void OnExit();

    void Clear();

    CBloomFXFacet* CreateBloomFX();
    CSSRFXFacet* CreateSSRFX();
    CDOFFXFacet* CreateDOFFX();
    CFXAAFXFacet* CreateFXAAFX();
    CSSAOFXFacet* CreateSSAOFX();
    CVolumeFogFXFacet* CreateVolumeFogFX();

    void Update();
} // namespace FXManager
} // namespace Dt
