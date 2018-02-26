
#pragma once

#include "graphic/gfx_component.h"
#include "graphic/gfx_material.h"

namespace Gfx
{
    class CMaterialComponent : public CComponent<CMaterialComponent>
    {
    public:

        void SetMaterial(CMaterialPtr _MaterialPtr);
        CMaterialPtr GetMaterial();

    public:

        CMaterialComponent();
        ~CMaterialComponent();

    protected:

        CMaterialPtr m_MaterialPtr;
    };
} // namespace Gfx