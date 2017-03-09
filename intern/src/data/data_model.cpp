
#include "data/data_precompiled.h"

#include "data/data_model.h"

namespace Dt
{
    CModel::CModel()
        : m_Modelname()
    {
    }

    // -----------------------------------------------------------------------------

    CModel::~CModel()
    {

    }

    // -----------------------------------------------------------------------------

    const std::string& CModel::GetModelname() const
    {
        return m_Modelname;
    }

    // -----------------------------------------------------------------------------

    unsigned int CModel::GetNumberOfMeshes() const
    {
        return static_cast<unsigned int>(m_Meshes.size());
    }

    // -----------------------------------------------------------------------------

    CMesh& CModel::GetMesh(unsigned int _ID)
    {
        assert(_ID >= 0 && _ID < m_Meshes.size());

        return *m_Meshes.at(_ID);
    }

    // -----------------------------------------------------------------------------

    CMesh& CModel::GetMesh(unsigned int _ID) const
    {
        assert(_ID >= 0 && _ID < m_Meshes.size());

        return *m_Meshes.at(_ID);
    }
} // namespace Dt