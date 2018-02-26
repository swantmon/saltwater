
#pragma once

#include "base/base_typedef.h"

namespace CON
{
    class CComponentManager;
}

namespace CON
{
    class IComponent
    {
    public:

        const Base::ID GetID() const
        {
            return m_ID;
        }

        virtual const Base::ID GetTypeID() const = 0;

        virtual ~IComponent() {};

    
     private:

        Base::ID m_ID;

    private:

        friend class CComponentManager;
    };
} // namespace CON