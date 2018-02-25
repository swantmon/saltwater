
#include "data/data_precompiled.h"

#include "data/data_mesh.h"

namespace Dt
{
    CMesh::CMesh()
        : m_Filename      ()
        , m_GeneratorFlag (0)
        , m_PredefinedMesh(Nothing)
    {
    }

    // -----------------------------------------------------------------------------

    CMesh::~CMesh()
    {

    }

    // -----------------------------------------------------------------------------

    const std::string& CMesh::GetFilename() const
    {
        return m_Filename;
    }

    // -----------------------------------------------------------------------------

    int CMesh::GetGeneratorFlag() const
    {
        return m_GeneratorFlag;
    }

    // -----------------------------------------------------------------------------

    CMesh::EPredefinedMesh CMesh::GetPredefinedMesh() const
    {
        return m_PredefinedMesh;
    }
} // namespace Dt