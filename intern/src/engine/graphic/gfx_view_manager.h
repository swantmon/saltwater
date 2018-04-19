
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
    ENGINE_API void OnStart();
    ENGINE_API void OnExit();
    
    ENGINE_API CViewPtr CreateView();
    ENGINE_API CCameraPtr CreateCamera(CViewPtr _ViewPtr);
    
    ENGINE_API CViewPortPtr CreateViewPort(const SViewPortDescriptor& _rDescriptor);
    
    ENGINE_API CViewPortSetPtr CreateViewPortSet(CViewPortPtr _ViewPort1Ptr);
    ENGINE_API CViewPortSetPtr CreateViewPortSet(CViewPortPtr _ViewPort1Ptr, CViewPortPtr _ViewPort2Ptr);
    ENGINE_API CViewPortSetPtr CreateViewPortSet(CViewPortPtr _ViewPort1Ptr, CViewPortPtr _ViewPort2Ptr, CViewPortPtr _ViewPort3Ptr);
    ENGINE_API CViewPortSetPtr CreateViewPortSet(CViewPortPtr _ViewPort1Ptr, CViewPortPtr _ViewPort2Ptr, CViewPortPtr _ViewPort3Ptr, CViewPortPtr _ViewPort4Ptr);
    ENGINE_API CViewPortSetPtr CreateViewPortSet(CViewPortPtr* _pViewPortPtrs, unsigned int _NumberOfViewPorts);
    
    ENGINE_API CCameraPtr GetMainCamera();
    ENGINE_API CCameraPtr GetDebugCamera();
    ENGINE_API CCameraPtr GetDecalCamera();
    ENGINE_API CCameraPtr GetGUICamera();
    ENGINE_API CCameraPtr GetFullQuadCamera();
    
    ENGINE_API CViewPortPtr GetViewPort();
    ENGINE_API CViewPortSetPtr GetViewPortSet();
} // ViewManager
} // namespace Gfx
