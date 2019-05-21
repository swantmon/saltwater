
#pragma once

#include "engine/engine_config.h"

#include "base/base_typedef.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_manager.h"

#include <vector>

namespace Dt
{
    class ENGINE_API CComponentFacet
    {
    public:

        using CComponentVector = std::vector<Dt::IComponent*>;

    public:

        template<class T>
        T* GetComponent();

        template<class T>
        const T* GetComponent() const;

        template<class T>
        bool HasComponent() const;

        CComponentVector& GetComponents();

        const CComponentVector& GetComponents() const;

    public:

        CComponentFacet();
        ~CComponentFacet();

    public:

        template <class TArchive>
        inline void Read(TArchive& _rCodec)
        {
            unsigned int NumberOfComponents;

            _rCodec >> NumberOfComponents;

            m_Components.resize(NumberOfComponents);

            for (int i = 0; i < NumberOfComponents; ++i)
            {
                Base::ID ID;

                _rCodec >> ID;

                m_Components[i] = Dt::CComponentManager::GetInstance().GetComponent<Dt::IComponent>(ID);
            }
        }

        template <class TArchive>
        inline void Write(TArchive& _rCodec)
        {
            unsigned int NumberOfComponents = 0;

            NumberOfComponents = m_Components.size();

            _rCodec << NumberOfComponents;

            for (auto Component : m_Components)
            {
                assert(Component != nullptr);
                
                _rCodec << Component->GetID();
            }
        }

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
