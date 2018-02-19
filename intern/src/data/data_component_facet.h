
#pragma once

#include "base/base_typedef.h"

#include "data/data_component.h"

#include <vector>

namespace Dt
{
    class CComponentFacet
    {
    public:

        typedef std::vector<Dt::IComponent*> CComponentVector;

    public:

        template<class T>
        T* GetComponent();

        template<class T>
        const T* GetComponent() const;

        template<class T>
        bool HasComponent() const;

        const CComponentVector& GetComponents() const;

    public:

        CComponentFacet();
        ~CComponentFacet();

    protected:

        template<class T>
        void AddComponent(T* _pComponent);

        template<class T>
        void RemoveComponent(T* _pComponent);

    private:

        CComponentVector m_Components;

    private:

        friend class CEntity;
    };
} // namespace Dt

namespace Dt
{
    template<class T>
    T* CComponentFacet::GetComponent()
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
    const T* CComponentFacet::GetComponent() const
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
    bool CComponentFacet::HasComponent() const
    {
        return GetComponent<T>() != nullptr;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    void CComponentFacet::AddComponent(T* _pComponent)
    {
        assert(_pComponent != nullptr);
        assert(_pComponent->GetHostEntity() != nullptr);

        m_Components.push_back(_pComponent);
    }

    // -----------------------------------------------------------------------------

    template<class T>
    void CComponentFacet::RemoveComponent(T* _pComponent)
    {
        assert(_pComponent != nullptr);
        assert(_pComponent->GetHostEntity() == nullptr);

        m_Components.erase(_pComponent);
    }
} // namespace Dt
