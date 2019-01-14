#pragma once

#include "base/base_clock.h"
#include "base/base_defines.h"
#include "base/base_memory.h"
#include "base/base_serialize_access.h"
#include "base/base_serialize_archive.h"
#include "base/base_timer.h"


using namespace Base;

namespace SER
{
    class CRecorder
    {
    public:

        inline  CRecorder();
        inline ~CRecorder();

    public:

        inline void SetSpeed(double _Speed);

        inline double GetSpeed() const;

        inline void Pause();

        inline void Resume();

        inline bool IsPaused() const;

    public:

        inline double GetTime();

        inline double GetDurationOfFrame();

        inline void RestartTimer();

    public:

        inline void Update();

    private:

        Base::CPerformanceClock m_Clock;
        Base::CTimer m_Timer;
    };
} // namespace SER

namespace SER
{
    inline CRecorder::CRecorder()
        : m_Clock()
        , m_Timer(m_Clock)
    {

    }

    // -----------------------------------------------------------------------------

    inline CRecorder::~CRecorder()
    {

    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::SetSpeed(double _Speed)
    {
        m_Timer.SetSpeed(_Speed);
    }

    // -----------------------------------------------------------------------------

    inline double CRecorder::GetSpeed() const
    {
        return m_Timer.GetSpeed();
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::Pause()
    {
        m_Timer.Pause();
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::Resume()
    {
        m_Timer.Resume();
    }

    // -----------------------------------------------------------------------------

    inline bool CRecorder::IsPaused() const
    {
        m_Timer.IsPaused();
    }

    // -----------------------------------------------------------------------------

    inline double CRecorder::GetTime()
    {
        return m_Timer.GetTime();
    }

    // -----------------------------------------------------------------------------

    inline double CRecorder::GetDurationOfFrame()
    {
        return m_Timer.GetDurationOfFrame();
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::RestartTimer()
    {
        m_Timer.Reset();
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::Update()
    {
        m_Clock.OnFrame();
    }
} // namespace SER