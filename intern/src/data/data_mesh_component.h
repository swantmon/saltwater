
#pragma once

#include "data/data_component.h"
#include "data/data_lod.h"

namespace Dt
{
    class CMesh;
    class CMaterial;
} // namespace Dt

namespace Dt
{
    class CMeshComponent : public CComponent<CMeshComponent>
    {
    public:

        void SetMesh(CMesh* _pModel);
        CMesh* GetMesh();

        void SetMaterial(unsigned int _Surface, CMaterial* _pMaterial);
        CMaterial* GetMaterial(unsigned int _Surface);

    public:

        CMeshComponent();
        ~CMeshComponent();

    private:

        CMesh*     m_pModel;
        CMaterial* m_pMaterial[CLOD::s_NumberOfSurfaces];
    };
} // namespace Dt