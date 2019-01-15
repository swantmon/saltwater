#pragma once

#include "engine/engine_config.h"

#include "engine/graphic/gfx_target_set.h"
#include "engine/graphic/gfx_texture.h"

namespace Gfx
{
namespace TargetSetManager
{
    void OnStart();
    void OnExit();
    
    ENGINE_API CTargetSetPtr GetSystemTargetSet();
    ENGINE_API CTargetSetPtr GetDefaultTargetSet();
    ENGINE_API CTargetSetPtr GetDeferredTargetSet();
    ENGINE_API CTargetSetPtr GetLightAccumulationTargetSet();
    ENGINE_API CTargetSetPtr GetHitProxyTargetSet();
    
    ENGINE_API CTargetSetPtr CreateTargetSet(CTexturePtr _Target1Ptr);
    ENGINE_API CTargetSetPtr CreateTargetSet(CTexturePtr _Target1Ptr, CTexturePtr _Target2Ptr);
    ENGINE_API CTargetSetPtr CreateTargetSet(CTexturePtr _Target1Ptr, CTexturePtr _Target2Ptr, CTexturePtr _Target3Ptr);
    ENGINE_API CTargetSetPtr CreateTargetSet(CTexturePtr _Target1Ptr, CTexturePtr _Target2Ptr, CTexturePtr _Target3Ptr, CTexturePtr _Target4Ptr);
    ENGINE_API CTargetSetPtr CreateTargetSet(CTexturePtr* _pTargetPtrs, unsigned int _NumberOfTargets);
    
    ENGINE_API CTargetSetPtr CreateEmptyTargetSet(int _Width, int _Height, int _Layers = 0);

    ENGINE_API void ClearTargetSet(CTargetSetPtr _TargetPtr, const glm::vec4& _rColor, float _Depth);
    ENGINE_API void ClearTargetSet(CTargetSetPtr _TargetPtr, const glm::vec4& _rColor);
    ENGINE_API void ClearTargetSet(CTargetSetPtr _TargetPtr, float _Depth);
    ENGINE_API void ClearTargetSet(CTargetSetPtr _TargetPtr);

    ENGINE_API void SetTargetSetLabel(CTargetSetPtr _TargetSetPtr, const char* _pLabel);
} // namespace TargetSetManager
} // namespace Gfx
