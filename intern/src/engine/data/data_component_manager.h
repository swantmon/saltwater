
#pragma once

#include "engine/engine_config.h"

#include "base/base_defines.h"
#include "base/base_delegate.h"
#include "base/base_serialize_text_reader.h"
#include "base/base_serialize_text_writer.h"
#include "base/base_singleton.h"
#include "base/base_typedef.h"
#include "base/base_uncopyable.h"

#include "engine/data/data_component.h"

#include <functional>
#include <map>
#include <memory>
#include <vector>

#define REGISTER_COMPONENT_SER(_Name)                                                                               \
    struct BASE_CONCAT(SRegisterSerialize, _Name)                                                               \
    {                                                                                                           \
        BASE_CONCAT(SRegisterSerialize, _Name)()                                                                \
        {                                                                                                       \
            static _Name BASE_CONCAT(s_SRegisterSerialize, _Name);                                              \
            Dt::CComponentManager::GetInstance().Register<_Name>(&BASE_CONCAT(s_SRegisterSerialize, _Name));    \
        }                                                                                                       \
    } BASE_CONCAT(g_SRegisterSerialize, _Name);

namespace Dt
{
    class ENGINE_API CComponentManager : Base::CUncopyable
    {
    public:             

        static CComponentManager& GetInstance();

    public:

        using CComponentDelegate = Base::CDelegate<Dt::IComponent*>;

    public:

        template<class T>
        T* Allocate();

        void Deallocate(Dt::IComponent* _pObject);

        void Deallocate(Base::ID _ID);

        template<class T>
        T* GetComponent(Base::ID _ID);

        template<class T>
        const std::vector<Dt::IComponent*>& GetComponents();

        void MarkComponentAsDirty(IComponent& _rComponent, unsigned int _DirtyFlags);

        CComponentDelegate::HandleType RegisterDirtyComponentHandler(CComponentDelegate::FunctionType _NewDelegate); 
        
        template<class T> 
        void Register(IComponent* _pBase)
        {
            auto ID = Base::CTypeInfo::GetTypeID<T>();

            if (m_Factory.find(ID) == m_Factory.end()) m_Factory.insert(CFactoryMapPair(ID, _pBase));
        }

    public:

        void Read(Base::CTextReader& _rCodec);
        void Write(Base::CTextWriter& _rCodec);

    private:

        using CComponents = std::vector<std::unique_ptr<Dt::IComponent>>;
        using CComponentsByID = std::map<Base::ID, Dt::IComponent*>;
        using CComponentsByType = std::map<Base::ID, std::vector<Dt::IComponent*>>;

        using CFactoryMap     = std::map<Base::ID, IComponent*>;
        using CFactoryMapPair = std::pair<Base::ID, IComponent*>;

    private:

        CFactoryMap m_Factory;

        CComponents       m_Components;
        CComponentsByID   m_ComponentByID;
        CComponentsByType m_ComponentsByType;
        Base::ID          m_CurrentID;

        CComponentDelegate m_ComponentDelegate;

    private:

        IComponent* InternAllocate(Base::ID _TypeID);

    private:

        CComponentManager();
        ~CComponentManager();
    };
} // namespace Dt

namespace Dt
{
    template<class T>
    T* CComponentManager::Allocate()
    {
        // -----------------------------------------------------------------------------
        // Allocate new component
        // -----------------------------------------------------------------------------
        m_Components.emplace_back(std::unique_ptr<T>(new T()));

        auto* pComponent = static_cast<T*>(m_Components.back().get());

        pComponent->m_ID = m_CurrentID++;

        // -----------------------------------------------------------------------------
        // Save component to organizer
        // -----------------------------------------------------------------------------
        m_ComponentByID[pComponent->m_ID] = pComponent;

        m_ComponentsByType[pComponent->GetTypeID()].emplace_back(pComponent);

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
} // namespace Dt