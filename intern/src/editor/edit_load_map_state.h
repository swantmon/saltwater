
#pragma once

#include "editor/edit_state.h"

namespace Edit
{
    class CLoadMapState : public CState
    {
        
    public:
        
        static CLoadMapState& GetInstance();
        
    public:
        
        void LoadFromFile(const std::string& _rFilename);
        const std::string& GetFilename() const;
        
    private:
        
        CLoadMapState();
        ~CLoadMapState();
        
    private:
        
        void InternOnEnter() override;
        void InternOnLeave() override;
        CState::EStateType InternOnRun() override;

    private:

        std::string m_Filename;
    };
} // namespace Edit