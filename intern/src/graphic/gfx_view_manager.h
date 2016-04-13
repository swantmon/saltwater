//
//  gfx_view_manager.h
//  graphic
//
//  Created by Tobias Schwandt on 23/10/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "graphic/gfx_camera.h"
#include "graphic/gfx_view.h"
#include "graphic/gfx_view_port.h"
#include "graphic/gfx_view_port_set.h"

namespace Gfx
{
    struct SViewPortDescriptor
    {
        float m_Width;
        float m_Height;
        float m_MinDepth;
        float m_MaxDepth;
        float m_TopLeftX;
        float m_TopLeftY;
    };
} // namespace Gfx

namespace Gfx
{
namespace ViewManager
{
    void OnStart();
    void OnExit();
    
    CViewPtr CreateView();
    CCameraPtr CreateCamera(CViewPtr _ViewPtr);
    
    CViewPortPtr CreateViewPort(const SViewPortDescriptor& _rDescriptor);
    
    CViewPortSetPtr CreateViewPortSet(CViewPortPtr _ViewPort1Ptr);
    CViewPortSetPtr CreateViewPortSet(CViewPortPtr _ViewPort1Ptr, CViewPortPtr _ViewPort2Ptr);
    CViewPortSetPtr CreateViewPortSet(CViewPortPtr _ViewPort1Ptr, CViewPortPtr _ViewPort2Ptr, CViewPortPtr _ViewPort3Ptr);
    CViewPortSetPtr CreateViewPortSet(CViewPortPtr _ViewPort1Ptr, CViewPortPtr _ViewPort2Ptr, CViewPortPtr _ViewPort3Ptr, CViewPortPtr _ViewPort4Ptr);
    CViewPortSetPtr CreateViewPortSet(CViewPortPtr* _pViewPortPtrs, unsigned int _NumberOfViewPorts);
    
    CCameraPtr GetMainCamera();
    CCameraPtr GetDebugCamera();
    CCameraPtr GetDecalCamera();
    CCameraPtr GetGUICamera();
    CCameraPtr GetFullQuadCamera();
    
    CViewPortPtr GetViewPort();
    CViewPortSetPtr GetViewPortSet();
} // ViewManager
} // namespace Gfx
