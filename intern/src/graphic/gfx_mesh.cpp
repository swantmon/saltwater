
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_mesh.h"

namespace Gfx
{
    CMesh::CMesh()
        : m_NumberOfLODs(0)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    unsigned int CMesh::GetNumberOfLODs() const
    {
        return m_NumberOfLODs;
    }
    
    // -----------------------------------------------------------------------------
    
    CLODPtr CMesh::GetLOD(unsigned int _Index) const
    {
        assert(_Index < s_NumberOfLODs);
        
        return m_LODs[_Index];
    }
    
    // -----------------------------------------------------------------------------
    
    Base::AABB3Float CMesh::GetAABB() const
    {
        return m_AABB;
    }
} // namespace Gfx