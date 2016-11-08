
#include "data/data_precompiled.h"

#include "data/data_lod.h"

namespace Dt
{
	
    CLOD::CLOD()
        : m_NumberOfSurfaces(0)
    {
        Base::CMemory::Zero(m_Surfaces, s_NumberOfSurfaces * sizeof(CSurface*));
    }

    // -----------------------------------------------------------------------------

    CLOD::~CLOD()
    {

    }

    // -----------------------------------------------------------------------------

    void CLOD::SetNumberOfSurfaces(unsigned int _NumberOfSurfaces)
    {
        m_NumberOfSurfaces = _NumberOfSurfaces;
    }

    // -----------------------------------------------------------------------------

    unsigned int CLOD::GetNumberOfSurfaces() const
    {
        return m_NumberOfSurfaces;
    }

    // -----------------------------------------------------------------------------

    void CLOD::SetSurface(unsigned int _Index, CSurface* _pSurface)
    {
        assert(_Index >= 0 && _Index < s_NumberOfSurfaces);

        m_Surfaces[_Index] = _pSurface;
    }

    // -----------------------------------------------------------------------------

    CSurface* CLOD::GetSurface(unsigned int _Index)
    {
        assert(_Index >= 0 && _Index < s_NumberOfSurfaces);

        return m_Surfaces[_Index];
    }

    // -----------------------------------------------------------------------------

    const CSurface* CLOD::GetSurface(unsigned int _Index) const
    {
        assert(_Index >= 0 && _Index < s_NumberOfSurfaces);

        return m_Surfaces[_Index];
    }
} // namespace Dt