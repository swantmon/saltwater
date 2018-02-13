
#pragma once

#include "base/base_memory.h"
#include "base/base_singleton.h"
#include "base/base_typedef.h"
#include "base/base_uncopyable.h"

#include "data/data_component.h"

#include <functional>
#include <vector>

namespace Dt
{
    class CComponentManager : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CComponentManager)

    public:

        typedef std::function<void(Base::ID _TypeID, Dt::IComponent* _pComponent) > CComponentDelegate;

    public:

        template<class T>
        T& Allocate();

        void MarkComponentAsDirty(Dt::IComponent& _rComponent, unsigned int _DirtyFlags);

        void RegisterDirtyComponentHandler(CComponentDelegate _NewDelegate);

    private:

        typedef std::vector<Dt::IComponent*>    CComponents;
        typedef std::vector<CComponentDelegate> CComponentDelegates;

    private:

        CComponents m_Components;
        CComponentDelegates m_ComponentDelegates;
        Base::ID m_CurrentID;

    private:

        CComponentManager();
        ~CComponentManager();
    };
} // namespace Dt

#define DATA_DIRTY_COMPONENT_METHOD(_Method) std::bind(_Method, this, std::placeholders::_1, std::placeholders::_2)

namespace Dt
{
    template<class T>
    T& CComponentManager::Allocate()
    {
        T* pComponent = Base::CMemory::NewObject<T>();

        pComponent->m_ID = m_CurrentID++;

        m_Components.push_back(pComponent);

        return *pComponent;
    }
} // namespace Dt