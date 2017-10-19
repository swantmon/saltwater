
#pragma once

#include "graphic/gfx_sampler.h"
#include "graphic/gfx_sampler_set.h"

namespace Gfx
{
namespace SamplerManager
{
    void OnStart();
    void OnExit();

    CSamplerPtr GetSampler(CSampler::ESampler _Sampler);

    CSamplerSetPtr CreateSamplerSet(CSamplerPtr _Sampler1Ptr);
    CSamplerSetPtr CreateSamplerSet(CSamplerPtr _Sampler1Ptr, CSamplerPtr _Sampler2Ptr);
    CSamplerSetPtr CreateSamplerSet(CSamplerPtr _Sampler1Ptr, CSamplerPtr _Sampler2Ptr, CSamplerPtr _Sampler3Ptr);
    CSamplerSetPtr CreateSamplerSet(CSamplerPtr _Sampler1Ptr, CSamplerPtr _Sampler2Ptr, CSamplerPtr _Sampler3Ptr, CSamplerPtr _Sampler4Ptr);
    CSamplerSetPtr CreateSamplerSet(CSamplerPtr* _pSamplerPtrs, unsigned int _NumberOfSamplers);

	void SetSamplerLabel(CSamplerPtr _SamplerPtr, const char* _pLabel);
} // namespace SamplerManager
} // namespace Gfx

