
#include "test_precompiled.h"

#include "base/base_clock.h"
#include "base/base_test_defines.h"
#include "base/base_include_glm.h"

#include "engine/core/core_record_reader.h"
#include "engine/core/core_record_writer.h"

#include <array>
#include <fstream>
#include <sstream>
#include <iostream>

BASE_TEST(RecordDataWithRecorderSStream)
{
    // -----------------------------------------------------------------------------
    // Data
    // -----------------------------------------------------------------------------
    struct SFrame
    {
        int   m_1;
        float m_2;
        std::vector<char> m_3;
    };

    std::array<SFrame, 120> Frames;

    int Index = 0;

    for (auto& OriginalFrame : Frames)
    {
        OriginalFrame.m_1 = Index;
        OriginalFrame.m_2 = 1337.0f - float(Index);

        OriginalFrame.m_3.resize(glm::linearRand(1, 4000000));

        ++Index;
    }

    // -----------------------------------------------------------------------------
    // Recording
    // -----------------------------------------------------------------------------
    std::stringstream Stream;

    Core::CRecordWriter RecordWriter(Stream, 1);

    for (auto& OriginalFrame : Frames)
    {
        RecordWriter << OriginalFrame.m_1;
        RecordWriter << OriginalFrame.m_2;

        Base::Write(RecordWriter, OriginalFrame.m_3);
    }

    Stream.seekg(Stream.beg);

    // -----------------------------------------------------------------------------
    // Read record
    // -----------------------------------------------------------------------------
    Core::CRecordReader RecordReaderFPS(Stream, 1);

    // -----------------------------------------------------------------------------
    // Check record w/ prev. record
    // -----------------------------------------------------------------------------
    for (auto& OriginalFrame : Frames)
    {
        SFrame ReadFrame;

        RecordReaderFPS >> ReadFrame.m_1;
        RecordReaderFPS >> ReadFrame.m_2;

        Base::Read(RecordReaderFPS, ReadFrame.m_3);

        BASE_CHECK(OriginalFrame.m_1 == ReadFrame.m_1);
        BASE_CHECK(OriginalFrame.m_2 == ReadFrame.m_2);
        BASE_CHECK(OriginalFrame.m_3.size() == ReadFrame.m_3.size());
    }
}

// -----------------------------------------------------------------------------

BASE_TEST(RecordDataWithRecorderFStream)
{
    // -----------------------------------------------------------------------------
    // Data
    // -----------------------------------------------------------------------------
    struct SFrame
    {
        int   m_1;
        float m_2;
        std::vector<char> m_3;
    };

    std::array<SFrame, 120> Frames;

    int Index = 0;

    for (auto& OriginalFrame : Frames)
    {
        OriginalFrame.m_1 = Index;
        OriginalFrame.m_2 = 1337.0f - float(Index);

        OriginalFrame.m_3.resize(glm::linearRand(1, 4000));

        for (auto& rChar : OriginalFrame.m_3)
        {
            rChar = glm::linearRand(0, 255);
        }

        ++Index;
    }

    // -----------------------------------------------------------------------------
    // Recording
    // -----------------------------------------------------------------------------
    std::ofstream OutFileStream;

    OutFileStream.open("test.rec", std::ofstream::binary);

    BASE_CHECK(OutFileStream.is_open());

    Core::CRecordWriter RecordWriter(OutFileStream, 1);

    for (auto& OriginalFrame : Frames)
    {
        RecordWriter << OriginalFrame.m_1;
        RecordWriter << OriginalFrame.m_2;

        Base::Write(RecordWriter, OriginalFrame.m_3);
    }

    OutFileStream.close();

    // -----------------------------------------------------------------------------
    // Read record
    // -----------------------------------------------------------------------------
    std::ifstream InFileStream;

    InFileStream.open("test.rec", std::ifstream::binary);

    BASE_CHECK(InFileStream.is_open());

    Core::CRecordReader RecordReader(InFileStream, 1);

    // -----------------------------------------------------------------------------
    // Check record w/ prev. record
    // -----------------------------------------------------------------------------
    for (auto& OriginalFrame : Frames)
    {
        SFrame ReadFrame;

        RecordReader >> ReadFrame.m_1;
        RecordReader >> ReadFrame.m_2;

        Base::Read(RecordReader, ReadFrame.m_3);

        BASE_CHECK(OriginalFrame.m_1 == ReadFrame.m_1);
        BASE_CHECK(OriginalFrame.m_2 == ReadFrame.m_2);
        BASE_CHECK(OriginalFrame.m_3.size() == ReadFrame.m_3.size());

        int Index = 0;

        for (auto& rChar : OriginalFrame.m_3)
        {
            BASE_CHECK(rChar == ReadFrame.m_3[Index]);

            ++Index;
        }
    }

    InFileStream.close();
}

// -----------------------------------------------------------------------------

BASE_TEST(RecordDataWithRecorderTimecode)
{
    // -----------------------------------------------------------------------------
    // Data
    // -----------------------------------------------------------------------------
    struct SFrame
    {
        int   m_1;
        float m_2;
        std::vector<char> m_3;
    };

    std::array<SFrame, 30> Frames;

    int Index = 0;

    for (auto& OriginalFrame : Frames)
    {
        OriginalFrame.m_1 = Index;
        OriginalFrame.m_2 = 1337.0f - float(Index);

        OriginalFrame.m_3.resize(glm::linearRand(1, 400));

        ++Index;
    }

    // -----------------------------------------------------------------------------
    // Record data with 30 FPS
    // -----------------------------------------------------------------------------
    std::stringstream Stream;

    Core::CRecordWriter RecordWriter(Stream, 1);

    Base::CPerformanceClock Clock;

    BASE_TIME_RESET();

    double CurrentFrameTime = 0.0;

    for (int Index = 0; Index < Frames.size();)
    {
        Clock.OnFrame();

        auto& OriginalFrame = Frames[Index];

        CurrentFrameTime += Clock.GetDurationOfFrame();

        if (CurrentFrameTime > 1.0 / 30.0)
        {
            RecordWriter << OriginalFrame.m_1;
            RecordWriter << OriginalFrame.m_2;

            Base::Write(RecordWriter, OriginalFrame.m_3);

            ++ Index;

            CurrentFrameTime = 0.0;
        }
    }

    BASE_TIME_LOG(FillRecorderIn1Sec);

    // -----------------------------------------------------------------------------
    // Read record
    // -----------------------------------------------------------------------------
    Core::CRecordReader RecordReaderFPS(Stream, 1);

    // -----------------------------------------------------------------------------
    // Check record w/ prev. record
    // -----------------------------------------------------------------------------
    Index = 0;

    CurrentFrameTime = 0.0;

    Clock.Reset();

    BASE_TIME_RESET();

    while (!RecordReaderFPS.IsEnd())
    {
        Clock.OnFrame();

        CurrentFrameTime += Clock.GetDurationOfFrame();

        if (CurrentFrameTime > 1.0 / 30.0)
        {
            SFrame ReadFrame;

            RecordReaderFPS >> ReadFrame.m_1;
            RecordReaderFPS >> ReadFrame.m_2;

            Base::Read(RecordReaderFPS, ReadFrame.m_3);

            auto& OriginalFrame = Frames[Index];

            BASE_CHECK(OriginalFrame.m_1 == ReadFrame.m_1);
            BASE_CHECK(OriginalFrame.m_2 == ReadFrame.m_2);
            BASE_CHECK(OriginalFrame.m_3.size() == ReadFrame.m_3.size());

            ++Index;

            CurrentFrameTime = 0.0;
        }
    }

    BASE_TIME_LOG(ReadRecorderIn1SecViaFPS);

    // -----------------------------------------------------------------------------
    // Read record
    // -----------------------------------------------------------------------------
    Core::CRecordReader RecordReaderTimecode(Stream, 1);

    // -----------------------------------------------------------------------------
    // Check record w/ prev. record
    // -----------------------------------------------------------------------------
    Index = 0;

    BASE_TIME_RESET();

    while (!RecordReaderTimecode.IsEnd())
    {
        RecordReaderTimecode.Update();

        if (RecordReaderTimecode.PeekTimecode() < RecordReaderTimecode.GetTime())
        {
            SFrame ReadFrame;

            RecordReaderTimecode >> ReadFrame.m_1;
            RecordReaderTimecode >> ReadFrame.m_2;

            Base::Read(RecordReaderTimecode, ReadFrame.m_3);

            auto& OriginalFrame = Frames[Index];

            BASE_CHECK(OriginalFrame.m_1 == ReadFrame.m_1);
            BASE_CHECK(OriginalFrame.m_2 == ReadFrame.m_2);
            BASE_CHECK(OriginalFrame.m_3.size() == ReadFrame.m_3.size());

            ++Index;
        }
    }

    BASE_TIME_LOG(ReadRecorderIn1SecViaTimecode);
}