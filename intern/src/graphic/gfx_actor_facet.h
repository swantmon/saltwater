
#pragma once

#include "graphic/gfx_lod.h"
#include "graphic/gfx_material.h"
#include "graphic/gfx_model.h"

namespace Gfx
{
    class CModelActorFacet
    {
    public:

        void SetModel(CModelPtr _ModelPtr);
        CModelPtr GetModel();

        void SetMaterial(CMaterialPtr _MaterialPtr);
        CMaterialPtr GetMaterial();

    public:

        CModelActorFacet();
        ~CModelActorFacet();

    protected:

        CModelPtr    m_ModelPtr;
        CMaterialPtr m_MaterialPtr;
    };
} // namespace Gfx

namespace Gfx
{
    class CARActorFacet
    {
    public:

        void SetModel(CModelPtr _ModelPtr);
        CModelPtr GetModel();

        void SetMaterial(CMaterialPtr _MaterialPtr);
        CMaterialPtr GetMaterial();

    public:

        CARActorFacet();
        ~CARActorFacet();

    protected:

        CModelPtr    m_ModelPtr;
        CMaterialPtr m_MaterialPtr;
    };
} // namespace Gfx