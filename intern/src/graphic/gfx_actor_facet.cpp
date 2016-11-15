
#include "graphic/gfx_precompiled.h"

#include "base/base_memory.h"

#include "graphic/gfx_actor_facet.h"

namespace Gfx
{
    CMeshActorFacet::CMeshActorFacet()
        : m_ModelPtr()
    {
        Base::CMemory::Zero(m_MaterialPtrs, sizeof(CMaterialPtr) * CLOD::s_NumberOfSurfaces);
    }

    // -----------------------------------------------------------------------------

    CMeshActorFacet::~CMeshActorFacet()
    {
        m_ModelPtr = 0;

        for (unsigned int IndexOfMaterial = 0; IndexOfMaterial < CLOD::s_NumberOfSurfaces; ++IndexOfMaterial)
        {
            m_MaterialPtrs[IndexOfMaterial] = 0;
        }
    }

    // -----------------------------------------------------------------------------

    void CMeshActorFacet::SetMesh(CMeshPtr _ModelPtr)
    {
        m_ModelPtr = _ModelPtr;
    }

    // -----------------------------------------------------------------------------

    CMeshPtr CMeshActorFacet::GetMesh()
    {
        return m_ModelPtr;
    }

    // -----------------------------------------------------------------------------

    void CMeshActorFacet::SetMaterial(unsigned int _Surface, CMaterialPtr _MaterialPtr)
    {
        assert(_Surface >= 0 && _Surface < CLOD::s_NumberOfSurfaces);

        m_MaterialPtrs[_Surface] = _MaterialPtr;
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CMeshActorFacet::GetMaterial(unsigned int _Surface)
    {
        return m_MaterialPtrs[_Surface];
    }
} // namespace Gfx

namespace Gfx
{
    CARActorFacet::CARActorFacet()
        : m_ModelPtr()
    {
        Base::CMemory::Zero(m_MaterialPtrs, sizeof(CMaterialPtr) * CLOD::s_NumberOfSurfaces);
    }

    // -----------------------------------------------------------------------------

    CARActorFacet::~CARActorFacet()
    {
        m_ModelPtr = 0;

        for (unsigned int IndexOfMaterial = 0; IndexOfMaterial < CLOD::s_NumberOfSurfaces; ++IndexOfMaterial)
        {
            m_MaterialPtrs[IndexOfMaterial] = 0;
        }
    }

    // -----------------------------------------------------------------------------

    void CARActorFacet::SetMesh(CMeshPtr _ModelPtr)
    {
        m_ModelPtr = _ModelPtr;
    }

    // -----------------------------------------------------------------------------

    CMeshPtr CARActorFacet::GetMesh()
    {
        return m_ModelPtr;
    }

    // -----------------------------------------------------------------------------

    void CARActorFacet::SetMaterial(unsigned int _Surface, CMaterialPtr _MaterialPtr)
    {
        assert(_Surface >= 0 && _Surface < CLOD::s_NumberOfSurfaces);

        m_MaterialPtrs[_Surface] = _MaterialPtr;
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CARActorFacet::GetMaterial(unsigned int _Surface)
    {
        return m_MaterialPtrs[_Surface];
    }
} // namespace Gfx

namespace Gfx
{
    CCameraActorFacet::CCameraActorFacet()
        : m_BackgroundTexture2DPtr(0)
        , m_BackgroundTextureSetPtr(0)
        , m_TimeStamp(static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CCameraActorFacet::~CCameraActorFacet()
    {
        m_BackgroundTexture2DPtr  = 0;
        m_BackgroundTextureSetPtr = 0;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetBackgroundTexture2D(CTexture2DPtr _Texture2DPtr)
    {
        m_BackgroundTexture2DPtr = _Texture2DPtr;
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr CCameraActorFacet::GetBackgroundTexture2D()
    {
        return m_BackgroundTexture2DPtr;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetBackgroundTextureSet(CTextureSetPtr _TextureSetPtr)
    {
        m_BackgroundTextureSetPtr = _TextureSetPtr;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CCameraActorFacet::GetBackgroundTextureSet()
    {
        return m_BackgroundTextureSetPtr;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetTimeStamp(Base::U64 _TimeStamp)
    {
        m_TimeStamp = _TimeStamp;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CCameraActorFacet::GetTimeStamp()
    {
        return m_TimeStamp;
    }
} // namespace Gfx