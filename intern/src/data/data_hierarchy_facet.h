
#pragma once

#include "base/base_typedef.h"

namespace Dt
{
    class CEntity;
} // namespace Dt

namespace Dt
{
    class CHierarchyFacet
    {
    public:

        void SetParent(CEntity* _pEntity);
        CEntity* GetParent();
        const CEntity* GetParent() const;

        void SetFirstChild(CEntity* _pEntity);
        CEntity* GetFirstChild();
        const CEntity* GetFirstChild() const;

        void SetSibling(CEntity* _pEntity);
        CEntity* GetSibling();
        const CEntity* GetSibling() const;

        void SetTimeStamp(Base::U64 _TimeStamp);
        Base::U64 GetTimeStamp();

    public:

        CHierarchyFacet();
        ~CHierarchyFacet();

    private:

        Base::U64 m_TimeStamp;              //< Time stamp to detect if entity already updated in this frame
        CEntity*  m_pParent;                //< Parent entity in tree
        CEntity*  m_pFirstChild;            //< First child on next level
        CEntity*  m_pSibling;               //< Next entity in the same level
    };
} // namespace Dt
