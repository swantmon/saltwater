
#pragma once

#include "graphic/gfx_component.h"
#include "graphic/gfx_lod.h"
#include "graphic/gfx_material.h"
#include "graphic/gfx_mesh.h"

namespace Gfx
{
    class CMeshComponent : public CComponent<CMeshComponent>
    {
    public:

        void SetMesh(CMeshPtr _ModelPtr);
        CMeshPtr GetMesh();

        void SetMaterial(CMaterialPtr _MaterialPtr);
        CMaterialPtr GetMaterial();

    public:

        CMeshComponent();
        ~CMeshComponent();

    protected:

        CMeshPtr     m_ModelPtr;
        CMaterialPtr m_MaterialPtr;
    };
} // namespace Gfx