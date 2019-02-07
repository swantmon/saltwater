
#pragma once

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
        using ContainerType = std::vector<std::weak_ptr<FunctionType>>;

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

        static HandleType Register(FunctionType _Function)
        {
            auto Ptr = std::make_shared<FunctionType>(_Function);
            s_Container.push_back(Ptr);
            return Ptr;
        }

        static ContainerType s_Container;
    };

    template<class ... Args>
    typename CDelegate<Args...>::ContainerType CDelegate<Args...>::s_Container;
} // namespace PAT