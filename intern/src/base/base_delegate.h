
#pragma once

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

        static void Notify(Args... _Args)
        {
            Clean();

            for (auto& rDelegate : s_Container)
            {
                assert(!rDelegate.expired());
                auto Delegate = *(rDelegate.lock());
                Delegate(_Args...);
            }
        }

        static void Clean()
        {
            s_Container.erase(std::remove_if(s_Container.begin(), s_Container.end(), [](const auto& _Handle) { return _Handle.expired(); }), s_Container.end());
        }

        static auto Register(FunctionType _Function)
        {
            auto Ptr = std::make_shared<FunctionType>(_Function);
            s_Container.push_back(Ptr);
            return Ptr;
        }

    private:

        using ContainerType = std::vector<std::weak_ptr<FunctionType>>;

        static ContainerType s_Container;
    };

    template<class ... Args>
    typename CDelegate<Args...>::ContainerType CDelegate<Args...>::s_Container;

    // -----------------------------------------------------------------------------

    template<int _TAmount, class ... Args>
    class CDelegates
    {
    public:

        using FunctionType = std::function<void(Args...)>;
        using HandleType = std::shared_ptr<FunctionType>;

        static void Notify(int _Container, Args... _Args)
        {
            Clean(_Container);

            for (auto& rDelegate : s_Container[_Container])
            {
                assert(!rDelegate.expired());
                auto Delegate = *(rDelegate.lock());
                Delegate(_Args...);
            }
        }

        static void Clean(int _Container)
        {
            s_Container[_Container].erase(std::remove_if(s_Container[_Container].begin(), s_Container[_Container].end(), [](const auto& _Handle) { return _Handle.expired(); }), s_Container[_Container].end());
        }

        static auto Register(int _Container, FunctionType _Function)
        {
            auto Ptr = std::make_shared<FunctionType>(_Function);
            s_Container[_Container].push_back(Ptr);
            return Ptr;
        }

    private:

        using ContainerType = std::array<std::vector<std::weak_ptr<FunctionType>>, _TAmount>;

        static ContainerType s_Container;
    };

    template<int _TAmount, class ... Args>
    typename CDelegates<_TAmount, Args...>::ContainerType CDelegates<_TAmount, Args...>::s_Container;
} // namespace PAT