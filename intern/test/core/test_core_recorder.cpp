
#include "test_precompiled.h"

#include "base/base_test_defines.h"
#include "base/base_serialize_binary_reader.h"
#include "base/base_serialize_binary_writer.h"
#include "base/base_serialize_text_reader.h"
#include "base/base_serialize_text_writer.h"

#include "engine/core/core_recorder.h"

#include <array>

BASE_TEST(RecordDataWithRecorder)
{
    // -----------------------------------------------------------------------------
    // Data
    // -----------------------------------------------------------------------------
    struct SFrame
    {
        int         m_1;
        float       m_2;
        std::string m_3;
    };

    std::array<SFrame, 120> Frames;

    int Index = 0;

    for (auto& Frame : Frames)
    {
        Frame.m_1 = Index;
        Frame.m_2 = 1337.0f - float(Index);
        Frame.m_3 = "The current frame number is " + std::to_string(Index);

        ++Index;
    }

    // -----------------------------------------------------------------------------
    // Recording
    // -----------------------------------------------------------------------------
    Core::CRecorder Recorder;

    for (auto& Frame : Frames)
    {
        Recorder.SetData(&Frame, sizeof(SFrame));

        Recorder.Step();
    }

    // -----------------------------------------------------------------------------
    // Write record
    // -----------------------------------------------------------------------------
    std::stringstream Stream;

    Base::CTextWriter Writer(Stream, 1);

    Writer << Recorder;

    // -----------------------------------------------------------------------------
    // Read record
    // -----------------------------------------------------------------------------
    Core::CRecorder RecorderCheck;

    Base::CTextReader Reader(Stream, 1);

    Reader >> RecorderCheck;

    // -----------------------------------------------------------------------------
    // Check record w/ prev. record
    // -----------------------------------------------------------------------------
    BASE_CHECK(Recorder.GetNumberOfFrames() == RecorderCheck.GetNumberOfFrames());

    Recorder.Restart();
    RecorderCheck.Restart();

    for (int FrameIndex = 0; FrameIndex < RecorderCheck.GetNumberOfFrames(); ++FrameIndex)
    {
        SFrame Frame;
        SFrame FrameCheck;

        Recorder.GetData(Frame);
        RecorderCheck.GetData(FrameCheck);

        RecorderCheck.Step();
        Recorder.Step();

        BASE_CHECK(Frame.m_1 == FrameCheck.m_1);
        BASE_CHECK(Frame.m_2 == FrameCheck.m_2);
        BASE_CHECK(Frame.m_3 == FrameCheck.m_3);
    }
}

BASE_TEST(RecordDataWithRecorderBinary)
{
    // -----------------------------------------------------------------------------
    // Data
    // -----------------------------------------------------------------------------
    struct SFrame
    {
        int         m_1;
        float       m_2;
        std::string m_3;
    };

    std::array<SFrame, 120> Frames;

    int Index = 0;

    for (auto& Frame : Frames)
    {
        Frame.m_1 = Index;
        Frame.m_2 = 1337.0f - float(Index);
        Frame.m_3 = "The current frame number is " + std::to_string(Index);

        ++Index;
    }

    // -----------------------------------------------------------------------------
    // Recording
    // -----------------------------------------------------------------------------
    Core::CRecorder Recorder;

    for (auto& Frame : Frames)
    {
        Recorder.SetData(&Frame, sizeof(SFrame));

        Recorder.Step();
    }

    // -----------------------------------------------------------------------------
    // Write record
    // -----------------------------------------------------------------------------
    std::stringstream Stream;

    Base::CBinaryWriter Writer(Stream, 1);

    Writer << Recorder;

    // -----------------------------------------------------------------------------
    // Read record
    // -----------------------------------------------------------------------------
    Core::CRecorder RecorderCheck;
    
    Base::CBinaryReader Reader(Stream, 1);

    Reader >> RecorderCheck;

    // -----------------------------------------------------------------------------
    // Check record w/ prev. record
    // -----------------------------------------------------------------------------
    BASE_CHECK(Recorder.GetNumberOfFrames() == RecorderCheck.GetNumberOfFrames());

    Recorder.Restart();
    RecorderCheck.Restart();

    for (int FrameIndex = 0; FrameIndex < RecorderCheck.GetNumberOfFrames(); ++FrameIndex)
    {
        SFrame Frame;
        SFrame FrameCheck;

        Recorder.GetData(Frame);
        RecorderCheck.GetData(FrameCheck);

        RecorderCheck.Step();
        Recorder.Step();

        BASE_CHECK(Frame.m_1 == FrameCheck.m_1);
        BASE_CHECK(Frame.m_2 == FrameCheck.m_2);
        BASE_CHECK(Frame.m_3 == FrameCheck.m_3);
    }
}

// -----------------------------------------------------------------------------

BASE_TEST(RecordDataWithRecorderWithFixedTime)
{
    // -----------------------------------------------------------------------------
    // Data
    // -----------------------------------------------------------------------------
    struct SFrame
    {
        int m_1;
    };

    std::array<SFrame, 120> Frames;

    int Index = 0;

    for (auto& Frame : Frames)
    {
        Frame.m_1 = Index;

        ++Index;
    }

    // -----------------------------------------------------------------------------
    // Recording
    // -----------------------------------------------------------------------------
    Core::CRecorder Recorder;

    for (auto& Frame : Frames)
    {
        Recorder.SetData(&Frame, sizeof(SFrame));

        Recorder.Step();
    }

    // -----------------------------------------------------------------------------
    // Write record
    // -----------------------------------------------------------------------------
    std::stringstream Stream;

    Base::CTextWriter Writer(Stream, 1);

    Writer << Recorder;

    // -----------------------------------------------------------------------------
    // Check record w/ prev. record
    // -----------------------------------------------------------------------------
    Recorder.Restart();

    Recorder.SetFPS(60);

    bool IsRecordEnd = false;

    BASE_TIME_RESET();

    while (!Recorder.IsEnd())
    {
        Recorder.Step();
    }

    BASE_TIME_LOG(TimeOfRecordIs2Sec);
}