
#pragma once

#include "data/data_component.h"

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

        void SetMesh(const CMesh* _pMesh);
        const CMesh* GetMesh();

        void SetMaterial(CMaterial* _pMaterial);
        CMaterial* GetMaterial();

    public:

        CMeshComponent();
        ~CMeshComponent();

    private:

        const CMesh* m_pMesh;
        CMaterial* m_pMaterial;
    };
} // namespace Dt