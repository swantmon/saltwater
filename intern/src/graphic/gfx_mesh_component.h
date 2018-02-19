
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

        void SetMaterial(unsigned int _Surface, CMaterialPtr _MaterialPtr);
        CMaterialPtr GetMaterial(unsigned int _Surface);

    public:

        CMeshComponent();
        ~CMeshComponent();

    protected:

        CMeshPtr     m_ModelPtr;
        CMaterialPtr m_MaterialPtrs[CLOD::s_NumberOfSurfaces];
    };
} // namespace Gfx