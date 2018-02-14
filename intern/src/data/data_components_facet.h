
#pragma once

#include "base/base_typedef.h"

namespace Dt
{
    class CComponentsFacet
    {
	public:

		typedef std::vector<Dt::IComponent*> CComponentVector;

    public:

        template<class T>
        void AddComponent(T* _pComponent);

        template<class T>
        T* GetComponent();

        template<class T>
        const T* GetComponent() const;

        template<class T>
        bool HasComponent() const;

    public:

        CComponentsFacet();
        ~CComponentsFacet();

    private:

        CComponentVector m_Components; //< Components added to this entity
    };
} // namespace Dt

namespace Dt
{
    template<class T>
    void CComponentsFacet::AddComponent(T* _pComponent)
    {
        assert(_pComponent != nullptr);

        _pComponent->SetLinkedEntity(this);

        m_Components.push_back(_pComponent);
    }

    // -----------------------------------------------------------------------------

    template<class T>
    T* CComponentsFacet::GetComponent()
    {
        for (auto Component : m_Components)
        {
            if (Component->GetTypeID() == Base::CTypeInfo::GetTypeID<T>())
            {
                return static_cast<T*>(Component);
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    const T* CComponentsFacet::GetComponent() const
    {
        for (auto Component : m_Components)
        {
            if (Component->GetTypeID() == Base::CTypeInfo::GetTypeID<T>())
            {
                return static_cast<T*>(Component);
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    bool CComponentsFacet::HasComponent() const
    {
        return GetComponent<T>() != nullptr;
    }
} // namespace Dt
