
#pragma once

#include "graphic/gfx_component.h"
#include "graphic/gfx_mesh.h"

namespace Gfx
{
    class CMeshComponent : public CComponent<CMeshComponent>
    {
    public:

        void SetMesh(CMeshPtr _ModelPtr);
        CMeshPtr GetMesh();

    public:

        CMeshComponent();
        ~CMeshComponent();

    protected:

        CMeshPtr m_MeshPtr;
    };
} // namespace Gfx