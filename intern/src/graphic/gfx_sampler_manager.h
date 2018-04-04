
#pragma once

#include "graphic/gfx_export.h"
#include "graphic/gfx_sampler.h"
#include "graphic/gfx_sampler_set.h"

namespace Gfx
{
namespace SamplerManager
{
    GFX_API void OnStart();
    GFX_API void OnExit();

    GFX_API CSamplerPtr GetSampler(CSampler::ESampler _Sampler);

    GFX_API CSamplerSetPtr CreateSamplerSet(CSamplerPtr _Sampler1Ptr);
    GFX_API CSamplerSetPtr CreateSamplerSet(CSamplerPtr _Sampler1Ptr, CSamplerPtr _Sampler2Ptr);
    GFX_API CSamplerSetPtr CreateSamplerSet(CSamplerPtr _Sampler1Ptr, CSamplerPtr _Sampler2Ptr, CSamplerPtr _Sampler3Ptr);
    GFX_API CSamplerSetPtr CreateSamplerSet(CSamplerPtr _Sampler1Ptr, CSamplerPtr _Sampler2Ptr, CSamplerPtr _Sampler3Ptr, CSamplerPtr _Sampler4Ptr);
    GFX_API CSamplerSetPtr CreateSamplerSet(CSamplerPtr* _pSamplerPtrs, unsigned int _NumberOfSamplers);

    GFX_API void SetSamplerLabel(CSamplerPtr _SamplerPtr, const char* _pLabel);
} // namespace SamplerManager
} // namespace Gfx

