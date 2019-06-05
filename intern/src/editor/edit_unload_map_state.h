
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
        const std::string& GetFilename() const;

        void PreventSaving(bool _Flag);
        
    private:
        
        CUnloadMapState();
        ~CUnloadMapState();

    private:

        void InternOnEnter() override;
        void InternOnLeave() override;
        CState::EStateType InternOnRun() override;

    private:

        std::string m_Filename;
        bool m_PreventSaving;
    };
} // namespace Edit