
#include "data/data_precompiled.h"

#include "data/data_mesh_facet.h"

namespace Dt
{
    CMeshActorFacet::CMeshActorFacet()
        : m_pModel(nullptr)
    {
        Base::CMemory::Zero(m_pMaterial, sizeof(CMaterial*) * CLOD::s_NumberOfSurfaces);
    }

    // -----------------------------------------------------------------------------

    CMeshActorFacet::~CMeshActorFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CMeshActorFacet::SetMesh(CMesh* _pModel)
    {
        m_pModel = _pModel;
    }

    // -----------------------------------------------------------------------------

    CMesh* CMeshActorFacet::GetMesh()
    {
        return m_pModel;
    }

    // -----------------------------------------------------------------------------

    void CMeshActorFacet::SetMaterial(unsigned int _Surface, CMaterial* _pMaterial)
    {
        assert(_Surface >= 0 && _Surface < CLOD::s_NumberOfSurfaces);

        m_pMaterial[_Surface] = _pMaterial;
    }

    // -----------------------------------------------------------------------------

    CMaterial* CMeshActorFacet::GetMaterial(unsigned int _Surface)
    {
        assert(_Surface >= 0 && _Surface < CLOD::s_NumberOfSurfaces);

        return m_pMaterial[_Surface];
    }
} // namespace Dt