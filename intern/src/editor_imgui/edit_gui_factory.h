
#pragma once

#include "base/base_type_info.h"

#include <map>

namespace Edit
{
    class IGUIFactory
    {
    public:
        virtual void* Create(void* _pChild) = 0;

        virtual void OnGUI() = 0;
    };

    // -----------------------------------------------------------------------------

    class CGUIFactory
    {
    public:
        
        inline static CGUIFactory& GetInstance();
        
    public:
        
        template<class T>
        void Register(IGUIFactory* _pObject);

        template<class T>
        IGUIFactory* Get(void* _pObject);

        inline IGUIFactory* Get(size_t _Hash, void* _pObject);

        template<class T>
        bool Has();

        inline bool Has(size_t _Hash);

        template<class T>
        size_t CalculateHash();

        template<class T>
        size_t CalculateHash(T _Object);
        
    private:
        
        std::map<size_t, IGUIFactory*> m_Factory;
    };
} // namespace Edit

namespace Edit
{
    inline CGUIFactory& CGUIFactory::GetInstance()
    {
        static CGUIFactory s_Instance;

        return s_Instance;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    void CGUIFactory::Register(IGUIFactory* _pClassObject)
    {
        if (!Has<T>())
        {
            m_Factory.insert(std::make_pair(CalculateHash<T>(), _pClassObject));
        }
    }

    // -----------------------------------------------------------------------------

    template<class T>
    IGUIFactory* CGUIFactory::Get(void* _pBaseClass)
    {
        if (Has<T>())
        {
            return static_cast<IGUIFactory*>(m_Factory.find(CalculateHash<T>())->second->Create(_pBaseClass));
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    inline IGUIFactory* CGUIFactory::Get(size_t _Hash, void* _pBaseClass)
    {
        if (Has(_Hash))
        {
            return static_cast<IGUIFactory*>(m_Factory.find(_Hash)->second->Create(_pBaseClass));
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