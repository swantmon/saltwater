
#include "data/data_precompiled.h"

#include "data/data_mesh_component.h"

namespace Dt
{
    CMeshComponent::CMeshComponent()
        : m_pModel(nullptr)
    {
        Base::CMemory::Zero(m_pMaterial, sizeof(CMaterial*) * CLOD::s_NumberOfSurfaces);
    }

    // -----------------------------------------------------------------------------

    CMeshComponent::~CMeshComponent()
    {

    }

    // -----------------------------------------------------------------------------

    void CMeshComponent::SetMesh(CMesh* _pModel)
    {
        m_pModel = _pModel;
    }

    // -----------------------------------------------------------------------------

    CMesh* CMeshComponent::GetMesh()
    {
        return m_pModel;
    }

    // -----------------------------------------------------------------------------

    void CMeshComponent::SetMaterial(unsigned int _Surface, CMaterial* _pMaterial)
    {
        assert(_Surface >= 0 && _Surface < CLOD::s_NumberOfSurfaces);

        m_pMaterial[_Surface] = _pMaterial;
    }

    // -----------------------------------------------------------------------------

    CMaterial* CMeshComponent::GetMaterial(unsigned int _Surface)
    {
        assert(_Surface >= 0 && _Surface < CLOD::s_NumberOfSurfaces);

        return m_pMaterial[_Surface];
    }
} // namespace Dt