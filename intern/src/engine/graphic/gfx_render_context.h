//
//  gfx_render_context.h
//  graphic
//
//  Created by Tobias Schwandt on 23/10/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_managed_pool.h"

#include "engine/graphic/gfx_camera.h"
#include "engine/graphic/gfx_render_state.h"
#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_view_port_set.h"

namespace Gfx
{
    class ENGINE_API CRenderContext : public Base::CManagedPoolItemBase
    {
    public:
        
        void SetCamera(CCameraPtr _CameraPtr);
        CCameraPtr GetCamera() const;
        
        void SetTargetSet(CTargetSetPtr _TargetSetPtr);
        CTargetSetPtr GetTargetSet() const;
        
        void SetViewPortSet(CViewPortSetPtr _ViewPortSetPtr);
        CViewPortSetPtr GetViewPortSet() const;
        
        void SetRenderState(CRenderStatePtr _RenderStatePtr);
        CRenderStatePtr GetRenderState() const;
        
    protected:
        
        CCameraPtr      m_CameraPtr;
        CTargetSetPtr   m_TargetSetPtr;
        CViewPortSetPtr m_ViewPortSetPtr;
        CRenderStatePtr m_RenderStatePtr;
        
    protected:
        
        CRenderContext();
        ~CRenderContext();
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CRenderContext> CRenderContextPtr;
} // namespace Gfx