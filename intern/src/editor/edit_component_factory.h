
#pragma once

#include "base/base_memory.h"
#include "base/base_type_info.h"
#include "base/base_uncopyable.h"

#include "engine/data/data_entity.h"

#include <vector>
#include <set>

#define REGISTER_COMPONENT(Name)                                                                                \
class BASE_CONCAT(Name, ComponentFactory) : public Edit::IGUIComponentFactory                                   \
{                                                                                                               \
public:                                                                                                         \
    BASE_CONCAT(Name, ComponentFactory)() { m_pChild = new Name(); }                                            \
    ~BASE_CONCAT(Name, ComponentFactory)() { delete m_pChild; }                                                 \
    void OnNewComponent(Dt::CEntity::BID _ID) { m_pChild->OnNewComponent(_ID); }                                \
    const char* GetHeader() { return m_pChild->GetHeader(); };                                                  \
private:                                                                                                        \
    Name* m_pChild;                                                                                             \
};                                                                                                              \
struct BASE_CONCAT(SRegComponentFactory, Name)                                                                  \
{                                                                                                               \
    BASE_CONCAT(SRegComponentFactory, Name)()                                                                   \
    {                                                                                                           \
        static BASE_CONCAT(Name, ComponentFactory) BASE_CONCAT(s_ComponentFactory, Name);                       \
        Edit::CComponentFactory::GetInstance().Register<Name>(&BASE_CONCAT(s_ComponentFactory, Name));          \
    }                                                                                                           \
} const BASE_CONCAT(g_SRegComponentFactory, Name);

namespace Edit
{
    class IGUIComponentFactory
    {
    public:

        virtual void OnNewComponent(Dt::CEntity::BID _ID) = 0;

        virtual const char* GetHeader() = 0;
    };

    // -----------------------------------------------------------------------------

    class CComponentFactory : Base::CUncopyable
    {
    public:

        inline static CComponentFactory& GetInstance();

    public:

        typedef std::vector<IGUIComponentFactory*> CFactoryVector;

    public:

        template<class T>
        void Register(IGUIComponentFactory* _pObject);

        inline const CFactoryVector& GetComponents() const;

    private:

        std::set<size_t> m_Hashes;

    private:

        CFactoryVector m_Factory;

    private:

        inline ~CComponentFactory();
    };
} // namespace Edit

namespace Edit
{
    inline CComponentFactory& CComponentFactory::GetInstance()
    {
        static CComponentFactory s_Instance;

        return s_Instance;
    }

    // -----------------------------------------------------------------------------

    inline CComponentFactory::~CComponentFactory()
    {
        m_Factory.clear();
        m_Hashes.clear();
    }

    // -----------------------------------------------------------------------------

    template<class T>
    void CComponentFactory::Register(IGUIComponentFactory* _pClassObject)
    {
        auto Hash = Base::CTypeInfo::GetTypeID<T>();

        if (m_Hashes.find(Hash) == m_Hashes.end())
        {
            m_Factory.push_back(_pClassObject);
        }
    }

    // -----------------------------------------------------------------------------

    inline const CComponentFactory::CFactoryVector& CComponentFactory::GetComponents() const
    {
        return m_Factory;
    }
} // namespace Edit