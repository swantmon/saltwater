
#include "engine/engine_precompiled.h"

#include "engine/graphic/gfx_mesh.h"

namespace Gfx
{
    CMesh::CMesh()
        : m_NumberOfLODs(0)
    {
    }

    // -----------------------------------------------------------------------------

    CMesh::~CMesh()
    {
        for(auto& rLOD : m_LODs) rLOD = 0;
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