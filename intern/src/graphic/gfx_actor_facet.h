
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

        void SetMaterial(unsigned int _Surface, CMaterialPtr _MaterialPtr);
        CMaterialPtr GetMaterial(unsigned int _Surface);

    public:

        CModelActorFacet();
        ~CModelActorFacet();

    protected:

        CModelPtr    m_ModelPtr;
        CMaterialPtr m_MaterialPtrs[CLOD::s_NumberOfSurfaces];
    };
} // namespace Gfx

namespace Gfx
{
    class CARActorFacet
    {
    public:

        void SetModel(CModelPtr _ModelPtr);
        CModelPtr GetModel();

        void SetMaterial(unsigned int _Surface, CMaterialPtr _MaterialPtr);
        CMaterialPtr GetMaterial(unsigned int _Surface);

    public:

        CARActorFacet();
        ~CARActorFacet();

    protected:

        CModelPtr    m_ModelPtr;
        CMaterialPtr m_MaterialPtrs[CLOD::s_NumberOfSurfaces];
    };
} // namespace Gfx