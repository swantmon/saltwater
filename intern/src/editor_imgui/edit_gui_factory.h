
#pragma once

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
        void RegisterClass(CBaseFactory* _pClassObject);
        
        template<class T>
        CBaseFactory* GetClass(void* _pBaseClass);
        
        template<class T>
        bool HasClass();
        
    private:
        
        std::map<size_t, CBaseFactory*> m_Factory;
        
    private:
        
        template<class T>
        size_t CalculateHash();
    };
} // namespace Edit

namespace Edit
{
    template<class T>
    void CGUIFactory::RegisterClass(CBaseFactory* _pClassObject)
    {
        if (!HasClass<T>())
        {
            m_Factory.insert(std::make_pair(CalculateHash<T>(), _pClassObject));
        }
    }

    // -----------------------------------------------------------------------------

    template<class T>
    bool CGUIFactory::HasClass()
    {
        return m_Factory.find(CalculateHash<T>()) != m_Factory.end();
    }

    // -----------------------------------------------------------------------------

    template<class T>
    CBaseFactory* CGUIFactory::GetClass(void* _pBaseClass)
    {
        if (HasClass<T>())
        {
            return static_cast<CBaseFactory*>(m_Factory.find(CalculateHash<T>())->second->Create(_pBaseClass));
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    size_t CGUIFactory::CalculateHash()
    {
        return typeid(T).hash_code();
    }
} // namespace Edit