
#pragma once

#include "editor/edit_state.h"

namespace Edit
{
    class CUnloadMapState : public CState
    {
        
    public:
        
        static CUnloadMapState& GetInstance();

    public:

        void SaveToFile(const std::string& _rFilename);
        
    private:
        
        CUnloadMapState();
        ~CUnloadMapState();

    private:

        CState::EStateType InternOnEnter() override;
        CState::EStateType InternOnLeave() override;
        CState::EStateType InternOnRun() override;

    private:

        std::string m_Filename;
    };
} // namespace Edit