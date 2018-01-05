
#pragma once

namespace App
{
    class CState
    {

    public:

        enum EStateType
        {
            Init,
            Start,
            Intro,
            LoadMap,
            MainMenu,
            Play,
            UnloadMap,
            Exit,
            NumberOfStateTypes,
            UndefinedStateType = -1
        };

    public:

        inline void OnEnter();
        inline void OnLeave();
        inline void OnRun();

    private:

        virtual void InternOnEnter() = 0;
        virtual void InternOnLeave() = 0;
        virtual void InternOnRun() = 0;

    };
} // namespace App

namespace App
{
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

    void CState::OnRun()
    {
        InternOnRun();
    }
} // namespace App