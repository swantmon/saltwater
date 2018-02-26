
#pragma once

#include "base/base_component.h"
#include "base/base_component_manager.h"
#include "base/base_type_info.h"

#pragma warning(push)
#pragma warning(disable : 4505)

namespace Gfx
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
} // namespace Gfx 

namespace Gfx
{
    template<class T>
    class CComponent : public Base::IComponent
    {
    public:

        static const Base::ID STATIC_TYPE_ID;

    public:

        const Base::ID GetTypeID() const override;

    private:

        friend class Base::CComponentManager;
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

#pragma warning(pop)