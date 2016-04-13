
#include "base/base_test_defines.h"

#include "base/base_tokenizer.h"

using namespace Base;

BASE_TEST(TestBaseTokenizerWriteToSCV)
{
    Base::CTokenizer Tokenizer;

    Tokenizer.Append("Test");
    Tokenizer.Append(1);
    Tokenizer.Append(2.123456f);
    Tokenizer.Append(-3);
    Tokenizer.Escape();

    Tokenizer.Append("Test2");
    Tokenizer.Append(10);
    Tokenizer.Append(20.0f);
    Tokenizer.Append(-30);
    Tokenizer.Escape();

    Tokenizer.SaveToFile("test.csv");
}