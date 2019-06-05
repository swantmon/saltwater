
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

        inline CState();
        inline CState(EStateType _State);
                
    public:
        
        inline void OnEnter();
        inline void OnLeave();
        inline CState::EStateType OnRun();

    public:

        inline void SetNextState(EStateType _State);

    protected:

        EStateType m_NextState = UndefinedStateType;

    private:
        
        virtual void InternOnEnter() = 0;
        virtual void InternOnLeave() = 0;
        virtual CState::EStateType InternOnRun() = 0;
    };
} // namespace Edit

namespace Edit
{
    CState::CState()
        : m_NextState(UndefinedStateType)
    {

    }

    // -----------------------------------------------------------------------------

    CState::CState(EStateType _State)
        : m_NextState(_State)
    {

    }

    // -----------------------------------------------------------------------------

    void CState::OnEnter()
    {
        InternOnEnter();
    }
    
    // -----------------------------------------------------------------------------
    
    void CState::OnLeave()
    {
        InternOnLeave();
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