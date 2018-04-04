#pragma once

#include "graphic/gfx_export.h"
#include "graphic/gfx_target_set.h"
#include "graphic/gfx_texture.h"

namespace Gfx
{
namespace TargetSetManager
{
    GFX_API void OnStart();
    GFX_API void OnExit();
    
    GFX_API CTargetSetPtr GetSystemTargetSet();
    GFX_API CTargetSetPtr GetDefaultTargetSet();
    GFX_API CTargetSetPtr GetDeferredTargetSet();
    GFX_API CTargetSetPtr GetLightAccumulationTargetSet();
    GFX_API CTargetSetPtr GetHitProxyTargetSet();
    
    GFX_API CTargetSetPtr CreateTargetSet(CTexturePtr _Target1Ptr);
    GFX_API CTargetSetPtr CreateTargetSet(CTexturePtr _Target1Ptr, CTexturePtr _Target2Ptr);
    GFX_API CTargetSetPtr CreateTargetSet(CTexturePtr _Target1Ptr, CTexturePtr _Target2Ptr, CTexturePtr _Target3Ptr);
    GFX_API CTargetSetPtr CreateTargetSet(CTexturePtr _Target1Ptr, CTexturePtr _Target2Ptr, CTexturePtr _Target3Ptr, CTexturePtr _Target4Ptr);
    GFX_API CTargetSetPtr CreateTargetSet(CTexturePtr* _pTargetPtrs, unsigned int _NumberOfTargets);
    
    GFX_API CTargetSetPtr CreateEmptyTargetSet(int _Width, int _Height, int _Layers = 0);

    GFX_API void ClearTargetSet(CTargetSetPtr _TargetPtr, const glm::vec4& _rColor, float _Depth);
    GFX_API void ClearTargetSet(CTargetSetPtr _TargetPtr, const glm::vec4& _rColor);
    GFX_API void ClearTargetSet(CTargetSetPtr _TargetPtr, float _Depth);
    GFX_API void ClearTargetSet(CTargetSetPtr _TargetPtr);

    GFX_API void SetTargetSetLabel(CTargetSetPtr _TargetSetPtr, const char* _pLabel);
} // namespace TargetSetManager
} // namespace Gfx
