
#pragma once

#include "editor/edit_state.h"

namespace Edit
{
    class CLoadMapState : public CState
    {
        
    public:
        
        static CLoadMapState& GetInstance();
        
    public:
        
        void SetMapfile(const char* _pFilename);
        const char* GetMapfile() const;
        
    private:
        
        const char* m_pMapfile;
        
    private:
        
        CLoadMapState();
        ~CLoadMapState();
        
    private:
        
        virtual CState::EStateType InternOnEnter();
        virtual CState::EStateType InternOnLeave();
        virtual CState::EStateType InternOnRun();
        
    };
} // namespace Edit