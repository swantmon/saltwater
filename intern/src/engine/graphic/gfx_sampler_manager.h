
#pragma once

#include "engine/engine_config.h"

#include "engine/graphic/gfx_sampler.h"
#include "engine/graphic/gfx_sampler_set.h"

namespace Gfx
{
namespace SamplerManager
{
    void OnStart();
    void OnExit();

    ENGINE_API CSamplerPtr GetSampler(CSampler::ESampler _Sampler);

    ENGINE_API CSamplerSetPtr CreateSamplerSet(CSamplerPtr _Sampler1Ptr);
    ENGINE_API CSamplerSetPtr CreateSamplerSet(CSamplerPtr _Sampler1Ptr, CSamplerPtr _Sampler2Ptr);
    ENGINE_API CSamplerSetPtr CreateSamplerSet(CSamplerPtr _Sampler1Ptr, CSamplerPtr _Sampler2Ptr, CSamplerPtr _Sampler3Ptr);
    ENGINE_API CSamplerSetPtr CreateSamplerSet(CSamplerPtr _Sampler1Ptr, CSamplerPtr _Sampler2Ptr, CSamplerPtr _Sampler3Ptr, CSamplerPtr _Sampler4Ptr);
    ENGINE_API CSamplerSetPtr CreateSamplerSet(CSamplerPtr* _pSamplerPtrs, unsigned int _NumberOfSamplers);

    ENGINE_API void SetSamplerLabel(CSamplerPtr _SamplerPtr, const char* _pLabel);
} // namespace SamplerManager
} // namespace Gfx

