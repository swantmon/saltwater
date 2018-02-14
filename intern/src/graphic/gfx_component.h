
#pragma once

#include "base/base_type_info.h"

#pragma warning(disable : 4505)

namespace Gfx
{
    class IComponent
    {
    public:

        virtual const Base::ID GetTypeID() const = 0;

        virtual ~IComponent() {};
    };
} // namespace Gfx

namespace Gfx
{
    template<class T>
    class CComponent : public IComponent
    {
    public:

        static const Base::ID STATIC_TYPE_ID;

    public:

        const Base::ID GetTypeID() const override;

    private:

        friend class CComponentManager;
    };
} // namespace Gfx

namespace Gfx
{
    template<class T>
    const Base::ID CComponent<T>::STATIC_TYPE_ID = Base::CTypeInfo::GetTypeID<T>();
} // namespace Gfx

namespace Gfx
{
    template<class T>
    const Base::ID CComponent<T>::GetTypeID() const
    {
        return STATIC_TYPE_ID;
    }
} // namespace Gfx

#pragma warning(default : 4505)