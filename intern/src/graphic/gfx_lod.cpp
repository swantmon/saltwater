
#include "graphic/gfx_precompiled.h"

#include "gfx_lod.h"

namespace Gfx
{
    CLOD::CLOD()
        : m_Surface(0)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CLOD::~CLOD()
    {
        m_Surface = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    CSurfacePtr CLOD::GetSurface() const
    {
        return m_Surface;
    }
} // namespace Gfx