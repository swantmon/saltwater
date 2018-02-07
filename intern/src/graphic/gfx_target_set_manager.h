//
//  gfx_target_set_manager.h
//  graphic
//
//  Created by Tobias Schwandt on 15/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "graphic/gfx_target_set.h"
#include "graphic/gfx_texture.h"

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
    
    CTargetSetPtr CreateTargetSet(CTexturePtr _Target1Ptr);
    CTargetSetPtr CreateTargetSet(CTexturePtr _Target1Ptr, CTexturePtr _Target2Ptr);
    CTargetSetPtr CreateTargetSet(CTexturePtr _Target1Ptr, CTexturePtr _Target2Ptr, CTexturePtr _Target3Ptr);
    CTargetSetPtr CreateTargetSet(CTexturePtr _Target1Ptr, CTexturePtr _Target2Ptr, CTexturePtr _Target3Ptr, CTexturePtr _Target4Ptr);
    CTargetSetPtr CreateTargetSet(CTexturePtr* _pTargetPtrs, unsigned int _NumberOfTargets);
    
    CTargetSetPtr CreateEmptyTargetSet(int _Width, int _Height, int _Layers = 0);

    void ClearTargetSet(CTargetSetPtr _TargetPtr, const glm::vec4& _rColor, float _Depth);
    void ClearTargetSet(CTargetSetPtr _TargetPtr, const glm::vec4& _rColor);
    void ClearTargetSet(CTargetSetPtr _TargetPtr, float _Depth);
    void ClearTargetSet(CTargetSetPtr _TargetPtr);

	void SetTargetSetLabel(CTargetSetPtr _TargetSetPtr, const char* _pLabel);
} // namespace TargetSetManager
} // namespace Gfx
