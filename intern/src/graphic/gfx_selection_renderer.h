//
//  gfx_selection_renderer.h
//  graphic
//
//  Created by Tobias Schwandt on 17/11/16.
//  Copyright (c) 2016 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_vector2.h"
#include "base/base_vector3.h"

namespace Dt
{
    class CEntity;
    class CRegion;
} // namespace Dt

namespace Gfx
{
    struct SPickingInfo
    {
        enum EFlag
        {
            Nothing = 0x00,
            Entity  = 0x01,
            Terrain = 0x02,
        };

        unsigned int m_Flags;
        Base::Float3 m_WSPosition;
        Base::Float3 m_WSNormal;
        float        m_Depth;

        union 
        {
            Dt::CEntity* m_pEntity;
            Dt::CRegion* m_pRegion;
        };
    };
} // namespace Gfx

namespace Gfx
{
namespace SelectionRenderer
{
    void OnStart();
    void OnExit();
    
    void OnSetupShader();
    void OnSetupKernels();
    void OnSetupRenderTargets();
    void OnSetupStates();
    void OnSetupTextures();
    void OnSetupBuffers();
    void OnSetupResources();
    void OnSetupModels();
    void OnSetupEnd();
    
    void OnReload();
    
    void Update();
    void Render();

    void SelectEntity(unsigned int _EntityID);
    void UnselectEntity();

    SPickingInfo Pick(const Base::Float2& _rUV, bool _IsHomogeneous = true);
} // namespace SelectionRenderer
} // namespace Gfx
