
#include "data/data_precompiled.h"

#include "data/data_model.h"

namespace Dt
{
    CModel::CModel()
        : m_Modelname   ()
        , m_NumberOfLODs(0)
    {
        Base::CMemory::Zero(&m_LODs, s_NumberOfLODs * sizeof(CLOD*));
    }

    // -----------------------------------------------------------------------------

    CModel::~CModel()
    {

    }

    // -----------------------------------------------------------------------------

    void CModel::SetModelname(const char* _pModelname)
    {
        m_Modelname = _pModelname;
    }

    // -----------------------------------------------------------------------------

    const char* CModel::GetModelname() const
    {
        return m_Modelname.GetConst();
    }

    // -----------------------------------------------------------------------------

    void CModel::SetNumberOfLODs(unsigned int _NumberOfLODs)
    {
        assert(_NumberOfLODs >= 0 && _NumberOfLODs < s_NumberOfLODs);

        m_NumberOfLODs = _NumberOfLODs;
    }

    // -----------------------------------------------------------------------------

    unsigned int CModel::GetNumberOfLODs() const
    {
        return m_NumberOfLODs;
    }

    // -----------------------------------------------------------------------------

    void CModel::SetLOD(unsigned int _Index, CLOD* _pLOD)
    {
        assert(_Index >= 0 && _Index < s_NumberOfLODs);

        m_LODs[_Index] = _pLOD;
    }

    // -----------------------------------------------------------------------------

    CLOD* CModel::GetLOD(unsigned int _Index)
    {
        assert(_Index >= 0 && _Index < s_NumberOfLODs);

        return m_LODs[_Index];
    }

    // -----------------------------------------------------------------------------

    const CLOD* CModel::GetLOD(unsigned int _Index) const
    {
        assert(_Index >= 0 && _Index < s_NumberOfLODs);

        return m_LODs[_Index];
    }
} // namespace Dt