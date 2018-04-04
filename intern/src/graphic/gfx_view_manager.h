
#pragma once

#include "graphic/gfx_camera.h"
#include "graphic/gfx_export.h"
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
    GFX_API void OnStart();
    GFX_API void OnExit();
    
    GFX_API CViewPtr CreateView();
    GFX_API CCameraPtr CreateCamera(CViewPtr _ViewPtr);
    
    GFX_API CViewPortPtr CreateViewPort(const SViewPortDescriptor& _rDescriptor);
    
    GFX_API CViewPortSetPtr CreateViewPortSet(CViewPortPtr _ViewPort1Ptr);
    GFX_API CViewPortSetPtr CreateViewPortSet(CViewPortPtr _ViewPort1Ptr, CViewPortPtr _ViewPort2Ptr);
    GFX_API CViewPortSetPtr CreateViewPortSet(CViewPortPtr _ViewPort1Ptr, CViewPortPtr _ViewPort2Ptr, CViewPortPtr _ViewPort3Ptr);
    GFX_API CViewPortSetPtr CreateViewPortSet(CViewPortPtr _ViewPort1Ptr, CViewPortPtr _ViewPort2Ptr, CViewPortPtr _ViewPort3Ptr, CViewPortPtr _ViewPort4Ptr);
    GFX_API CViewPortSetPtr CreateViewPortSet(CViewPortPtr* _pViewPortPtrs, unsigned int _NumberOfViewPorts);
    
    GFX_API CCameraPtr GetMainCamera();
    GFX_API CCameraPtr GetDebugCamera();
    GFX_API CCameraPtr GetDecalCamera();
    GFX_API CCameraPtr GetGUICamera();
    GFX_API CCameraPtr GetFullQuadCamera();
    
    GFX_API CViewPortPtr GetViewPort();
    GFX_API CViewPortSetPtr GetViewPortSet();
} // ViewManager
} // namespace Gfx
