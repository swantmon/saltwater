
#pragma once

#include "base/base_singleton.h"
#include "base/base_typedef.h"
#include "base/base_uncopyable.h"

#include "data/data_component.h"

#include <functional>
#include <memory>
#include <vector>

namespace Dt
{
    class CComponentManager : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CComponentManager)

    public:

        typedef std::function<void(Dt::IComponent* _pComponent) > CComponentDelegate;

    public:

        template<class T>
        T* Allocate();

        template<class T>
        void MarkComponentAsDirty(T* _pComponent, unsigned int _DirtyFlags);

        void RegisterDirtyComponentHandler(CComponentDelegate _NewDelegate);

    private:

        typedef std::vector<std::shared_ptr<Dt::IComponent>> CComponents;
        typedef std::vector<CComponentDelegate>              CComponentDelegates;

    private:

        CComponents m_Components;
        CComponentDelegates m_ComponentDelegates;
        Base::ID m_CurrentID;

    private:

        CComponentManager();
        ~CComponentManager();
    };
} // namespace Dt

#define DATA_DIRTY_COMPONENT_METHOD(_Method) std::bind(_Method, this, std::placeholders::_1)

namespace Dt
{
    template<class T>
    T* CComponentManager::Allocate()
    {
        m_Components.emplace_back(std::shared_ptr<T>(new T()));

        T* pComponent = static_cast<T*>(m_Components.back().get());

        pComponent->m_ID = m_CurrentID++;

        return pComponent;
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
} // namespace Dt