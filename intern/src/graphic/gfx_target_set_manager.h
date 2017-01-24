//
//  gfx_target_set_manager.h
//  graphic
//
//  Created by Tobias Schwandt on 15/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_vector4.h"

#include "graphic/gfx_target_set.h"
#include "graphic/gfx_texture_base.h"

namespace Gfx
{
namespace TargetSetManager
{
    void OnStart();
    void OnExit();
    
    CTargetSetPtr GetSystemTargetSet();
    CTargetSetPtr GetDefaultTargetSet();
    CTargetSetPtr GetDeferredTargetSet();
    CTargetSetPtr GetLightAccumulationTargetSet();
    CTargetSetPtr GetHitProxyTargetSet();
    
    CTargetSetPtr CreateTargetSet(CTextureBasePtr _Target1Ptr);
    CTargetSetPtr CreateTargetSet(CTextureBasePtr _Target1Ptr, CTextureBasePtr _Target2Ptr);
    CTargetSetPtr CreateTargetSet(CTextureBasePtr _Target1Ptr, CTextureBasePtr _Target2Ptr, CTextureBasePtr _Target3Ptr);
    CTargetSetPtr CreateTargetSet(CTextureBasePtr _Target1Ptr, CTextureBasePtr _Target2Ptr, CTextureBasePtr _Target3Ptr, CTextureBasePtr _Target4Ptr);
    CTargetSetPtr CreateTargetSet(CTextureBasePtr* _pTargetPtrs, unsigned int _NumberOfTargets);
    
    void ClearTargetSet(CTargetSetPtr _TargetPtr, const Base::Float4& _rColor, float _Depth);
    void ClearTargetSet(CTargetSetPtr _TargetPtr, const Base::Float4& _rColor);
    void ClearTargetSet(CTargetSetPtr _TargetPtr, float _Depth);
    void ClearTargetSet(CTargetSetPtr _TargetPtr);
} // namespace TargetSetManager
} // namespace Gfx
