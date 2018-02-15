
#pragma once

#include "base/base_type_info.h"

#pragma warning(disable : 4505)

namespace Dt
{
    class CEntity;
} // namespace Dt

namespace Dt
{
    class IComponent
    {
    public:

        virtual const Base::ID GetTypeID() const = 0;

        virtual ~IComponent() {};
    };
} // namespace Dt

namespace Dt
{
    template<class T>
    class CComponent : public IComponent
    {
    public:

        enum EDirtyFlags
        {
            DirtyCreate  = 0x01,
            DirtyInfo    = 0x02,
            DirtyDestroy = 0x04,
        };

    public:

        static const Base::ID STATIC_TYPE_ID;

    public:

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

#pragma warning(default : 4201) 

    public:

        CComponent();
        ~CComponent();

        const Base::ID GetID() const;
        const Base::ID GetTypeID() const override;

        const Dt::CEntity* GetHostEntity() const;

        void SetActive(bool _Flag);
        bool IsActive() const;

        void SetDirtyFlags(unsigned int _Flags);
        unsigned int GetDirtyFlags() const;

    private:

        Base::ID           m_ID;
        SFlags             m_Flags;
        const Dt::CEntity* m_pHostEntity;

    private:

        friend class CComponentManager;
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
    }

    // -----------------------------------------------------------------------------

    template<class T>
    CComponent<T>::~CComponent()
    {

    }

    // -----------------------------------------------------------------------------

    template<class T>
    const Base::ID CComponent<T>::GetID() const
    {
        return m_ID;
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
} // namespace Dt

#pragma warning(default : 4505)