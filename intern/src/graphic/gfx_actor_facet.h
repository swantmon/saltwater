
#pragma once

#include "graphic/gfx_lod.h"
#include "graphic/gfx_material.h"
#include "graphic/gfx_mesh.h"
#include "graphic/gfx_texture_2d.h"

namespace Gfx
{
    class CMeshActorFacet
    {
    public:

        void SetMesh(CMeshPtr _ModelPtr);
        CMeshPtr GetMesh();

        void SetMaterial(unsigned int _Surface, CMaterialPtr _MaterialPtr);
        CMaterialPtr GetMaterial(unsigned int _Surface);

    public:

        CMeshActorFacet();
        ~CMeshActorFacet();

    protected:

        CMeshPtr    m_ModelPtr;
        CMaterialPtr m_MaterialPtrs[CLOD::s_NumberOfSurfaces];
    };
} // namespace Gfx

namespace Gfx
{
    class CARActorFacet
    {
    public:

        void SetMesh(CMeshPtr _ModelPtr);
        CMeshPtr GetMesh();

        void SetMaterial(unsigned int _Surface, CMaterialPtr _MaterialPtr);
        CMaterialPtr GetMaterial(unsigned int _Surface);

    public:

        CARActorFacet();
        ~CARActorFacet();

    protected:

        CMeshPtr    m_ModelPtr;
        CMaterialPtr m_MaterialPtrs[CLOD::s_NumberOfSurfaces];
    };
} // namespace Gfx

namespace Gfx
{
    class CCameraActorFacet
    {
    public:

        void SetBackgroundTexture2D(CTexture2DPtr _Texture2DPtr);
        CTexture2DPtr GetBackgroundTexture2D();

        void SetBackgroundTextureSet(CTextureSetPtr _TextureSetPtr);
        CTextureSetPtr GetBackgroundTextureSet();

        void SetTimeStamp(Base::U64 _TimeStamp);
        Base::U64 GetTimeStamp();

    public:

        CCameraActorFacet();
        ~CCameraActorFacet();

    protected:

        CTexture2DPtr     m_BackgroundTexture2DPtr;
        CTextureSetPtr    m_BackgroundTextureSetPtr;
        Base::U64         m_TimeStamp;
    };
} // namespace Gfx