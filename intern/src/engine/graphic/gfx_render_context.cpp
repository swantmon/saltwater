
#include "engine/engine_precompiled.h"

#include "engine/graphic/gfx_render_context.h"

namespace Gfx
{
    CRenderContext::CRenderContext()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CRenderContext::~CRenderContext()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    void CRenderContext::SetCamera(CCameraPtr _CameraPtr)
    {
        m_CameraPtr = _CameraPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CCameraPtr CRenderContext::GetCamera() const
    {
        return m_CameraPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CRenderContext::SetTargetSet(CTargetSetPtr _TargetSetPtr)
    {
        m_TargetSetPtr = _TargetSetPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CTargetSetPtr CRenderContext::GetTargetSet() const
    {
        return m_TargetSetPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CRenderContext::SetViewPortSet(CViewPortSetPtr _ViewPortSetPtr)
    {
        m_ViewPortSetPtr = _ViewPortSetPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CViewPortSetPtr CRenderContext::GetViewPortSet() const
    {
        return m_ViewPortSetPtr;
    }
    
    // -----------------------------------------------------------------------------
    
    void CRenderContext::SetRenderState(CRenderStatePtr _RenderStatePtr)
    {
        m_RenderStatePtr = _RenderStatePtr;
    }
    
    // -----------------------------------------------------------------------------
    
    CRenderStatePtr CRenderContext::GetRenderState() const
    {
        return m_RenderStatePtr;
    }
} // namespace Gfx