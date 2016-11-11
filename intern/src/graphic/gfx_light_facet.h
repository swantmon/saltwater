
#pragma once

#include "base/base_typedef.h"

#include "graphic/gfx_render_context.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_set.h"

namespace Gfx
{
    class CPointLightFacet
    {
    public:

        void SetRenderContext(CRenderContextPtr _RenderContextPtr);
        CRenderContextPtr GetRenderContext();

        void SetTextureSMSet(CTextureSetPtr _TextureSMPtr);
        CTextureSetPtr GetTextureSMSet();

        void SetTimeStamp(Base::U64 _TimeStamp);
        Base::U64 GetTimeStamp();

    public:

        CPointLightFacet();
        ~CPointLightFacet();

    protected:

        CRenderContextPtr m_RenderContextPtr;
        CTextureSetPtr    m_TextureSMPtr;
        Base::U64         m_TimeStamp;
    };
} // namespace Gfx

namespace Gfx
{
    class CSunLightFacet
    {
    public:

        void SetRenderContext(CRenderContextPtr _RenderContextPtr);
        CRenderContextPtr GetRenderContext();

        void SetTextureSMSet(CTextureSetPtr _TextureSMPtr);
        CTextureSetPtr GetTextureSMSet();

        void SetTimeStamp(Base::U64 _TimeStamp);
        Base::U64 GetTimeStamp();

    public:

        CSunLightFacet();
        ~CSunLightFacet();

    protected:

        CRenderContextPtr m_RenderContextPtr;
        CTextureSetPtr    m_TextureSMPtr;
        Base::U64         m_TimeStamp;
    };
} // namespace Gfx

namespace Gfx
{
    class CGlobalProbeLightFacet
    {
    public:

        typedef std::vector<CTargetSetPtr>   CTargetSets;
        typedef std::vector<CViewPortSetPtr> CViewPortSets;

    public:

        void SetCustomTexture2D(CTexture2DPtr _CustomTexture2DPtr);
        CTexture2DPtr GetCustomTexture2D();

        void SetCustomTextureSet(CTextureSetPtr _CustomTextureSetPtr);
        CTextureSetPtr GetCustomTextureSet();

        void SetCustomHDRTexture2D(CTexture2DPtr _CustomHDRTexture2DPtr);
        CTexture2DPtr GetCustomHDRTexture2D();

        void SetFilteredTextureSet(CTextureSetPtr _FilteredTextureSetPtr);
        CTextureSetPtr GetFilteredTextureSet();

        void SetCustomHDRTextureSet(CTextureSetPtr _CustomHDRTextureSetPtr);
        CTextureSetPtr GetCustomHDRTextureSet();

        void SetCustomHDRTargetSet(CTargetSetPtr _CustomHDRTargetSetPtr);
        CTargetSetPtr GetCustomHDRTargetSet();

        void SetCustomViewPortSet(CViewPortSetPtr _CustomViewPortSetPtr);
        CViewPortSetPtr GetCustomViewPortSet();

        void SetSpecularHDRTargetSet(CTargetSets _SpecularHDRTargetSetPtrs);
        CTargetSets& GetSpecularHDRTargetSets();

        void SetSpecularViewPortSet(CViewPortSets _SpecularViewPortSetPtrs);
        CViewPortSets& GetSpecularViewPortSets();

        void SetDiffuseHDRTargetSet(CTargetSetPtr _DiffuseHDRTargetSetPtr);
        CTargetSetPtr GetDiffuseHDRTargetSet();

        void SetDiffuseViewPortSet(CViewPortSetPtr _DiffuseViewPortSetPtr);
        CViewPortSetPtr GetDiffuseViewPortSet();

        void SetTimeStamp(Base::U64 _TimeStamp);
        Base::U64 GetTimeStamp();

    public:

        CGlobalProbeLightFacet();
        ~CGlobalProbeLightFacet();

    private:

        CTexture2DPtr  m_CustomTexture2DPtr;
        CTextureSetPtr m_CustomTextureSetPtr;

        CTexture2DPtr m_CustomHDRTexture2DPtr;

        CTextureSetPtr m_FilteredTextureSetPtr;
        CTextureSetPtr m_CustomHDRTextureSetPtr;

        CTargetSetPtr   m_CustomHDRTargetSetPtr;
        CViewPortSetPtr m_CustomViewPortSetPtr;

        CTargetSets   m_SpecularHDRTargetSetPtrs;
        CViewPortSets m_SpecularViewPortSetPtrs;

        CTargetSetPtr   m_DiffuseHDRTargetSetPtr;
        CViewPortSetPtr m_DiffuseViewPortSetPtr;

        Base::U64 m_TimeStamp;
    };
} // namespace Gfx

namespace Gfx
{
    class CSkyboxFacet
    {
    public:

        void SetCubemapTexture2D(CTexture2DPtr _CustomTexture2DPtr);
        CTexture2DPtr GetCubemapTexture2D();

        void SetCubemapTextureSet(CTextureSetPtr _CustomTextureSetPtr);
        CTextureSetPtr GetCubemapTextureSet();

        void SetPanoramaTexture2D(CTexture2DPtr _Texture2DPtr);
        CTexture2DPtr GetPanoramaTexture2D();

        void SetPanoramaTextureSet(CTextureSetPtr _TextureSetPtr);
        CTextureSetPtr GetPanoramaTextureSet();

        void SetTimeStamp(Base::U64 _TimeStamp);
        Base::U64 GetTimeStamp();

    public:

        CSkyboxFacet();
        ~CSkyboxFacet();

    private:

        CTexture2DPtr     m_CubemapTexture2DPtr;
        CTextureSetPtr    m_CubemapTextureSetPtr;
        CTexture2DPtr     m_BackgroundPanoramaTexture2DPtr;
        CTextureSetPtr    m_BackgroundPanoramaTextureSetPtr;
        Base::U64         m_TimeStamp;
    };
} // namespace Gfx