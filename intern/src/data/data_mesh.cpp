
#include "data/data_precompiled.h"

#include "data/data_mesh.h"

namespace Dt
{
    CMesh::CMesh()
        : m_Meshname    ()
        , m_NumberOfLODs(0)
    {
        Base::CMemory::Zero(&m_LODs, s_NumberOfLODs * sizeof(CLOD*));
    }

    // -----------------------------------------------------------------------------

    CMesh::~CMesh()
    {

    }

    // -----------------------------------------------------------------------------

    void CMesh::SetMeshname(const Base::Char* _pMeshname)
    {
        m_Meshname = _pMeshname;
    }

    // -----------------------------------------------------------------------------

    const Base::Char* CMesh::GetMeshname() const
    {
        return m_Meshname.c_str();
    }

    // -----------------------------------------------------------------------------

    void CMesh::SetNumberOfLODs(unsigned int _NumberOfLODs)
    {
        assert(_NumberOfLODs >= 0 && _NumberOfLODs < s_NumberOfLODs);

        m_NumberOfLODs = _NumberOfLODs;
    }

    // -----------------------------------------------------------------------------

    unsigned int CMesh::GetNumberOfLODs() const
    {
        return m_NumberOfLODs;
    }

    // -----------------------------------------------------------------------------

    void CMesh::SetLOD(unsigned int _Index, CLOD* _pLOD)
    {
        assert(_Index >= 0 && _Index < s_NumberOfLODs);

        m_LODs[_Index] = _pLOD;
    }

    // -----------------------------------------------------------------------------

    CLOD* CMesh::GetLOD(unsigned int _Index)
    {
        assert(_Index >= 0 && _Index < s_NumberOfLODs);

        return m_LODs[_Index];
    }

    // -----------------------------------------------------------------------------

    const CLOD* CMesh::GetLOD(unsigned int _Index) const
    {
        assert(_Index >= 0 && _Index < s_NumberOfLODs);

        return m_LODs[_Index];
    }
} // namespace Dt