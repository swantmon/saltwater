
#pragma once

#include "base/base_aabb2.h"
#include "base/base_matrix3x3.h"

#include "data/data_lod.h"

namespace Dt
{
    class CMesh;
    class CMaterial;
} // namespace Dt

namespace Dt
{
    class CARActorFacet
    {
    public:

        void SetModel(CMesh* _pModel);
        CMesh* GetModel();

        void SetMaterial(unsigned int _Surface, CMaterial* _pMaterial);
        CMaterial* GetMaterial(unsigned int _Surface);

    public:

        CARActorFacet();
        ~CARActorFacet();

    private:

        CMesh*     m_pModel;
        CMaterial* m_pMaterial[CLOD::s_NumberOfSurfaces];
    };
} // namespace Dt