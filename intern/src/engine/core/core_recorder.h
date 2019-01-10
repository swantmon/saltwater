#pragma once

#include "base/base_clock.h"
#include "base/base_memory.h"
#include "base/base_serialize_archive.h"
#include "base/base_serialize_std_vector.h"

namespace Core
{
    class CRecorder
    {
    public:

        enum EPlaybackMode
        {
            SINGLE,
            FPS,
            TIMECODE
        };

    public:

        CRecorder(EPlaybackMode _PlaybackMode = SINGLE);
        ~CRecorder();

    public:

        template<class TType>
        inline double GetData(TType& _rBytes);

        inline void SetData(void* _pBytes, size_t _NumberOfBytes);

        inline void Play();

        inline void Record();

        inline void Eject();

        inline void Pause();

        inline void Restart();

        inline void Step();

        inline bool IsEnd();

        inline void SetFPS(int _FramesPerSeconds);

        inline void SetPlaybackMode(EPlaybackMode _Mode);

        inline double GetTime();

    public:

        template <class TArchive>
        inline void Read(TArchive& _rCodec);

        template <class TArchive>
        inline void Write(TArchive& _rCodec);

    private:

        enum EMode
        {
            IDLE,
            PLAY,
            PAUSE,
            RECORD,
        };

    private:

        class CFrame
        {

        public:

            CFrame();
            ~CFrame();

            void SetIndex(size_t _Timecode);
            size_t GetIndex() const;

            void SetTimecode(double _Timecode);
            double GetTimecode() const;

            void Get(void*& _prBytes, size_t& _rNumberOfBytes);

            void Set(void* _pBytes, size_t _NumberOfBytes);

            void Clear();

            bool HasData();

        public:

            template <class TArchive>
            inline void Read(TArchive& _rCodec);

            template <class TArchive>
            inline void Write(TArchive& _rCodec);

        private:

            size_t m_Index;
            double m_Timecode;
            void* m_pBytes;
            size_t m_NumberOfBytes;
        };

    private:

        typedef std::vector<CFrame> CFrames;

    private:

        CFrames m_Frames;

        int m_FramesPerSeconds;
        double m_Frequenz;
        double m_TimeSinceLastStep;
        Base::CPerformanceClock m_Clock;
        EMode m_Mode;
        EPlaybackMode m_PlaybackMode;
        CFrames::iterator m_CurrentFrameIter;

    private:

        void AcquireNewFrame();
        void UpdateReel();
        void Clear();
        void CheckEOL();
    };
} // namespace Core

namespace Core
{
    CRecorder::CFrame::CFrame()
        : m_Index        (-1)
        , m_Timecode     (-1)
        , m_pBytes       (0)
        , m_NumberOfBytes(0)
    { }

    // -----------------------------------------------------------------------------

    CRecorder::CFrame::~CFrame()
    {
    }

    // -----------------------------------------------------------------------------

    void CRecorder::CFrame::SetIndex(size_t _Index)
    {
        m_Index = _Index;
    }

    // -----------------------------------------------------------------------------

    size_t CRecorder::CFrame::GetIndex() const
    {
        return m_Index;
    }

    // -----------------------------------------------------------------------------

    void CRecorder::CFrame::SetTimecode(double _Timecode)
    {
        m_Timecode = _Timecode;
    }

    // -----------------------------------------------------------------------------

    double CRecorder::CFrame::GetTimecode() const
    {
        return m_Timecode;
    }

    // -----------------------------------------------------------------------------

    void CRecorder::CFrame::Get(void*& _prBytes, size_t& _rNumberOfBytes)
    {
        _prBytes = m_pBytes;

        _rNumberOfBytes = m_NumberOfBytes;
    }

    // -----------------------------------------------------------------------------

    void CRecorder::CFrame::Set(void* _pBytes, size_t _NumberOfBytes)
    {
        Clear();

        m_NumberOfBytes = _NumberOfBytes;

        m_pBytes = Base::CMemory::Allocate(_NumberOfBytes);

        Base::CMemory::Copy(m_pBytes, _pBytes, _NumberOfBytes);
    }

    // -----------------------------------------------------------------------------

    void CRecorder::CFrame::Clear()
    {
        if (m_pBytes != 0) Base::CMemory::Free(m_pBytes);

        m_pBytes        = 0;
        m_NumberOfBytes = 0;
    }

    // -----------------------------------------------------------------------------

    bool CRecorder::CFrame::HasData()
    {
        return m_pBytes != nullptr && m_NumberOfBytes > 0;
    }

    // -----------------------------------------------------------------------------

    template <class TArchive>
    inline void CRecorder::CFrame::Read(TArchive& _rCodec)
    {
        _rCodec >> m_Index;
        _rCodec >> m_Timecode;
        _rCodec >> m_NumberOfBytes;

        m_pBytes = Base::CMemory::Allocate(m_NumberOfBytes);

        _rCodec.ReadBinary(m_pBytes, m_NumberOfBytes);
    }

    // -----------------------------------------------------------------------------

    template <class TArchive>
    inline void CRecorder::CFrame::Write(TArchive& _rCodec)
    {
        _rCodec << m_Index;
        _rCodec << m_Timecode;
        _rCodec << m_NumberOfBytes;

        _rCodec.WriteBinary(m_pBytes, m_NumberOfBytes);
    }
} // namespace Core

namespace Core
{
    CRecorder::CRecorder(EPlaybackMode _PlaybackMode)
        : m_FramesPerSeconds (-1)
        , m_Frequenz         (0.0)
        , m_TimeSinceLastStep(0.0)
        , m_Clock            ( )
        , m_Mode             (IDLE)
        , m_PlaybackMode     (_PlaybackMode)
        , m_CurrentFrameIter (m_Frames.begin())
    { }

    // -----------------------------------------------------------------------------

    CRecorder::~CRecorder()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    template<class TType>
    inline double CRecorder::GetData(TType& _rBytes)
    {
        if (m_Mode == IDLE) return 0.0;

        void* pBytes = 0;
        size_t NumberOfBytes = 0;

        m_CurrentFrameIter->Get(pBytes, NumberOfBytes);

        _rBytes = *static_cast<TType*>(pBytes);

        return m_CurrentFrameIter->GetTimecode();
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::SetData(void* _pBytes, size_t _NumberOfBytes)
    {
        if (m_Mode != RECORD) return;

        if (m_CurrentFrameIter == m_Frames.end())
        {
            AcquireNewFrame();

            m_CurrentFrameIter = std::prev(m_Frames.end());
        }

        m_CurrentFrameIter->SetTimecode(m_Clock.GetTime());

        m_CurrentFrameIter->Set(_pBytes, _NumberOfBytes);
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::Play()
    {
        switch (m_Mode)
        {
        case IDLE:
            Restart();
            break;
        }

        m_Mode = PLAY;
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::Record()
    {
        switch (m_Mode)
        {
        case IDLE:
            Restart();
            break;
        }

        m_Mode = RECORD;

        if (IsEnd()) UpdateReel();
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::Pause()
    {
        if (m_Mode == IDLE) return;

        m_Mode = PAUSE;
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::Eject()
    {
        m_Mode = IDLE;

        Restart();
        Clear();
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::Restart()
    {
        CheckEOL();

        m_TimeSinceLastStep = 0.0;
        m_CurrentFrameIter = m_Frames.begin();

        m_Clock.Reset();
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::Step()
    {
        m_Clock.OnFrame();

        switch (m_Mode)
        {
        default:
        case Core::CRecorder::IDLE:
        case Core::CRecorder::PAUSE:
            return;
            break;
        case Core::CRecorder::PLAY:
        case Core::CRecorder::RECORD:
            break;
        }

        switch (m_PlaybackMode)
        {
        default:
        case Core::CRecorder::SINGLE:
            UpdateReel();
            break;
        case Core::CRecorder::FPS:
            m_TimeSinceLastStep += m_Clock.GetDurationOfFrame();

            if (m_TimeSinceLastStep > m_Frequenz)
            {
                UpdateReel();

                m_TimeSinceLastStep = 0.0f;
            }
            break;
        case Core::CRecorder::TIMECODE:
            CFrames::iterator NextFrameIter = std::next(m_CurrentFrameIter);

            if (NextFrameIter == m_Frames.end())
            {
                UpdateReel();
            }
            else if (NextFrameIter->GetTimecode() < m_Clock.GetTime())
            {
                UpdateReel();
            }
            break;
        }
    }

    // -----------------------------------------------------------------------------

    inline bool CRecorder::IsEnd()
    {
        return m_CurrentFrameIter == m_Frames.end();
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::SetFPS(int _FramesPerSeconds)
    {
        m_FramesPerSeconds = _FramesPerSeconds;
        m_Frequenz         = 1.0 / (double)_FramesPerSeconds;
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::SetPlaybackMode(EPlaybackMode _Mode)
    {
        m_PlaybackMode = _Mode;
    }

    // -----------------------------------------------------------------------------

    inline double CRecorder::GetTime()
    {
        return m_Clock.GetTime();
    }

    // -----------------------------------------------------------------------------

    template <class TArchive>
    inline void CRecorder::Read(TArchive& _rCodec)
    {
        Clear();

        Base::Read(_rCodec, m_Frames);
    }

    // -----------------------------------------------------------------------------

    template <class TArchive>
    inline void CRecorder::Write(TArchive& _rCodec)
    {
        CheckEOL();

        Base::Write(_rCodec, m_Frames);
    }

    // -----------------------------------------------------------------------------

    void CRecorder::AcquireNewFrame()
    {
        CFrame NewFrame;

        NewFrame.SetIndex(m_Frames.size());

        m_Frames.push_back(NewFrame);
    }

    // -----------------------------------------------------------------------------

    void CRecorder::UpdateReel()
    {
        if (!IsEnd())
        {
            ++ m_CurrentFrameIter;
        }
    }

    // -----------------------------------------------------------------------------

    void CRecorder::Clear()
    {
        for (auto& rFrame : m_Frames)
        {
            rFrame.Clear();
        }

        m_Frames.clear();
    }

    // -----------------------------------------------------------------------------

    void CRecorder::CheckEOL()
    {
        if (m_Frames.size() > 0 && !m_Frames.back().HasData())
        {
            m_Frames.pop_back();
        }
    }
} // namespace Core