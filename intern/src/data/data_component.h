
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

        virtual void SetDirtyFlags(unsigned int _Flags) = 0;
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
                    unsigned int m_Padding    : 24;
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

        void SetDirtyFlags(unsigned int _Flags) override;
        unsigned int GetDirtyFlags() const;

        void SetLinkedEntity(const Dt::CEntity* _pEntity);
        const Dt::CEntity* GetLinkedEntity() const;

    private:

        Base::ID           m_ID;
        SFlags             m_Flags;
        const Dt::CEntity* m_pEntity;

    private:

        friend class CComponentManager;
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
    void CComponent<T>::SetDirtyFlags(unsigned int _Flags)
    {
        m_Flags.m_Key = _Flags;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    unsigned int CComponent<T>::GetDirtyFlags() const
    {
        return m_Flags.m_Key;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    void CComponent<T>::SetLinkedEntity(const Dt::CEntity* _pEntity)
    {
        m_pEntity = _pEntity;
    }

    // -----------------------------------------------------------------------------

    template<class T>
    const Dt::CEntity* CComponent<T>::GetLinkedEntity() const
    {
        return m_pEntity;
    }
} // namespace Dt

#pragma warning(default : 4505)