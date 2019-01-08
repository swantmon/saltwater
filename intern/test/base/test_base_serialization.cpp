
#include "test_precompiled.h"

#include "base/base_test_defines.h"
#include "base/base_serialize_std_string.h"
#include "base/base_serialize_std_vector.h"
#include "base/base_serialize_text_reader.h"
#include "base/base_serialize_text_writer.h"

#include <vector>

static const char* s_pTmpSerializeFilename = "test.txt";


BASE_TEST(SerializeDataWithText)
{
    // -----------------------------------------------------------------------------
    // Data
    // -----------------------------------------------------------------------------
    int IntegerValue = 4;
    float FloatingValue = 13.37f;
    double DoubleValue = 12.34;
    char* pCharValue = "This is a test";
    std::string StringValue = "This is just another test!";

    std::vector<int> IntegerListValue = { 0, 1, 2, 3 };

    // -----------------------------------------------------------------------------
    // Writing
    // -----------------------------------------------------------------------------
    std::ofstream outFile;

    outFile.open(s_pTmpSerializeFilename);

    Base::CTextWriter TextWriter(outFile, 1);

    TextWriter << IntegerValue;
    TextWriter << FloatingValue;
    TextWriter << DoubleValue;
    TextWriter << pCharValue;

    Base::Serialize(TextWriter, StringValue);
    Base::Serialize(TextWriter, IntegerListValue);

    outFile.close();

    // -----------------------------------------------------------------------------
    // Test data
    // -----------------------------------------------------------------------------
    int IntegerValueTest;
    float FloatingValueTest;
    double DoubleValueTest;
    char* pCharValueTest;
    std::string StringValueTest;

    std::vector<int> IntegerListValueTest;

    // -----------------------------------------------------------------------------
    // Reading
    // -----------------------------------------------------------------------------
    std::ifstream inFile;

    inFile.open(s_pTmpSerializeFilename);

    Base::CTextReader TextReader(inFile, 1);

    TextReader >> IntegerValueTest;
    TextReader >> FloatingValueTest;
    TextReader >> DoubleValueTest;
    TextReader >> pCharValueTest;

    Base::Serialize(TextReader, StringValueTest);
    Base::Serialize(TextReader, IntegerListValueTest);

    outFile.close();

    // -----------------------------------------------------------------------------
    // Check
    // -----------------------------------------------------------------------------
    BASE_CHECK(IntegerValue == IntegerValueTest);
    BASE_CHECK(FloatingValue == FloatingValueTest);
    BASE_CHECK(DoubleValue == DoubleValueTest);
    BASE_CHECK(strcmp(pCharValue, pCharValueTest) == 0);
    BASE_CHECK(StringValue == StringValueTest);
}