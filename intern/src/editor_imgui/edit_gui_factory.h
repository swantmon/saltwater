
#pragma once

#include "base/base_type_info.h"

#include "editor_imgui/edit_base_factory.h"

#include <map>

namespace Edit
{
    class CGUIFactory
    {
    public:
        
        static CGUIFactory& GetInstance()
        {
            static CGUIFactory s_Instance;

            return s_Instance;
        }
        
    public:
        
        template<class T>
        void Register(CBaseFactory* _pClassObject);

        template<class T>
        CBaseFactory* Get(void* _pBaseClass);

        inline CBaseFactory* Get(size_t _Hash, void* _pBaseClass);

        template<class T>
        bool Has();

        inline bool Has(size_t _Hash);

        template<class T>
        size_t CalculateHash();

        template<class T>
        size_t CalculateHash(T _Class);
        
    private:
        
        std::map<size_t, CBaseFactory*> m_Factory;
    };
} // namespace Edit

namespace Edit
{
    template<class T>
    void CGUIFactory::Register(CBaseFactory* _pClassObject)
    {
        if (!Has<T>())
        {
            m_Factory.insert(std::make_pair(CalculateHash<T>(), _pClassObject));
        }
    }

    // -----------------------------------------------------------------------------

    template<class T>
    CBaseFactory* CGUIFactory::Get(void* _pBaseClass)
    {
        if (Has<T>())
        {
            return static_cast<CBaseFactory*>(m_Factory.find(CalculateHash<T>())->second->Create(_pBaseClass));
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    inline CBaseFactory* CGUIFactory::Get(size_t _Hash, void* _pBaseClass)
    {
        if (Has(_Hash))
        {
            return static_cast<CBaseFactory*>(m_Factory.find(_Hash)->second->Create(_pBaseClass));
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    bool CGUIFactory::Has()
    {
        return m_Factory.find(CalculateHash<T>()) != m_Factory.end();
    }

    // -----------------------------------------------------------------------------

    inline bool CGUIFactory::Has(size_t _Hash)
    {
        return m_Factory.find(_Hash) != m_Factory.end();
    }

    // -----------------------------------------------------------------------------

    template<class T>
    size_t CGUIFactory::CalculateHash()
    {
        return Base::CTypeInfo::GetTypeID<T>();
    }

    // -----------------------------------------------------------------------------

    template<class T>
    size_t CGUIFactory::CalculateHash(T _Class)
    {
        return Base::CTypeInfo::GetTypeID<T>();
    }
} // namespace Edit