
#include "test_precompiled.h"

#include "base/base_clock.h"
#include "base/base_test_defines.h"
#include "base/base_include_glm.h"

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
        std::vector<char> m_3;
    };

    std::array<SFrame, 120> Frames;

    int Index = 0;

    for (auto& Frame : Frames)
    {
        Frame.m_1 = Index;
        Frame.m_2 = 1337.0f - float(Index);

        Frame.m_3.resize(glm::linearRand(1, 4000000));

        ++Index;
    }

    // -----------------------------------------------------------------------------
    // Recording
    // -----------------------------------------------------------------------------
    std::stringstream Stream;

    Core::CRecordWriter RecordWriter(Stream, 1);

    for (auto& Frame : Frames)
    {
        RecordWriter << Frame.m_1;
        RecordWriter << Frame.m_2;

        Base::Write(RecordWriter, Frame.m_3);
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

        RecordReader >> FrameCheck.m_1;
        RecordReader >> FrameCheck.m_2;

        Base::Read(RecordReader, FrameCheck.m_3);

        BASE_CHECK(Frame.m_1 == FrameCheck.m_1);
        BASE_CHECK(Frame.m_2 == FrameCheck.m_2);
        BASE_CHECK(Frame.m_3.size() == FrameCheck.m_3.size());
    }
}