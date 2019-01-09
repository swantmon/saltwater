
#include "test_precompiled.h"

#include "base/base_test_defines.h"

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
    std::ofstream RecordFileOutput;

    RecordFileOutput.open("test_recording/test_record.txt");

    Base::CTextWriter Writer(RecordFileOutput, 1);

    Writer << Recorder;

    RecordFileOutput.close();

    // -----------------------------------------------------------------------------
    // Read record
    // -----------------------------------------------------------------------------
    Core::CRecorder RecorderCheck;

    std::ifstream RecordFileInput;

    RecordFileInput.open("test_recording/test_record.txt");

    Base::CTextReader Reader(RecordFileInput, 1);

    Reader >> RecorderCheck;

    RecordFileInput.close();

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
        Recorder.GetData(FrameCheck);

        RecorderCheck.Step();
        Recorder.Step();

        BASE_CHECK(Frame.m_1 == FrameCheck.m_1);
        BASE_CHECK(Frame.m_2 == FrameCheck.m_2);
        BASE_CHECK(Frame.m_3 == FrameCheck.m_3);
    }
}