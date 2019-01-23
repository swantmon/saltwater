
#pragma once

namespace Edit
{
    class CBaseFactory
    {
    public:
        virtual void* Create(void* _pChild) = 0;
        
        virtual void OnGUI() = 0;
    };
} // namespace Edit
