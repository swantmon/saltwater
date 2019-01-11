
#include "test_precompiled.h"

#include "base/base_clock.h"
#include "base/base_test_defines.h"

#include "engine/core/core_record_reader.h"
#include "engine/core/core_record_writer.h"

#include <array>

BASE_TEST(RecordDataWithRecorder)
{
    // -----------------------------------------------------------------------------
    // Data
    // -----------------------------------------------------------------------------
    struct SFrame
    {
        int   m_1;
        float m_2;
    };

    std::array<SFrame, 120> Frames;

    int Index = 0;

    for (auto& Frame : Frames)
    {
        Frame.m_1 = Index;
        Frame.m_2 = 1337.0f - float(Index);

        ++Index;
    }

    // -----------------------------------------------------------------------------
    // Recording
    // -----------------------------------------------------------------------------
    std::stringstream Stream;

    Core::CRecordWriter RecordWriter(Stream, 1);

    for (auto& Frame : Frames)
    {
        RecordWriter.WriteBinary(&Frame, sizeof(SFrame));
    }

    // -----------------------------------------------------------------------------
    // Read record
    // -----------------------------------------------------------------------------
    Core::CRecordReader RecordReader(Stream, 1);

    // -----------------------------------------------------------------------------
    // Check record w/ prev. record
    // -----------------------------------------------------------------------------    
    for (auto& Frame : Frames)
    {
        RecordReader.Update();

        SFrame FrameCheck;

        RecordReader.ReadBinary(&FrameCheck, sizeof(SFrame));

        BASE_CHECK(Frame.m_1 == FrameCheck.m_1);
        BASE_CHECK(Frame.m_2 == FrameCheck.m_2);
    }
}