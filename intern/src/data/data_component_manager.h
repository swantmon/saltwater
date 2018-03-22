
#pragma once

#include "base/base_uncopyable.h"
#include "base/base_defines.h"
#include "base/base_singleton.h"
#include "base/base_typedef.h"

#include "data/data_component.h"

#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace Dt
{
    class CComponentManager : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CComponentManager)

    public:

        typedef std::function<void(Dt::IComponent* _pComponent)> CComponentDelegate;

    public:

        template<class T>
        T* Allocate();

        void Deallocate(Dt::IComponent* _pObject);

        void Deallocate(Base::ID _ID);

        template<class T>
        T* GetComponent(Base::ID _ID);

        template<class T>
        const std::vector<Dt::IComponent*>& GetComponents();

        template<class T>
        void MarkComponentAsDirty(T* _pComponent, unsigned int _DirtyFlags);

        void RegisterDirtyComponentHandler(CComponentDelegate _NewDelegate);

    private:

        typedef std::vector<std::unique_ptr<Dt::IComponent>>     CComponents;
        typedef std::map<Base::ID, Dt::IComponent*>              CComponentsByID;
        typedef std::map<Base::ID, std::vector<Dt::IComponent*>> CComponentsByType;
        typedef std::vector<CComponentDelegate>                  CComponentDelegates;

    private:

        CComponents         m_Components; 
        CComponentsByID     m_ComponentByID;
        CComponentsByType   m_ComponentsByType;
        CComponentDelegates m_ComponentDelegates;
        Base::ID            m_CurrentID;

    private:

        CComponentManager();
        ~CComponentManager();
    };
} // namespace Dt

#define BASE_DIRTY_COMPONENT_METHOD(_Method) std::bind(_Method, this, std::placeholders::_1)

namespace Dt
{
    template<class T>
    T* CComponentManager::Allocate()
    {
        // -----------------------------------------------------------------------------
        // Allocate new component
        // -----------------------------------------------------------------------------
        m_Components.emplace_back(std::unique_ptr<T>(new T()));

        T* pComponent = static_cast<T*>(m_Components.back().get());

        pComponent->m_ID = m_CurrentID++;

        // -----------------------------------------------------------------------------
        // Save component to organizer
        // -----------------------------------------------------------------------------
        m_ComponentByID[pComponent->m_ID] = pComponent;

        m_ComponentsByType[Base::CTypeInfo::GetTypeID<T>()].emplace_back(pComponent);

        return pComponent;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    T* CComponentManager::GetComponent(Base::ID _ID)
    {
        if (m_ComponentByID.find(_ID) == m_ComponentByID.end()) return nullptr;

        return static_cast<T*>(m_ComponentByID[_ID]);
    }

    // -----------------------------------------------------------------------------

    template<class T>
    const std::vector<Dt::IComponent*>& CComponentManager::GetComponents()
    {
        return m_ComponentsByType[Base::CTypeInfo::GetTypeID<T>()];
    }

    // -----------------------------------------------------------------------------

    template<class T>
    void CComponentManager::MarkComponentAsDirty(T* _pComponent, unsigned int _DirtyFlags)
    {
        _pComponent->m_DirtyFlags = _DirtyFlags;

        for (auto Delegate : m_ComponentDelegates)
        {
            Delegate(_pComponent);
        }
    }
} // namespace Dt