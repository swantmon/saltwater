
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

        inline void RegisterClass(size_t _Hash, CBaseFactory* _pClassObject);

        inline CBaseFactory* GetClass(size_t _Hash, void* _pBaseClass);

        inline bool HasClass(size_t _Hash);

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
    void CGUIFactory::RegisterClass(CBaseFactory* _pClassObject)
    {
        RegisterClass(CalculateHash<T>(), _pClassObject);
    }

    // -----------------------------------------------------------------------------

    inline void CGUIFactory::RegisterClass(size_t _Hash, CBaseFactory* _pClassObject)
    {
        if (!HasClass(_Hash))
        {
            m_Factory.insert(std::make_pair(_Hash, _pClassObject));
        }
    }

    // -----------------------------------------------------------------------------

    inline CBaseFactory* CGUIFactory::GetClass(size_t _Hash, void* _pBaseClass)
    {
        if (HasClass(_Hash))
        {
            return static_cast<CBaseFactory*>(m_Factory.find(_Hash)->second->Create(_pBaseClass));
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    inline bool CGUIFactory::HasClass(size_t _Hash)
    {
        return m_Factory.find(_Hash) != m_Factory.end();
    }

    // -----------------------------------------------------------------------------

    template<class T>
    size_t CGUIFactory::CalculateHash()
    {
        T* pSample = 0;

        return CalculateHash(pSample);
    }

    // -----------------------------------------------------------------------------

    template<class T>
    size_t CGUIFactory::CalculateHash(T _Class)
    {
        return typeid(_Class).hash_code();
    }
} // namespace Edit