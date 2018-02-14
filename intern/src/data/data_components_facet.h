
#pragma once

#include "base/base_typedef.h"

#include "data/data_component.h"

#include <vector>

namespace Dt
{
    class CComponentsFacet
    {
    public:

        template<class T>
        T* GetComponent();

        template<class T>
        const T* GetComponent() const;

        template<class T>
        bool HasComponent() const;

    public:

        CComponentsFacet();
        ~CComponentsFacet();

    protected:

        template<class T>
        void AddComponent(T* _pComponent);

        template<class T>
        void RemoveComponent(T* _pComponent);

    private:

        typedef std::vector<Dt::IComponent*> CComponentVector;

    private:

        CComponentVector m_Components;

    private:

        friend class CEntity;
    };
} // namespace Dt

namespace Dt
{
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

    // -----------------------------------------------------------------------------

    template<class T>
    void CComponentsFacet::AddComponent(T* _pComponent)
    {
        assert(_pComponent != nullptr);
        assert(_pComponent->GetLinkedEntity() != nullptr);

        m_Components.push_back(_pComponent);
    }

    // -----------------------------------------------------------------------------

    template<class T>
    void CComponentsFacet::RemoveComponent(T* _pComponent)
    {
        assert(_pComponent != nullptr);
        assert(_pComponent->GetLinkedEntity() == nullptr);

        m_Components.erase(_pComponent);
    }
} // namespace Dt
