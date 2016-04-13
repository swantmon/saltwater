
#include "graphic/gfx_view_port_set.h"

namespace Gfx
{
    CViewPortSet::CViewPortSet()
        : m_NumberOfViewPorts(0)
    {
    }

    // -----------------------------------------------------------------------------

    CViewPortSet::~CViewPortSet()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    unsigned int CViewPortSet::GetNumberOfViewPorts() const
    {
        return m_NumberOfViewPorts;
    }
    
    // -----------------------------------------------------------------------------
    
    CViewPortPtr* CViewPortSet::GetViewPorts()
    {
        return m_ViewPortPtrs;
    }
    
    // -----------------------------------------------------------------------------
    
    const CViewPortPtr* CViewPortSet::GetViewPorts() const
    {
        return m_ViewPortPtrs;
    }
} // namespace Gfx
