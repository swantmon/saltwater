
#pragma once

#include "base/base_defines.h"

#include <array>
#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <vector>

namespace PAT
{
    template<class ... Args>
    class CDelegate
    {
    public:

        using FunctionType = std::function<void(Args...)>;
        using HandleType = std::shared_ptr<FunctionType>;

        void Notify(Args... _Args)
        {
            Clean();

            for (auto& rDelegate : m_Container)
            {
                assert(!rDelegate.expired());
                auto Delegate = *(rDelegate.lock());
                Delegate(_Args...);
            }
        }

        void Clean()
        {
            m_Container.erase(std::remove_if(m_Container.begin(), m_Container.end(), [](const auto& _Handle) { return _Handle.expired(); }), m_Container.end());
        }

        auto Register(FunctionType _Function)
        {
            auto Ptr = std::make_shared<FunctionType>(_Function);
            m_Container.push_back(Ptr);
            return Ptr;
        }

    private:

        using ContainerType = std::vector<std::weak_ptr<FunctionType>>;

        ContainerType m_Container;
    };

    // -----------------------------------------------------------------------------

    template<int _TAmount, class ... Args>
    class CDelegates
    {
    public:

        using FunctionType = std::function<void(Args...)>;
        using HandleType = std::shared_ptr<FunctionType>;

        void Notify(int _Container, Args... _Args)
        {
            Clean(_Container);

            for (auto& rDelegate : m_Container[_Container])
            {
                assert(!rDelegate.expired());
                auto Delegate = *(rDelegate.lock());
                Delegate(_Args...);
            }
        }

        void Clean(int _Container)
        {
            m_Container[_Container].erase(std::remove_if(m_Container[_Container].begin(), m_Container[_Container].end(), [](const auto& _Handle) { return _Handle.expired(); }), m_Container[_Container].end());
        }

        auto Register(int _Container, FunctionType _Function)
        {
            auto Ptr = std::make_shared<FunctionType>(_Function);
            m_Container[_Container].push_back(Ptr);
            return Ptr;
        }

    private:

        using ContainerType = std::array<std::vector<std::weak_ptr<FunctionType>>, _TAmount>;

        ContainerType m_Container;
    };
} // namespace PAT