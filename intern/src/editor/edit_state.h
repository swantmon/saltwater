
#pragma once

namespace Edit
{
    class CState
    {
        
    public:
        
        enum EStateType
        {
            Start,
            Intro,
            LoadMap,
            Edit,
            Play,
            UnloadMap,
            Exit,
            NumberOfStateTypes,
            UndefinedStateType = -1
        };
                
    public:
        
        inline CState::EStateType OnEnter();
        inline CState::EStateType OnLeave();
        inline CState::EStateType OnRun();

    public:

        inline void SetNextState(EStateType _State);

    protected:

        EStateType m_NextState = UndefinedStateType;

    private:
        
        virtual CState::EStateType InternOnEnter() = 0;
        virtual CState::EStateType InternOnLeave() = 0;
        virtual CState::EStateType InternOnRun()   = 0;
    };
} // namespace Edit

namespace Edit
{
    CState::EStateType CState::OnEnter()
    {
        return InternOnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CState::OnLeave()
    {
        return InternOnLeave();
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CState::OnRun()
    {
        return InternOnRun();
    }

    // -----------------------------------------------------------------------------

    inline void CState::SetNextState(EStateType _State)
    {
        m_NextState = _State;
    }
} // namespace Edit