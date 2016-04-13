
#pragma once

#include "base/base_defines.h"
#include "base/base_typedef.h"

#include <assert.h>
#include <chrono>

namespace CORE
{
    class CTimer;
} // namespace CORE

namespace CORE
{
    class CClockBase
    {
    public:
        
        inline CClockBase();
        
    public:
        
        inline void AddTimer(CTimer& _rTimer);
        inline void RemoveTimer(CTimer& _rTimer);
        
    protected:
        
        inline void NotifyTimer(double _DurationOfFrame);
        
    private:
        
        CTimer* m_pFirstTimer;
        CTimer* m_pLastTimer;
    };
} // namespace CORE

namespace CORE
{
    template <Size TMaxNumberOfDurations = 8>
    class CClock : public CClockBase
    {
    public:
        
        typedef CClockBase                    CBase;
        typedef CClock<TMaxNumberOfDurations> CThis;

    public:

        static const Size s_MaxNumberOfDurations = TMaxNumberOfDurations;

    public:

        inline CClock(double _DefaultDurationOfFrame = 0.030);

    public:

        inline void Reset();

        inline void OnFrame();

        inline double GetTime() const;
        inline double GetDurationOfFrame() const;
        inline double GetNumberOfFrame() const;
        inline float GetFrameRate() const;
        
    private:
        
        typedef std::chrono::high_resolution_clock::time_point CTimeStamp;

    private:

        static const double s_MaxPeak;

    private:

        double         m_LastTick;
        double         m_DefaultDurationOfFrame;
        double         m_PredictedDurationOfFrame;
        double         m_CurrentTime;
        Size           m_NumberOfFrames;
        Size           m_IndexOfHistory;
        double         m_History[s_MaxNumberOfDurations];
        CTimeStamp     m_StartTimestamp;

    private:

        inline double CalcDurationOfLastFrame();
        inline double CalcDurationOfNextFrame() const;
        inline void AddDurationToHistory(double _DurationOfFrame);

    private:

        CClock(const CThis&);

        CThis& operator = (const CThis&);
    };
} // namespace CORE

namespace CORE
{
    typedef CClock<8> CDefaultClock;
    typedef CClock<1> CPerformanceClock;
} // namespace CORE

#include "base/base_timer.h"

namespace CORE
{
    inline CClockBase::CClockBase()
    : m_pFirstTimer(0)
    , m_pLastTimer (0)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    inline void CClockBase::AddTimer(CTimer& _rTimer)
    {
        _rTimer.m_pNext     = 0;
        _rTimer.m_pPrevious = m_pLastTimer;
        
        if (m_pFirstTimer == 0)
        {
            m_pFirstTimer = &_rTimer;
        }
        else
        {
            m_pLastTimer->m_pNext = &_rTimer;
        }
        
        m_pLastTimer = &_rTimer;
    }
    
    // -----------------------------------------------------------------------------
    
    inline void CClockBase::RemoveTimer(CTimer& _rTimer)
    {
        if (_rTimer.m_pNext != 0)
        {
            _rTimer.m_pNext->m_pPrevious = _rTimer.m_pPrevious;
        }
        else
        {
            m_pLastTimer = _rTimer.m_pPrevious;
        }
        
        if (_rTimer.m_pPrevious != 0)
        {
            _rTimer.m_pPrevious->m_pNext = _rTimer.m_pNext;
        }
        else
        {
            m_pFirstTimer = _rTimer.m_pNext;
        }
    }
    
    // -----------------------------------------------------------------------------
    
    inline void CClockBase::NotifyTimer(double _DurationOfFrame)
    {
        CTimer* pCurrenTimer;
        
        pCurrenTimer = m_pFirstTimer;
        
        while (pCurrenTimer != 0)
        {
            pCurrenTimer->Notify(_DurationOfFrame);
            
            pCurrenTimer = pCurrenTimer->m_pNext;
        }
    }
} // namespace CORE

namespace CORE
{
    template <Size TMaxNumberOfDurations>
    const double CClock<TMaxNumberOfDurations>::s_MaxPeak = 0.200;
} // namespace CORE

namespace CORE
{
    template <Size TMaxNumberOfDurations>
    inline CClock<TMaxNumberOfDurations>::CClock(double _DefaultDurationOfFrame)
        : m_LastTick                (0.0)
        , m_DefaultDurationOfFrame  (_DefaultDurationOfFrame)
        , m_PredictedDurationOfFrame(0.0)
        , m_CurrentTime             (0.0)
        , m_NumberOfFrames          (0)
        , m_IndexOfHistory          (0)
        , m_StartTimestamp          (std::chrono::high_resolution_clock::now())
    {
        Reset();
    }

    // -----------------------------------------------------------------------------

    template <Size TMaxNumberOfDurations>
    inline void CClock<TMaxNumberOfDurations>::OnFrame()
    {
        double DurationOfLastFrame;

        // -----------------------------------------------------------------------------
        // Add the duration of the last frame to the history.
        // -----------------------------------------------------------------------------
        DurationOfLastFrame = CalcDurationOfLastFrame();

        AddDurationToHistory(DurationOfLastFrame);

        // -----------------------------------------------------------------------------
        // Predict the expected duration of the current frame, which also depends on the
        // previously added duration of the last frame. 
        // -----------------------------------------------------------------------------
        m_PredictedDurationOfFrame = CalcDurationOfNextFrame();

        // -----------------------------------------------------------------------------
        // Forward the current time.
        // -----------------------------------------------------------------------------
        m_CurrentTime += m_PredictedDurationOfFrame;

        ++ m_NumberOfFrames;

        // -----------------------------------------------------------------------------
        // Notify all timer about the frame step.
        // -----------------------------------------------------------------------------
        NotifyTimer(m_PredictedDurationOfFrame);
    }

    // -----------------------------------------------------------------------------

    template <Size TMaxNumberOfDurations>
    inline double CClock<TMaxNumberOfDurations>::GetTime() const
    {
        return m_CurrentTime;
    }

    // -----------------------------------------------------------------------------

    template <Size TMaxNumberOfDurations>
    inline double CClock<TMaxNumberOfDurations>::GetDurationOfFrame() const
    {
        return m_PredictedDurationOfFrame;
    }

    // -----------------------------------------------------------------------------

    template <Size TMaxNumberOfDurations>
    inline double CClock<TMaxNumberOfDurations>::GetNumberOfFrame() const
    {
        return m_NumberOfFrames;
    }

    // -----------------------------------------------------------------------------

    template <Size TMaxNumberOfDurations>
    inline float CClock<TMaxNumberOfDurations>::GetFrameRate() const
    {
        return 1.0f / static_cast<float>(m_PredictedDurationOfFrame);
    }

    // -----------------------------------------------------------------------------

    template <Size TMaxNumberOfDurations>
    inline double CClock<TMaxNumberOfDurations>::CalcDurationOfLastFrame()
    {
        double CurrentTick;
        double DurationOfFrame;

        std::chrono::high_resolution_clock::time_point CurrentStamp = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double> CurrentTimeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(CurrentStamp - m_StartTimestamp);
        
        CurrentTick = CurrentTimeSpan.count();

        DurationOfFrame = CurrentTick - m_LastTick;

        if (DurationOfFrame > s_MaxPeak && s_MaxNumberOfDurations > 1)
        {
            DurationOfFrame = m_History[(m_IndexOfHistory - 1) & (s_MaxNumberOfDurations - 1)];
        }

        m_LastTick = CurrentTick;

        return DurationOfFrame;
    }

    // -----------------------------------------------------------------------------

    template <Size TMaxNumberOfDurations>
    inline double CClock<TMaxNumberOfDurations>::CalcDurationOfNextFrame() const
    {
        Size   IndexOfHistory;
        double DurationOfFrames;

        IndexOfHistory = (m_IndexOfHistory + 1) & (s_MaxNumberOfDurations - 1);

        DurationOfFrames = m_History[m_IndexOfHistory];

        while (IndexOfHistory != m_IndexOfHistory)
        {
            DurationOfFrames += m_History[IndexOfHistory];

            ++ IndexOfHistory; IndexOfHistory &= s_MaxNumberOfDurations - 1;
        }

        return DurationOfFrames / s_MaxNumberOfDurations;
    }

    // -----------------------------------------------------------------------------

    template <Size TMaxNumberOfDurations>
    inline void CClock<TMaxNumberOfDurations>::AddDurationToHistory(double _DurationOfFrame)
    {
        m_History[m_IndexOfHistory ++] = _DurationOfFrame;

        m_IndexOfHistory &= s_MaxNumberOfDurations - 1;
    }

    // -----------------------------------------------------------------------------

    template <Size TMaxNumberOfDurations>
    inline void CClock<TMaxNumberOfDurations>::Reset()
    {
        for (m_IndexOfHistory = 0; m_IndexOfHistory < s_MaxNumberOfDurations; ++ m_IndexOfHistory)
        {
            m_History[m_IndexOfHistory] = m_DefaultDurationOfFrame;
        }

        m_StartTimestamp           = std::chrono::high_resolution_clock::now();
        m_PredictedDurationOfFrame = m_DefaultDurationOfFrame;
        m_CurrentTime              = 0.0;
        m_NumberOfFrames           = 0;
        m_IndexOfHistory           = 0;
    }
} // namespace CORE