#pragma once

#include "base/base_memory.h"
#include "base/base_serialize_text_reader.h"
#include "base/base_serialize_text_writer.h"
#include "base/base_serialize_std_vector.h"

#include <fstream>

namespace Core
{
    class CRecorder
    {        
    public:

        CRecorder();
        ~CRecorder();

    public:

        template<class TType>
        inline void GetFrameData(TType& _rBytes);

        inline void SetFrameData(void* _pBytes, size_t _NumberOfBytes);

        inline int GetNumberOfFrames();

        inline void Restart();

        inline void Step();
                
    public:

        template <class TArchive>
        inline void Read(TArchive& _rCodec);

        template <class TArchive>
        inline void Write(TArchive& _rCodec);

    private:

        class CFrame
        {

        public:

            CFrame();
            ~CFrame();

            void SetTimecode(size_t _Timecode);
            size_t GetTimecode() const;

            void Get(void*& _prBytes, size_t& _rNumberOfBytes);

            void Set(void* _pBytes, size_t _NumberOfBytes);

            void Clear();

        public:

            template <class TArchive>
            inline void Read(TArchive& _rCodec);

            template <class TArchive>
            inline void Write(TArchive& _rCodec);

        private:

            size_t m_Timecode;
            void* m_pBytes;
            size_t m_NumberOfBytes;
        };

    private:

        typedef std::vector<CFrame> CFrames;

    private:

        CFrames m_Frames;

        int m_FrameIndex;

    private:

        void AddFrame();
    };
} // namespace Core

namespace Core
{
    CRecorder::CFrame::CFrame()
        : m_Timecode     (-1)
        , m_pBytes       (0)
        , m_NumberOfBytes(0)
    { }

    // -----------------------------------------------------------------------------

    CRecorder::CFrame::~CFrame()
    {
    }

    // -----------------------------------------------------------------------------

    void CRecorder::CFrame::SetTimecode(size_t _Timecode)
    {
        m_Timecode = _Timecode;
    }

    // -----------------------------------------------------------------------------

    size_t CRecorder::CFrame::GetTimecode() const
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

    inline void CRecorder::CFrame::Clear()
    {
        if (m_pBytes != 0) Base::CMemory::Free(m_pBytes);

        m_pBytes        = 0;
        m_NumberOfBytes = 0;
    }

    // -----------------------------------------------------------------------------

    template <class TArchive>
    inline void CRecorder::CFrame::Read(TArchive& _rCodec)
    {
        _rCodec >> m_Timecode;
        _rCodec >> m_NumberOfBytes;

        m_pBytes = Base::CMemory::Allocate(m_NumberOfBytes);

        _rCodec.ReadBinary(m_pBytes, m_NumberOfBytes);
    }

    // -----------------------------------------------------------------------------

    template <class TArchive>
    inline void CRecorder::CFrame::Write(TArchive& _rCodec)
    {
        _rCodec << m_Timecode;
        _rCodec << m_NumberOfBytes;

        _rCodec.WriteBinary(m_pBytes, m_NumberOfBytes);
    }
} // namespace Core

namespace Core
{
    CRecorder::CRecorder()
        : m_FrameIndex(0)
    {
        if (m_Frames.size() == 0) AddFrame();
    }

    // -----------------------------------------------------------------------------

    CRecorder::~CRecorder()
    {
        for (auto& rFrame : m_Frames)
        {
            rFrame.Clear();
        }
    }

    // -----------------------------------------------------------------------------

    template<class TType>
    inline void CRecorder::GetFrameData(TType& _rBytes)
    {
        CFrame& rCurrentFrame = m_Frames[m_FrameIndex];

        void* pBytes = 0;
        size_t NumberOfBytes = 0;

        rCurrentFrame.Get(pBytes, NumberOfBytes);

        _rBytes = *static_cast<TType*>(pBytes);
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::SetFrameData(void* _pBytes, size_t _NumberOfBytes)
    {
        if (m_FrameIndex == GetNumberOfFrames()) AddFrame();

        CFrame& rCurrentFrame = m_Frames[m_FrameIndex];

        rCurrentFrame.Set(_pBytes, _NumberOfBytes);
    }

    // -----------------------------------------------------------------------------

    inline int CRecorder::GetNumberOfFrames()
    {
        return m_Frames.size();
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::Restart()
    {
        m_FrameIndex = 0;
    }

    // -----------------------------------------------------------------------------

    inline void CRecorder::Step()
    {
        ++m_FrameIndex;
    }

    // -----------------------------------------------------------------------------

    template <class TArchive>
    inline void CRecorder::Read(TArchive& _rCodec)
    {
        m_Frames.clear();

        Base::Read(_rCodec, m_Frames);
    }

    // -----------------------------------------------------------------------------

    template <class TArchive>
    inline void CRecorder::Write(TArchive& _rCodec)
    {
        Base::Write(_rCodec, m_Frames);
    }

    // -----------------------------------------------------------------------------

    void CRecorder::AddFrame()
    {
        CFrame NewFrame;

        NewFrame.SetTimecode(m_FrameIndex);

        m_Frames.push_back(NewFrame);
    }
} // namespace Core