
#pragma once

#include "engine/engine_config.h"

#include "engine/graphic/gfx_camera.h"
#include "engine/graphic/gfx_view.h"
#include "engine/graphic/gfx_view_port.h"
#include "engine/graphic/gfx_view_port_set.h"

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
    
    ENGINE_API CCameraPtr GetMainCamera();
    CCameraPtr GetDebugCamera();
    CCameraPtr GetDecalCamera();
    CCameraPtr GetGUICamera();
    CCameraPtr GetFullQuadCamera();
    
    ENGINE_API CViewPortPtr GetViewPort();
    ENGINE_API CViewPortSetPtr GetViewPortSet();
} // ViewManager
} // namespace Gfx
