
#pragma once

#include "base/base_memory.h"
#include "base/base_type_info.h"
#include "base/base_uncopyable.h"

#include "editor/edit_asset_helper.h"

#include <array>
#include <map>

#define REGISTER_GUI(Name, Child, HEADER)                                                                       \
class BASE_CONCAT(Name, Factory) : public Edit::IGUIFactory                                                     \
{                                                                                                               \
public:                                                                                                         \
    BASE_CONCAT(Name, Factory)() { }                                                                            \
    IGUIFactory* Create() { return new BASE_CONCAT(Name, Factory)(); }                                          \
    void SetChild(void* _pChild) { m_pChild = (Name*)(_pChild); }                                               \
    bool OnGUI() { return m_pChild ? m_pChild->OnGUI() : false; }                                               \
    const char* GetHeader() { return HEADER; }                  					                            \
    void OnDropAsset(const Edit::CAsset& _rAsset) { m_pChild ? m_pChild->OnDropAsset(_rAsset) : 0; }            \
private:                                                                                                        \
    Name* m_pChild;                                                                                             \
};                                                                                                              \
struct BASE_CONCAT(SRegFactory, Name)                                                                           \
{                                                                                                               \
    BASE_CONCAT(SRegFactory, Name)()                                                                            \
    {                                                                                                           \
        static BASE_CONCAT(Name, Factory) BASE_CONCAT(s_Factory, Name);                                         \
        Edit::CGUIFactory::GetInstance().Register<Child>(&BASE_CONCAT(s_Factory, Name));                        \
    }                                                                                                           \
} const BASE_CONCAT(g_SRegFactory, Name);

namespace Edit
{
    class IGUIFactory
    {
    public:
        virtual IGUIFactory* Create() = 0;

        virtual void SetChild(void* _pChild) = 0;

        virtual bool OnGUI() = 0;

        virtual const char* GetHeader() = 0;

        virtual void OnDropAsset(const Edit::CAsset& _rAsset) = 0;
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

        inline IGUIFactory* Get(Base::CTypeInfo::BInfo _TypeInfo, void* _pObject);

        template<class T>
        bool Has();

        inline bool Has(Base::CTypeInfo::BInfo _TypeInfo);

    private:

        struct SFactoryElement
        {
            IGUIFactory* m_pFactory;
            std::map<size_t, IGUIFactory*> m_Instances;
        };

    private:

        using CFactoryMap = std::map<Base::CTypeInfo::BInfo, SFactoryElement>;
        using CFactoryMapPair = std::pair<Base::CTypeInfo::BInfo, SFactoryElement>;
        
    private:
        
        CFactoryMap m_Factory;

    private:

        inline ~CGUIFactory();
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

    inline CGUIFactory::~CGUIFactory()
    {
        for (auto& [rFactoryKey, rFactory] : m_Factory)
        {
            for (auto& [rInstanceKey, rInstance] : rFactory.m_Instances)
            {
                Base::CMemory::DeleteObject(rInstance);
            }

            rFactory.m_Instances.clear();
        }

        m_Factory.clear();
    }

    // -----------------------------------------------------------------------------

    template<class T>
    void CGUIFactory::Register(IGUIFactory* _pClassObject)
    {
        if (!Has<T>())
        {
            SFactoryElement NewElement;

            NewElement.m_pFactory = _pClassObject;

            m_Factory.insert(CFactoryMapPair(Base::CTypeInfo::Get<T>(), NewElement));
        }
    }

    // -----------------------------------------------------------------------------

    template<class T>
    IGUIFactory* CGUIFactory::Get(void* _pBaseClass)
    {
        if (Has<T>())
        {
            SFactoryElement& rElement = m_Factory.find(Base::CTypeInfo::Get<T>())->second;

            if (rElement.m_Instances.find((size_t)_pBaseClass) == rElement.m_Instances.end())
            {
                rElement.m_Instances[(size_t)_pBaseClass] = rElement.m_pFactory->Create();

                rElement.m_Instances[(size_t)_pBaseClass]->SetChild(_pBaseClass);
            }

            auto CurrentInstance = rElement.m_Instances[(size_t)_pBaseClass];

            return CurrentInstance;
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    inline IGUIFactory* CGUIFactory::Get(Base::CTypeInfo::BInfo _TypeInfo, void* _pBaseClass)
    {
        if (Has(_TypeInfo))
        {
            SFactoryElement& rElement = m_Factory.find(_TypeInfo)->second;

            if (rElement.m_Instances.find((size_t)_pBaseClass) == rElement.m_Instances.end())
            {
                rElement.m_Instances[(size_t)_pBaseClass] = rElement.m_pFactory->Create();

                rElement.m_Instances[(size_t)_pBaseClass]->SetChild(_pBaseClass);
            }

            auto CurrentInstance = rElement.m_Instances[(size_t)_pBaseClass];

            return CurrentInstance;
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    bool CGUIFactory::Has()
    {
        return m_Factory.find(Base::CTypeInfo::Get<T>()) != m_Factory.end();
    }

    // -----------------------------------------------------------------------------

    inline bool CGUIFactory::Has(Base::CTypeInfo::BInfo _TypeInfo)
    {
        return m_Factory.find(_TypeInfo) != m_Factory.end();
    }
} // namespace Edit