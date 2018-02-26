
#pragma once

#include "base/base_component.h"
#include "base/base_defines.h"
#include "base/base_typedef.h"

#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace CON
{
    class CComponentManager
    {
    public:

        CComponentManager();
        ~CComponentManager();

    public:

        typedef std::function<void(CON::IComponent* _pComponent)> CComponentDelegate;

    public:

        template<class T>
        T* Allocate();

        template<class T>
        T* Allocate(Base::ID _ID);

        template<class T>
        T* GetComponent(Base::ID _ID);

        template<class T>
        const std::vector<CON::IComponent*>& GetComponents();

        template<class T>
        void MarkComponentAsDirty(T* _pComponent, unsigned int _DirtyFlags);

        void RegisterDirtyComponentHandler(CComponentDelegate _NewDelegate);

    private:

        typedef std::vector<std::unique_ptr<CON::IComponent>>     CComponents;
        typedef std::map<Base::ID, CON::IComponent*>              CComponentsByID;
        typedef std::map<Base::ID, std::vector<CON::IComponent*>> CComponentsByType;
        typedef std::vector<CComponentDelegate>                   CComponentDelegates;

    private:

        CComponents         m_Components; 
        CComponentsByID     m_ComponentByID;
        CComponentsByType   m_ComponentsByType;
        CComponentDelegates m_ComponentDelegates;
        Base::ID            m_CurrentID;
    };
} // namespace CON

#define BASE_DIRTY_COMPONENT_METHOD(_Method) std::bind(_Method, this, std::placeholders::_1)

namespace CON
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
    T* CComponentManager::Allocate(Base::ID _ID)
    {
        if (m_ComponentByID.find(_ID) != m_ComponentByID.end())
        {
            return static_cast<T*>(m_ComponentByID.at(_ID));
        }

        // -----------------------------------------------------------------------------
        // Allocate new component
        // -----------------------------------------------------------------------------
        m_Components.emplace_back(std::unique_ptr<T>(new T()));

        T* pComponent = static_cast<T*>(m_Components.back().get());

        pComponent->m_ID = _ID;

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
        return static_cast<T*>(m_ComponentByID[_ID]);
    }

    // -----------------------------------------------------------------------------

    template<class T>
    const std::vector<CON::IComponent*>& CComponentManager::GetComponents()
    {
        return m_ComponentsByType[Base::CTypeInfo::GetTypeID<T>()];
    }

    // -----------------------------------------------------------------------------

    template<class T>
    void CComponentManager::MarkComponentAsDirty(T* _pComponent, unsigned int _DirtyFlags)
    {
        _pComponent->SetDirtyFlags(_DirtyFlags);

        for (auto Delegate : m_ComponentDelegates)
        {
            Delegate(_pComponent);
        }
    }
} // namespace CON