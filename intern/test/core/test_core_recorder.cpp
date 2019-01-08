
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
    Core::CRecorder NewRecorder("test_recording/", true, 3);

    NewRecorder.Write(&IntegerValue, sizeof(IntegerValue), 0);
    NewRecorder.Write(&FloatingValue, sizeof(FloatingValue), 1);
    NewRecorder.Write(&pCharValue, strlen(pCharValue), 2);

    NewRecorder.Step();

    NewRecorder.Write(&DoubleValue, sizeof(DoubleValue), 0);
    NewRecorder.Write(&StringValue, sizeof(StringValue), 1);
    NewRecorder.Write(&ComplexStructValue, sizeof(SComplex), 2);

    NewRecorder.Stop();

    // -----------------------------------------------------------------------------
    // Test data
    // -----------------------------------------------------------------------------
    int IntegerValueTest;
    float FloatingValueTest;
    double DoubleValueTest;
    char pCharValueTest[14];
    std::string StringValueTest;

    SComplex ComplexStructValueTest = { 0, 0 };

    // -----------------------------------------------------------------------------
    // Playing
    // -----------------------------------------------------------------------------
    Core::CRecorder NewRecordPlayer("test_recording/", false);

    NewRecordPlayer.Read(&IntegerValueTest, sizeof(IntegerValueTest), 0);
    NewRecordPlayer.Read(&FloatingValueTest, sizeof(FloatingValueTest), 1);
    NewRecordPlayer.Read(&pCharValueTest, 14, 2);

    NewRecordPlayer.Step();

    NewRecordPlayer.Read(&DoubleValueTest, sizeof(DoubleValueTest), 0);
    NewRecordPlayer.Read(&StringValueTest, sizeof(StringValueTest), 1);
    NewRecordPlayer.Read(&ComplexStructValueTest, sizeof(SComplex), 2);

    // -----------------------------------------------------------------------------
    // Check
    // -----------------------------------------------------------------------------
    BASE_CHECK(IntegerValue == IntegerValueTest);
    BASE_CHECK(FloatingValue == FloatingValueTest);
    BASE_CHECK(DoubleValue == DoubleValueTest);
    BASE_CHECK(strcmp(pCharValue, pCharValueTest) == 0);
    BASE_CHECK(StringValue == StringValueTest);
}