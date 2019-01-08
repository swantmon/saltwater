
#include "test_precompiled.h"

#include "base/base_test_defines.h"
#include "base/base_serialize_std_string.h"
#include "base/base_serialize_std_vector.h"
#include "base/base_serialize_text_reader.h"
#include "base/base_serialize_text_writer.h"
#include "base/base_serialize_binary_reader.h"
#include "base/base_serialize_binary_writer.h"

#include <vector>

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
    // Stream (this could be also a file)
    // -----------------------------------------------------------------------------
    std::stringstream Stream;

    // -----------------------------------------------------------------------------
    // Writing
    // -----------------------------------------------------------------------------
    Base::CTextWriter Writer(Stream, 1);

    Writer << IntegerValue;
    Writer << FloatingValue;
    Writer << DoubleValue;
    Writer << pCharValue;

    Base::Serialize(Writer, StringValue);
    Base::Serialize(Writer, IntegerListValue);

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
    Base::CTextReader Reader(Stream, 1);

    Reader >> IntegerValueTest;
    Reader >> FloatingValueTest;
    Reader >> DoubleValueTest;
    Reader >> pCharValueTest;

    Base::Serialize(Reader, StringValueTest);
    Base::Serialize(Reader, IntegerListValueTest);

    // -----------------------------------------------------------------------------
    // Check
    // -----------------------------------------------------------------------------
    BASE_CHECK(IntegerValue == IntegerValueTest);
    BASE_CHECK(FloatingValue == FloatingValueTest);
    BASE_CHECK(DoubleValue == DoubleValueTest);
    BASE_CHECK(strcmp(pCharValue, pCharValueTest) == 0);
    BASE_CHECK(StringValue == StringValueTest);
}

BASE_TEST(SerializeDataWithBinary)
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
    // Stream (this could be also a file)
    // -----------------------------------------------------------------------------
    std::stringstream Stream;

    // -----------------------------------------------------------------------------
    // Writing
    // -----------------------------------------------------------------------------
    Base::CBinaryWriter Writer(Stream, 1);

    Writer << IntegerValue;
    Writer << FloatingValue;
    Writer << DoubleValue;
    Writer << pCharValue;

    Base::Serialize(Writer, StringValue);
    Base::Serialize(Writer, IntegerListValue);

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
    Base::CBinaryReader Reader(Stream, 1);

    Reader >> IntegerValueTest;
    Reader >> FloatingValueTest;
    Reader >> DoubleValueTest;
    Reader >> pCharValueTest;

    Base::Serialize(Reader, StringValueTest);
    Base::Serialize(Reader, IntegerListValueTest);

    // -----------------------------------------------------------------------------
    // Check
    // -----------------------------------------------------------------------------
    BASE_CHECK(IntegerValue == IntegerValueTest);
    BASE_CHECK(FloatingValue == FloatingValueTest);
    BASE_CHECK(DoubleValue == DoubleValueTest);
    BASE_CHECK(strcmp(pCharValue, pCharValueTest) == 0);
    BASE_CHECK(StringValue == StringValueTest);
}