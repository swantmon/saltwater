
#pragma once

#include "base/base_type_info.h"
#include "base/base_component.h"
#include "base/base_component_manager.h"

#pragma warning(push)
#pragma warning(disable : 4505)

namespace Dt
{
    class CComponentManager
    {
    public:
        static Base::CComponentManager& GetInstance()
        {
            static Base::CComponentManager s_Singleton;

            return s_Singleton;
        }
    };
} // namespace Dt 

namespace Dt
{
    class CEntity;
} // namespace Dt

namespace Dt
{
    template<class T>
    class CComponent : public Base::IComponent
    {
    public:

        enum EDirtyFlags
        {
            DirtyCreate  = 0x01,
            DirtyInfo    = 0x02,
            DirtyDestroy = 0x04,
        };

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
                    unsigned int m_DirtyFlags : 8;        //< Dirty flags if something happens
                    unsigned int m_IsActive   : 1;        //< Either the component is active or not
                    unsigned int m_Padding    : 23;
                };

                unsigned int m_Key;
            };
        };

#pragma warning(pop) 

    public:

        CComponent();
        ~CComponent();

        const Base::ID GetTypeID() const override;

        const Dt::CEntity* GetHostEntity() const;

        void SetActive(bool _Flag);
        bool IsActive() const;
        bool IsActiveAndUsable() const;

        void SetDirtyFlags(unsigned int _Flags);
        unsigned int GetDirtyFlags() const;

        void SetFacet(unsigned int _Category, void* _pFacet);
        void* GetFacet(unsigned int _Category);
        const void* GetFacet(unsigned int _Category) const;

    private:

        SFlags             m_Flags;
        const Dt::CEntity* m_pHostEntity;
        void*              m_pFacets[NumberOfFacets];

    private:

        friend class CEntity;
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

        for (int i = 0; i < NumberOfFacets; ++i)
        {
            m_pFacets[i] = 0;
        }
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
    const Dt::CEntity* CComponent<T>::GetHostEntity() const
    {
        return m_pHostEntity;
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
        assert(m_pHostEntity);

        return m_Flags.m_IsActive == true && m_pHostEntity->IsActive() == true;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    void CComponent<T>::SetDirtyFlags(unsigned int _Flags)
    {
        m_Flags.m_DirtyFlags = _Flags;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    unsigned int CComponent<T>::GetDirtyFlags() const
    {
        return m_Flags.m_DirtyFlags;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    void CComponent<T>::SetFacet(unsigned int _Category, void* _pFacet)
    {
        m_pFacets[_Category] = _pFacet;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    void* CComponent<T>::GetFacet(unsigned int _Category)
    {
        return m_pFacets[_Category];
    }

    // -----------------------------------------------------------------------------

    template<class T>
    const void* CComponent<T>::GetFacet(unsigned int _Category) const
    {
        return m_pFacets[_Category];
    }
} // namespace Dt

#pragma warning(pop)