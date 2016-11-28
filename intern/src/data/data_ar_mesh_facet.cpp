
#include "data/data_precompiled.h"

#include "data/data_ar_mesh_facet.h"

namespace Dt
{
    CARActorFacet::CARActorFacet()
        : m_pModel(nullptr)
    {
        Base::CMemory::Zero(m_pMaterial, sizeof(CMaterial*) * CLOD::s_NumberOfSurfaces);
    }

    // -----------------------------------------------------------------------------

    CARActorFacet::~CARActorFacet()
    {

    }

    // -----------------------------------------------------------------------------


    void CARActorFacet::SetModel(CMesh* _pAR)
    {
        m_pModel = _pAR;
    }

    // -----------------------------------------------------------------------------

    CMesh* CARActorFacet::GetModel()
    {
        return m_pModel;
    }

    // -----------------------------------------------------------------------------

    void CARActorFacet::SetMaterial(unsigned int _Surface, CMaterial* _pMaterial)
    {
        assert(_Surface >= 0 && _Surface < CLOD::s_NumberOfSurfaces);

        m_pMaterial[_Surface] = _pMaterial;
    }

    // -----------------------------------------------------------------------------

    CMaterial* CARActorFacet::GetMaterial(unsigned int _Surface)
    {
        assert(_Surface >= 0 && _Surface < CLOD::s_NumberOfSurfaces);

        return m_pMaterial[_Surface];
    }
} // namespace Dt