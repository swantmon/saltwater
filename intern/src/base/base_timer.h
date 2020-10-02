
#pragma once

#include "base/base_defines.h"

namespace CORE
{
    class CClockBase;
} // namespace CORE

namespace CORE
{
    class CTimer
    {
        friend class CClockBase;

        public:

            inline CTimer();
            inline CTimer(CClockBase& _rClock);
            inline ~CTimer();

        public:

            inline void SetClock(CClockBase& _rClock);

        public:

            inline void SetTime(double _Time);

            inline double GetTime() const;
            inline double GetDurationOfFrame() const;

            inline void SetSpeed(double _TimeScale);
            inline double GetSpeed() const;

            inline void Pause();
            inline void Resume();

            inline bool IsPaused() const;

            inline void Reset();

        private:

            CClockBase* m_pClock;
            CTimer*     m_pNext;
            CTimer*     m_pPrevious;
            double      m_CurrentTime;
            double      m_PredictedDurationOfFrame;
            double      m_Speed;
            bool        m_IsPaused;

        private:

            inline void Notify(double _DurationOfFrame);
    };
} // namespace CORE

#include "base/base_clock.h"

namespace CORE
{
    inline CTimer::CTimer()
        : m_pClock                  (0)
        , m_pNext                   (0)
        , m_pPrevious               (0)
        , m_CurrentTime             (0.0)
        , m_PredictedDurationOfFrame(0.0)
        , m_Speed                   (1.0)
        , m_IsPaused                (false)
    {
    }

    // -----------------------------------------------------------------------------

    inline CTimer::CTimer(CClockBase& _rClock)
        : m_pClock                  (&_rClock)
        , m_pNext                   (0)
        , m_pPrevious               (0)
        , m_CurrentTime             (0.0)
        , m_PredictedDurationOfFrame(0.0)
        , m_Speed                   (1.0)
        , m_IsPaused                (false)
    {
        m_pClock->AddTimer(*this);
    }

    // -----------------------------------------------------------------------------

    inline CTimer::~CTimer()
    {
        if (m_pClock != 0)
        {
            m_pClock->RemoveTimer(*this);
        }
    }

    // -----------------------------------------------------------------------------

    inline void CTimer::SetClock(CClockBase& _rClock)
    {
        // -----------------------------------------------------------------------------
        // Check if the time is connected to another clock.
        // -----------------------------------------------------------------------------
        if (m_pClock != 0)
        {
            m_pClock->RemoveTimer(*this);
        }

        // -----------------------------------------------------------------------------
        // Memorize the new clock.
        // -----------------------------------------------------------------------------
        m_pClock = &_rClock; 

        // -----------------------------------------------------------------------------
        // Add the timer to the clock.
        // -----------------------------------------------------------------------------
        m_pClock->AddTimer(*this);
    }

    // -----------------------------------------------------------------------------

    inline void CTimer::SetTime(double _Time)
    {
        m_CurrentTime = _Time;
    }

    // -----------------------------------------------------------------------------

    inline double CTimer::GetTime() const
    {
        return m_CurrentTime;
    }

    // -----------------------------------------------------------------------------

    inline double CTimer::GetDurationOfFrame() const
    {
        return m_PredictedDurationOfFrame;
    }

    // -----------------------------------------------------------------------------

    inline void CTimer::SetSpeed(double _TimeScale)
    {
        m_Speed = _TimeScale;
    }

    // -----------------------------------------------------------------------------

    inline double CTimer::GetSpeed() const
    {
        return m_Speed;
    }

    // -----------------------------------------------------------------------------

    inline void CTimer::Pause()
    {
        m_IsPaused = true;
    }

    // -----------------------------------------------------------------------------

    inline void CTimer::Resume()
    {
        m_IsPaused = false;
    }

    // -----------------------------------------------------------------------------

    inline bool CTimer::IsPaused() const
    {
        return m_IsPaused;
    }

    // -----------------------------------------------------------------------------

    inline void CTimer::Reset()
    {
        m_CurrentTime              = 0.0;
        m_PredictedDurationOfFrame = 0.0;
        m_Speed                    = 1.0;
    }

    // -----------------------------------------------------------------------------

    inline void CTimer::Notify(double _DurationOfFrame)
    {
        if (!m_IsPaused)
        {
            m_PredictedDurationOfFrame = _DurationOfFrame * m_Speed;

            m_CurrentTime += m_PredictedDurationOfFrame;
        }
    }
} // namespace CORE