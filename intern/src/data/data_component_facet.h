
#pragma once

#include "engine/engine_config.h"

#include "base/base_typedef.h"

#include "data/data_component.h"

#include <vector>

namespace Dt
{
    class ENGINE_API CComponentFacet
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
        if (_pComponent == nullptr || _pComponent->GetHostEntity() == nullptr) return;

        assert(_pComponent->GetHostEntity() != nullptr);

        m_Components.push_back(_pComponent);
    }

    // -----------------------------------------------------------------------------

    template<class T>
    void CComponentFacet::RemoveComponent(T* _pComponent)
    {
        if (_pComponent == nullptr || _pComponent->GetHostEntity() != nullptr) return;

        m_Components.erase(std::find(m_Components.begin(), m_Components.end(), _pComponent));
    }
} // namespace Dt
