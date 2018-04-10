
#include "engine/engine_precompiled.h"

#include "data/data_hierarchy_facet.h"

namespace Dt
{
    CHierarchyFacet::CHierarchyFacet()
        : m_TimeStamp  (static_cast<Base::U64>(-1))
        , m_pParent    (nullptr)
        , m_pFirstChild(nullptr)
        , m_pSibling   (nullptr)
    {

    }

    // -----------------------------------------------------------------------------

    CHierarchyFacet::~CHierarchyFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CHierarchyFacet::SetParent(CEntity* _pEntity)
    {
        m_pParent = _pEntity;
    }

    // -----------------------------------------------------------------------------

    CEntity* CHierarchyFacet::GetParent()
    {
        return m_pParent;
    }

    // -----------------------------------------------------------------------------

    const CEntity* CHierarchyFacet::GetParent() const
    {
        return m_pParent;
    }

    // -----------------------------------------------------------------------------

    void CHierarchyFacet::SetFirstChild(CEntity* _pEntity)
    {
        m_pFirstChild = _pEntity;
    }

    // -----------------------------------------------------------------------------

    CEntity* CHierarchyFacet::GetFirstChild()
    {
        return m_pFirstChild;
    }

    // -----------------------------------------------------------------------------

    const CEntity* CHierarchyFacet::GetFirstChild() const
    {
        return m_pFirstChild;
    }

    // -----------------------------------------------------------------------------

    void CHierarchyFacet::SetSibling(CEntity* _pEntity)
    {
        m_pSibling = _pEntity;
    }

    // -----------------------------------------------------------------------------

    CEntity* CHierarchyFacet::GetSibling()
    {
        return m_pSibling;
    }

    // -----------------------------------------------------------------------------

    const CEntity* CHierarchyFacet::GetSibling() const
    {
        return m_pSibling;
    }

    // -----------------------------------------------------------------------------

    void CHierarchyFacet::SetTimeStamp(Base::U64 _TimeStamp)
    {
        m_TimeStamp = _TimeStamp;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CHierarchyFacet::GetTimeStamp()
    {
        return m_TimeStamp;
    }
} // namespace Dt