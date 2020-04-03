
#pragma once

#include "engine/engine_config.h"

#include "base/base_crc.h"
#include "base/base_defines.h"
#include "base/base_delegate.h"
#include "base/base_serialize_text_reader.h"
#include "base/base_serialize_text_writer.h"
#include "base/base_singleton.h"
#include "base/base_typedef.h"
#include "base/base_uncopyable.h"

#include "engine/data/data_component.h"

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

// -----------------------------------------------------------------------------
// This define is only necessary on Android because th hash-code can be
// different between different shared libraries. The name stays the same.
// TODO: Find out why this behavior happens and how to solve it more
// 	     efficiently.
// -----------------------------------------------------------------------------
#ifdef PLATFORM_ANDROID
#define COMPONENT_MANAGER_MAPTYPE_BY_NAME
#endif // PLATFORM_ANDROID

#define REGISTER_COMPONENT_SER_NAME(_Name, _Class)                                                                    \
    struct BASE_CONCAT(SRegisterSerialize, _Class)                                                                    \
    {                                                                                                                 \
        BASE_CONCAT(SRegisterSerialize, _Class)()                                                                     \
        {                                                                                                             \
            static _Class BASE_CONCAT(s_SRegisterSerialize, _Class);                                                  \
            Dt::CComponentManager::GetInstance().Register<_Class>(_Name, &BASE_CONCAT(s_SRegisterSerialize, _Class)); \
        }                                                                                                             \
    } BASE_CONCAT(g_SRegisterSerialize, _Class);

#define REGISTER_COMPONENT_SER(_Class) REGISTER_COMPONENT_SER_NAME(#_Class, _Class)

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

		void Clear();

	public:

		template<class T>
		void Register(const std::string& _rName, IComponent* _pBase);

        void Read(Base::CTextReader& _rCodec);
        void Write(Base::CTextWriter& _rCodec);

	private:

#ifdef COMPONENT_MANAGER_MAPTYPE_BY_NAME
		using BComponentTypeKey = std::string;
#else
		using BComponentTypeKey = Base::CTypeInfo::BInfo;
#endif // COMPONENT_MANAGER_MAPTYPE_BY_NAME

    private:

        using CComponents       = std::vector<std::unique_ptr<IComponent>>;
        using CComponentsByID   = std::unordered_map<Base::ID, IComponent*>;
        using CComponentsByType = std::unordered_map<BComponentTypeKey, std::vector<IComponent*>>;

        using CFactoryMap     = std::unordered_map<Base::BHash, IComponent*>;
        using CFactoryMapPair = std::pair<Base::BHash, IComponent*>;

        using CFactoryHashMap     = std::unordered_map<Base::CTypeInfo::BInfo, Base::BHash>;
        using CFactoryHashMapPair = std::pair<Base::CTypeInfo::BInfo, Base::BHash>;

    private:

        CFactoryMap m_Factory;
        CFactoryHashMap m_FactoryHash;

        CComponents       m_Components;
        CComponentsByID   m_ComponentByID;
        CComponentsByType m_ComponentsByType;
        Base::ID          m_CurrentID;

        CComponentDelegate m_ComponentDelegate;

    private:

        IComponent* InternAllocateByHash(Base::BHash _TypeID);

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

#ifdef COMPONENT_MANAGER_MAPTYPE_BY_NAME
		m_ComponentsByType[pComponent->GetTypeInfo().name()].emplace_back(pComponent);
#else
		m_ComponentsByType[pComponent->GetTypeInfo()].emplace_back(pComponent);
#endif // COMPONENT_MANAGER_MAPTYPE_BY_NAME

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
#ifdef COMPONENT_MANAGER_MAPTYPE_BY_NAME
		return m_ComponentsByType[Base::CTypeInfo::Get<T>().name()];
#else
        return m_ComponentsByType[Base::CTypeInfo::Get<T>()];
#endif
    }

	// -----------------------------------------------------------------------------

	template<class T>
	void CComponentManager::Register(const std::string& _rName, IComponent* _pBase)
	{
		auto TypeInfo = Base::CTypeInfo::Get<T>();

		auto Hash = Base::CRC32(_rName.c_str(), static_cast<unsigned int>(_rName.length()));

		assert(m_Factory.find(Hash) == m_Factory.end());

		assert(m_FactoryHash.find(TypeInfo) == m_FactoryHash.end());

		m_Factory.insert(CFactoryMapPair(Hash, _pBase));

		m_FactoryHash.insert(CFactoryHashMapPair(TypeInfo, Hash));
	}
} // namespace Dt