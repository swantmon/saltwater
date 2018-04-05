
#pragma once

#include "engine/engine_config.h"

#include "base/base_type_info.h"

#include "data/data_entity.h"

#include <array>

#pragma warning(push)
#pragma warning(disable : 4505)

namespace Dt
{
    class CComponentManager;
} // namespace Dt

namespace Dt
{
    class IComponent
    {
    public:

        enum EDirtyFlags
        {
            DirtyCreate = 0x01,
            DirtyInfo = 0x02,
            DirtyDestroy = 0x04,
        };

    public:

        IComponent()
            : m_ID(0)
            , m_pHostEntity(0)
            , m_DirtyFlags(0)
        {};

        const Base::ID GetID() const
        {
            return m_ID;
        }

        const Dt::CEntity* GetHostEntity() const
        {
            return m_pHostEntity;
        }

        unsigned int GetDirtyFlags() const
        {
            return m_DirtyFlags;
        }

    public:

        virtual const Base::ID GetTypeID() const = 0;

        virtual ~IComponent() {};

    protected:

        Base::ID m_ID;
        Dt::CEntity* m_pHostEntity;
        unsigned int m_DirtyFlags;

    private:

        friend class CEntity;
        friend class CComponentManager;
    };
} // namespace Dt

namespace Dt
{
    template<class T>
    class CComponent : public Dt::IComponent
    {
    public:

        enum EFacets
        {
            Graphic,
            NumberOfFacets
        };

    public:

        static const Base::ID STATIC_TYPE_ID;

    public:

#pragma warning(push)
#pragma warning(disable : 4201)

        struct SFlags
        {
            union
            {
                struct
                {
                    unsigned int m_IsActive   : 1;        //< Either the component is active or not
                    unsigned int m_Padding    : 31;
                };

                unsigned int m_Key;
            };
        };

#pragma warning(pop) 

    public:

        CComponent();
        ~CComponent();

        const Base::ID GetTypeID() const override;

        void SetActive(bool _Flag);
        bool IsActive() const;
        bool IsActiveAndUsable() const;

        void SetFacet(unsigned int _Category, void* _pFacet);
        void* GetFacet(unsigned int _Category);
        const void* GetFacet(unsigned int _Category) const;

    private:

        typedef std::array<void*, NumberOfFacets> CFacets;

    private:

        SFlags  m_Flags;
        CFacets m_Facets;
    };
} // namespace Dt

namespace Dt
{
    template<class T>
    const Base::ID CComponent<T>::STATIC_TYPE_ID = Base::CTypeInfo::GetTypeID<T>();
} // namespace Dt

namespace Dt
{
    template<class T>
    CComponent<T>::CComponent()
    {
        m_Flags.m_Key = 0;

        m_Flags.m_IsActive = true;

        for (auto Facet : m_Facets) Facet = 0;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    CComponent<T>::~CComponent()
    {

    }

    // -----------------------------------------------------------------------------

    template<class T>
    const Base::ID CComponent<T>::GetTypeID() const
    {
        return STATIC_TYPE_ID;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    void CComponent<T>::SetActive(bool _Flag)
    {
        m_Flags.m_IsActive = _Flag;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    bool CComponent<T>::IsActive() const
    {
        return m_Flags.m_IsActive == true;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    bool CComponent<T>::IsActiveAndUsable() const
    {
        return m_Flags.m_IsActive == true && m_pHostEntity != nullptr && m_pHostEntity->IsActive() == true;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    void CComponent<T>::SetFacet(unsigned int _Category, void* _pFacet)
    {
        m_Facets[_Category] = _pFacet;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    void* CComponent<T>::GetFacet(unsigned int _Category)
    {
        return m_Facets[_Category];
    }

    // -----------------------------------------------------------------------------

    template<class T>
    const void* CComponent<T>::GetFacet(unsigned int _Category) const
    {
        return m_Facets[_Category];
    }
} // namespace Dt

#pragma warning(pop)