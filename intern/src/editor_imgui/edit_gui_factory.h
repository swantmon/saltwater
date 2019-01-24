
#pragma once

#include "base/base_type_info.h"
#include "base/base_uncopyable.h"

#include <array>
#include <map>

namespace Edit
{
    class IGUIFactory
    {
    public:
        virtual IGUIFactory* Create() = 0;

        virtual void SetChild(void* _pChild) = 0;

        virtual void OnGUI() = 0;

        virtual const char* GetHeader() = 0;
    };

    // -----------------------------------------------------------------------------

    class CGUIFactory : Base::CUncopyable
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

        static const int s_MaxNumberOfInstances = 12;

    private:

        struct SFactoryElement
        {
            IGUIFactory* m_pFactory;
            int m_Index;
            std::array<IGUIFactory*, s_MaxNumberOfInstances> m_Instances;
        };

    private:

        typedef std::map<size_t, SFactoryElement> CFactoryMap;
        typedef std::pair<size_t, SFactoryElement> CFactoryMapPair;
        
    private:
        
        CFactoryMap m_Factory;
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
            SFactoryElement NewElement;

            NewElement.m_pFactory = _pClassObject;
            NewElement.m_Index = 0;

            for (auto& rInstance : NewElement.m_Instances)
            {
                rInstance = _pClassObject->Create();
            }

            m_Factory.insert(CFactoryMapPair(CalculateHash<T>(), NewElement));
        }
    }

    // -----------------------------------------------------------------------------

    template<class T>
    IGUIFactory* CGUIFactory::Get(void* _pBaseClass)
    {
        if (Has<T>())
        {
            SFactoryElement& rElement = m_Factory.find(CalculateHash<T>())->second;

            auto CurrentInstance = rElement.m_Instances[rElement.m_Index];

            rElement.m_Index = ++rElement.m_Index % s_MaxNumberOfInstances;

            CurrentInstance->SetChild(_pBaseClass);

            return CurrentInstance;
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    inline IGUIFactory* CGUIFactory::Get(size_t _Hash, void* _pBaseClass)
    {
        if (Has(_Hash))
        {
            SFactoryElement& rElement = m_Factory.find(_Hash)->second;

            auto CurrentInstance = rElement.m_Instances[rElement.m_Index];

            rElement.m_Index = ++rElement.m_Index % s_MaxNumberOfInstances;

            CurrentInstance->SetChild(_pBaseClass);

            return CurrentInstance;
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
        BASE_UNUSED(_Class);

        return Base::CTypeInfo::GetTypeID<T>();
    }
} // namespace Edit