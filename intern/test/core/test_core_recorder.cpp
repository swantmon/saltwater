
#include "test_precompiled.h"

#include "base/base_test_defines.h"

#include "engine/core/core_recorder.h"

BASE_TEST(RecordDataWithRecorder)
{
    // -----------------------------------------------------------------------------
    // Data
    // -----------------------------------------------------------------------------
    int IntegerValue = 4;
    float FloatingValue = 13.37f;
    double DoubleValue = 12.34;
    char* pCharValue = "This is a test";
    std::string StringValue = "This is just another test!";

    struct SComplex
    {
        int m_Header;
        float m_Value;
    };

    SComplex ComplexStructValue = { 0, 1.0 };


    // -----------------------------------------------------------------------------
    // Recording
    // -----------------------------------------------------------------------------
    Core::CRecorder NewRecorder;

    NewRecorder.Dump(&IntegerValue, sizeof(IntegerValue));
    NewRecorder.Dump(&FloatingValue, sizeof(FloatingValue));
    NewRecorder.Dump(&pCharValue, strlen(pCharValue));

    NewRecorder.Step();

    NewRecorder.Dump(&DoubleValue, sizeof(DoubleValue));
    NewRecorder.Dump(&StringValue, sizeof(StringValue));
    NewRecorder.Dump(&ComplexStructValue, sizeof(SComplex));

    NewRecorder.Step();

    NewRecorder.SaveRecordingToFile("test_recording.7z");
}