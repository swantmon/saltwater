
#pragma once

#include "engine/engine_config.h"

#include "base/base_serialize_std_string.h"
#include "base/base_typedef.h"

#include "engine/data/data_entity_manager.h"

namespace Dt
{
    class CEntity;
} // namespace Dt

namespace Dt
{
    class ENGINE_API CHierarchyFacet
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

    public:

        template <class TArchive>
        inline void Read(TArchive& _rCodec)
        {
            bool Check = false;
            Base::ID ID;

            _rCodec >> Check;
            if (Check)
            {
                _rCodec >> ID;
                m_pParent = Dt::CEntityManager::GetInstance().GetEntityByID(ID);
            }

            _rCodec >> Check;
            if (Check)
            {
                _rCodec >> ID;
                m_pFirstChild = Dt::CEntityManager::GetInstance().GetEntityByID(ID);
            }

            _rCodec >> Check;
            if (Check)
            {
                _rCodec >> ID;
                m_pSibling = Dt::CEntityManager::GetInstance().GetEntityByID(ID);
            }
        }

        template <class TArchive>
        inline void Write(TArchive& _rCodec)
        {
            bool Check = false;

            Check = m_pParent != nullptr;
            _rCodec << Check;
            if (Check) _rCodec << m_pParent->GetID();

            Check = m_pFirstChild != nullptr;
            _rCodec << Check;
            if (Check) _rCodec << m_pFirstChild->GetID();

            Check = m_pSibling != nullptr;
            _rCodec << Check;
            if (Check) _rCodec << m_pSibling->GetID();
        }

    private:

        Base::U64 m_TimeStamp;              //< Time stamp to detect if entity already updated in this frame
        CEntity*  m_pParent;                //< Parent entity in tree
        CEntity*  m_pFirstChild;            //< First child on next level
        CEntity*  m_pSibling;               //< Next entity in the same level
    };
} // namespace Dt
