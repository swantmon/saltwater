
#pragma once

#include "base/base_clock.h"
#include "base/base_timer.h"

namespace Core
{
    class CBaseRecorder
    {
    public:
        typedef CBaseRecorder CThis;

    public:

        CBaseRecorder();
        ~CBaseRecorder();

    public:

        inline double GetTime();

    public:

        inline void RestartTimer();

    public:

        inline void Update();

    private:

        Base::CPerformanceClock m_Clock;
        Base::CTimer m_Timer;
    };
} // namespace Core

namespace Core
{
    CBaseRecorder::CBaseRecorder()
        : m_Clock()
        , m_Timer(m_Clock)
    {
    }

    // -----------------------------------------------------------------------------

    CBaseRecorder::~CBaseRecorder()
    { }

    // -----------------------------------------------------------------------------

    inline double CBaseRecorder::GetTime()
    {
        return m_Timer.GetTime();
    }

    // -----------------------------------------------------------------------------

    inline void CBaseRecorder::RestartTimer()
    {
        m_Timer.Reset();
    }

    // -----------------------------------------------------------------------------

    inline void CBaseRecorder::Update()
    {
        m_Clock.OnFrame();
    }
} // namespace Core