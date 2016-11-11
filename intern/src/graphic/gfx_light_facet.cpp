
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_light_facet.h"

namespace Gfx
{
    CPointLightFacet::CPointLightFacet()
        : m_RenderContextPtr(0)
        , m_TextureSMPtr    (0)
        , m_TimeStamp       (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CPointLightFacet::~CPointLightFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::SetRenderContext(CRenderContextPtr _RenderContextPtr)
    {
        m_RenderContextPtr = _RenderContextPtr;
    }

    // -----------------------------------------------------------------------------

    CRenderContextPtr CPointLightFacet::GetRenderContext()
    {
        return m_RenderContextPtr;
    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::SetTextureSMSet(CTextureSetPtr _TextureSMPtr)
    {
        m_TextureSMPtr = _TextureSMPtr;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CPointLightFacet::GetTextureSMSet()
    {
        return m_TextureSMPtr;
    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::SetTimeStamp(Base::U64 _TimeStamp)
    {
        m_TimeStamp = _TimeStamp;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CPointLightFacet::GetTimeStamp()
    {
        return m_TimeStamp;
    }
} // namespace Gfx

namespace Gfx
{
    CSunLightFacet::CSunLightFacet()
        : m_RenderContextPtr(0)
        , m_TextureSMPtr    (0)
        , m_TimeStamp       (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CSunLightFacet::~CSunLightFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CSunLightFacet::SetRenderContext(CRenderContextPtr _RenderContextPtr)
    {
        m_RenderContextPtr = _RenderContextPtr;
    }

    // -----------------------------------------------------------------------------

    CRenderContextPtr CSunLightFacet::GetRenderContext()
    {
        return m_RenderContextPtr;
    }

    // -----------------------------------------------------------------------------

    void CSunLightFacet::SetTextureSMSet(CTextureSetPtr _TextureSMPtr)
    {
        m_TextureSMPtr = _TextureSMPtr;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CSunLightFacet::GetTextureSMSet()
    {
        return m_TextureSMPtr;
    }

    // -----------------------------------------------------------------------------

    void CSunLightFacet::SetTimeStamp(Base::U64 _TimeStamp)
    {
        m_TimeStamp = _TimeStamp;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CSunLightFacet::GetTimeStamp()
    {
        return m_TimeStamp;
    }
} // namespace Gfx

namespace Gfx
{
    CGlobalProbeLightFacet::CGlobalProbeLightFacet()
        : m_CustomTexture2DPtr    (0)
        , m_CustomTextureSetPtr   (0)
        , m_CustomHDRTexture2DPtr (0)
        , m_FilteredTextureSetPtr (0)
        , m_CustomHDRTextureSetPtr(0)
        , m_CustomHDRTargetSetPtr (0)
        , m_CustomViewPortSetPtr  (0)
        , m_DiffuseHDRTargetSetPtr(0)
        , m_DiffuseViewPortSetPtr (0)
        , m_TimeStamp             (static_cast<Base::U64>(-1))
    {
        m_SpecularHDRTargetSetPtrs.clear();
        m_SpecularViewPortSetPtrs .clear();
    }

    // -----------------------------------------------------------------------------

    CGlobalProbeLightFacet::~CGlobalProbeLightFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CGlobalProbeLightFacet::SetCustomTexture2D(CTexture2DPtr _CustomTexture2DPtr)
    {
        m_CustomTexture2DPtr = _CustomTexture2DPtr;
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr CGlobalProbeLightFacet::GetCustomTexture2D()
    {
        return m_CustomTexture2DPtr;
    }

    // -----------------------------------------------------------------------------

    void CGlobalProbeLightFacet::SetCustomTextureSet(CTextureSetPtr _CustomTextureSetPtr)
    {
        m_CustomTextureSetPtr = _CustomTextureSetPtr;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CGlobalProbeLightFacet::GetCustomTextureSet()
    {
        return m_CustomTextureSetPtr;
    }

    // -----------------------------------------------------------------------------

    void CGlobalProbeLightFacet::SetCustomHDRTexture2D(CTexture2DPtr _CustomHDRTexture2DPtr)
    {
        m_CustomHDRTexture2DPtr = _CustomHDRTexture2DPtr;
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr CGlobalProbeLightFacet::GetCustomHDRTexture2D()
    {
        return m_CustomHDRTexture2DPtr;
    }

    // -----------------------------------------------------------------------------

    void CGlobalProbeLightFacet::SetFilteredTextureSet(CTextureSetPtr _FilteredTextureSetPtr)
    {
        m_FilteredTextureSetPtr = _FilteredTextureSetPtr;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CGlobalProbeLightFacet::GetFilteredTextureSet()
    {
        return m_FilteredTextureSetPtr;
    }

    // -----------------------------------------------------------------------------

    void CGlobalProbeLightFacet::SetCustomHDRTextureSet(CTextureSetPtr _CustomHDRTextureSetPtr)
    {
        m_CustomHDRTextureSetPtr = _CustomHDRTextureSetPtr;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CGlobalProbeLightFacet::GetCustomHDRTextureSet()
    {
        return m_CustomHDRTextureSetPtr;
    }

    // -----------------------------------------------------------------------------

    void CGlobalProbeLightFacet::SetCustomHDRTargetSet(CTargetSetPtr _CustomHDRTargetSetPtr)
    {
        m_CustomHDRTargetSetPtr = _CustomHDRTargetSetPtr;
    }

    // -----------------------------------------------------------------------------

    CTargetSetPtr CGlobalProbeLightFacet::GetCustomHDRTargetSet()
    {
        return m_CustomHDRTargetSetPtr;
    }

    // -----------------------------------------------------------------------------

    void CGlobalProbeLightFacet::SetCustomViewPortSet(CViewPortSetPtr _CustomViewPortSetPtr)
    {
        m_CustomViewPortSetPtr = _CustomViewPortSetPtr;
    }

    // -----------------------------------------------------------------------------

    CViewPortSetPtr CGlobalProbeLightFacet::GetCustomViewPortSet()
    {
        return m_CustomViewPortSetPtr;
    }

    // -----------------------------------------------------------------------------

    void CGlobalProbeLightFacet::SetSpecularHDRTargetSet(CTargetSets _SpecularHDRTargetSetPtrs)
    {
        m_SpecularHDRTargetSetPtrs = _SpecularHDRTargetSetPtrs;
    }

    // -----------------------------------------------------------------------------

    CGlobalProbeLightFacet::CTargetSets& CGlobalProbeLightFacet::GetSpecularHDRTargetSets()
    {
        return m_SpecularHDRTargetSetPtrs;
    }

    // -----------------------------------------------------------------------------

    void CGlobalProbeLightFacet::SetSpecularViewPortSet(CViewPortSets _SpecularViewPortSetPtrs)
    {
        m_SpecularViewPortSetPtrs = _SpecularViewPortSetPtrs;
    }

    // -----------------------------------------------------------------------------

    CGlobalProbeLightFacet::CViewPortSets& CGlobalProbeLightFacet::GetSpecularViewPortSets()
    {
        return m_SpecularViewPortSetPtrs;
    }

    // -----------------------------------------------------------------------------

    void CGlobalProbeLightFacet::SetDiffuseHDRTargetSet(CTargetSetPtr _DiffuseHDRTargetSetPtr)
    {
        m_DiffuseHDRTargetSetPtr = _DiffuseHDRTargetSetPtr;
    }

    // -----------------------------------------------------------------------------

    CTargetSetPtr CGlobalProbeLightFacet::GetDiffuseHDRTargetSet()
    {
        return m_DiffuseHDRTargetSetPtr;
    }

    // -----------------------------------------------------------------------------

    void CGlobalProbeLightFacet::SetDiffuseViewPortSet(CViewPortSetPtr _DiffuseViewPortSetPtr)
    {
        m_DiffuseViewPortSetPtr = _DiffuseViewPortSetPtr;
    }

    // -----------------------------------------------------------------------------

    CViewPortSetPtr CGlobalProbeLightFacet::GetDiffuseViewPortSet()
    {
        return m_DiffuseViewPortSetPtr;
    }

    // -----------------------------------------------------------------------------

    void CGlobalProbeLightFacet::SetTimeStamp(Base::U64 _TimeStamp)
    {
        m_TimeStamp = _TimeStamp;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CGlobalProbeLightFacet::GetTimeStamp()
    {
        return m_TimeStamp;
    }
} // namespace Gfx

namespace Gfx
{
    CSkyboxFacet::CSkyboxFacet()
        : m_CubemapTexture2DPtr            (0)
        , m_CubemapTextureSetPtr           (0)
        , m_BackgroundPanoramaTexture2DPtr (0)
        , m_BackgroundPanoramaTextureSetPtr(0)
        , m_TimeStamp                      (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CSkyboxFacet::~CSkyboxFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CSkyboxFacet::SetCubemapTexture2D(CTexture2DPtr _CustomTexture2DPtr)
    {
        m_CubemapTexture2DPtr = _CustomTexture2DPtr;
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr CSkyboxFacet::GetCubemapTexture2D()
    {
        return m_CubemapTexture2DPtr;
    }

    // -----------------------------------------------------------------------------

    void CSkyboxFacet::SetCubemapTextureSet(CTextureSetPtr _CustomTextureSetPtr)
    {
        m_CubemapTextureSetPtr = _CustomTextureSetPtr;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CSkyboxFacet::GetCubemapTextureSet()
    {
        return m_CubemapTextureSetPtr;
    }

    // -----------------------------------------------------------------------------

    void CSkyboxFacet::SetPanoramaTexture2D(CTexture2DPtr _Texture2DPtr)
    {
        m_BackgroundPanoramaTexture2DPtr = _Texture2DPtr;
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr CSkyboxFacet::GetPanoramaTexture2D()
    {
        return m_BackgroundPanoramaTexture2DPtr;
    }

    // -----------------------------------------------------------------------------

    void CSkyboxFacet::SetPanoramaTextureSet(CTextureSetPtr _TextureSetPtr)
    {
        m_BackgroundPanoramaTextureSetPtr = _TextureSetPtr;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CSkyboxFacet::GetPanoramaTextureSet()
    {
        return m_BackgroundPanoramaTextureSetPtr;
    }

    // -----------------------------------------------------------------------------

    void CSkyboxFacet::SetTimeStamp(Base::U64 _TimeStamp)
    {
        m_TimeStamp = _TimeStamp;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CSkyboxFacet::GetTimeStamp()
    {
        return m_TimeStamp;
    }
} // namespace Gfx