
#pragma once

#include "base/base_singleton.h"
#include "base/base_typedef.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_component.h"

#include <map>
#include <memory>

namespace Gfx
{
    class CComponentManager : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CComponentManager)

    public:

        template<class T>
        T* Allocate(Base::ID _ID);

        template<class T>
        T* GetComponent(Base::ID _ID);

        void Clear();

    private:

        typedef std::map<Base::ID, std::shared_ptr<Gfx::IComponent>> CComponentByID;

    private:

        CComponentByID m_Components;

    private:

        CComponentManager();
        ~CComponentManager();
    };
} // namespace Gfx

namespace Gfx
{
    template<class T>
    T* CComponentManager::Allocate(Base::ID _ID)
    {
        m_Components.insert(std::make_pair(_ID, std::shared_ptr<T>(new T())));

        return static_cast<T*>(m_Components.at(_ID).get());
    }

    // -----------------------------------------------------------------------------

    template<class T>
    T* CComponentManager::GetComponent(Base::ID _ID)
    {
        if (m_Components.find(_ID) == m_Components.end()) return nullptr;

        return static_cast<T*>(m_Components.at(_ID).get());
    }
} // namespace Gfx