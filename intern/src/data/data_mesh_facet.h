
#pragma once

#include "data/data_lod.h"

namespace Dt
{
    class CMesh;
    class CMaterial;
} // namespace Dt

namespace Dt
{
    class CMeshActorFacet
    {
    public:

        void SetMesh(CMesh* _pModel);
        CMesh* GetMesh();

        void SetMaterial(unsigned int _Surface, CMaterial* _pMaterial);
        CMaterial* GetMaterial(unsigned int _Surface);

    public:

        CMeshActorFacet();
        ~CMeshActorFacet();

    private:

        CMesh*     m_pModel;
        CMaterial* m_pMaterial[CLOD::s_NumberOfSurfaces];
    };
} // namespace Dt