
#include "data/data_precompiled.h"

#include "data/data_mesh_component.h"

#include <assert.h>

namespace Dt
{
    CMeshComponent::CMeshComponent()
        : m_pMesh    (0)
        , m_pMaterial(0)
    {
    }

    // -----------------------------------------------------------------------------

    CMeshComponent::~CMeshComponent()
    {

    }

    // -----------------------------------------------------------------------------

    void CMeshComponent::SetMesh(const CMesh* _pModel)
    {
        m_pMesh = _pModel;
    }

    // -----------------------------------------------------------------------------

    const CMesh* CMeshComponent::GetMesh()
    {
        return m_pMesh;
    }

    // -----------------------------------------------------------------------------

    void CMeshComponent::SetMaterial(CMaterial* _pMaterial)
    {
        m_pMaterial = _pMaterial;
    }

    // -----------------------------------------------------------------------------

    CMaterial* CMeshComponent::GetMaterial()
    {
        return m_pMaterial;
    }
} // namespace Dt