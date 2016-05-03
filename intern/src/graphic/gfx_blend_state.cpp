
#include "graphic/gfx_precompiled.h"

#include "base/base_typedef.h"

#include "graphic/gfx_blend_state.h"

namespace Gfx
{
    CBlendState::CBlendState()
        : m_SampleMask (0xFFFFFFFF)
        , m_Flags      (0)
        , m_Description()
    {
        unsigned int IndexOfBlendFactor;

        for (IndexOfBlendFactor = 0; IndexOfBlendFactor < s_MaxNumberOfBlendFactors; ++ IndexOfBlendFactor)
        {
            m_BlendFactors[IndexOfBlendFactor] = 1.0f;
        }
    }

    // -----------------------------------------------------------------------------

    CBlendState::~CBlendState()
    {
    }

    // -----------------------------------------------------------------------------

    void CBlendState::SetBlendFactors(const float* _pBlendFactors)
    {
        unsigned int IndexOfBlendFactor;

        if (_pBlendFactors != nullptr)
        {
            for (IndexOfBlendFactor = 0; IndexOfBlendFactor < s_MaxNumberOfBlendFactors; ++ IndexOfBlendFactor)
            {
                m_BlendFactors[IndexOfBlendFactor] = _pBlendFactors[IndexOfBlendFactor];
            }
        }
        else
        {
            for (IndexOfBlendFactor = 0; IndexOfBlendFactor < s_MaxNumberOfBlendFactors; ++ IndexOfBlendFactor)
            {
                m_BlendFactors[IndexOfBlendFactor] = 1.0f;
            }
        }
    }

    // -----------------------------------------------------------------------------

    const float* CBlendState::GetBlendFactors() const
    {
        return &m_BlendFactors[0];
    }

    // -----------------------------------------------------------------------------

    void CBlendState::SetSampleMask(unsigned int _SampleMask)
    { 
        m_SampleMask = _SampleMask; 
    }

    // -----------------------------------------------------------------------------

    unsigned int CBlendState::GetSampleMask() const 
    { 
        return m_SampleMask; 
    }
    
    // -----------------------------------------------------------------------------
    
    unsigned int CBlendState::GetFlags() const
    {
        return m_Flags;
    }
    
    // -----------------------------------------------------------------------------
    
    const SBlendDescription& CBlendState::GetDescription() const
    {
        return m_Description;
    }
} // namespace Gfx