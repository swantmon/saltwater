
#include "base/base_test_defines.h"

#include "base/base_string.h"

BASE_TEST(Test_Base_String)
{
    Base::CharString FranzInBayern("Franz jagt im komplett verwahrlosten Taxi quer durch Bayern");
    Base::CharString FranzInTaxi  ("Quer durch Bayern jagt Franz im komplett verwahrlosten Taxi");

    const Base::Char* pFranzInBayern = "Franz jagt im komplett verwahrlosten Taxi quer durch Bayern";

    BASE_CHECK(FranzInBayern  == FranzInBayern);
    BASE_CHECK(FranzInBayern  == pFranzInBayern);
    BASE_CHECK(pFranzInBayern == FranzInBayern);

    BASE_CHECK(FranzInBayern  != FranzInTaxi);
    BASE_CHECK(FranzInTaxi    != pFranzInBayern);
    BASE_CHECK(pFranzInBayern != FranzInTaxi);

    Base::CharString FranzCopy = FranzInBayern;

    BASE_CHECK(FranzCopy == FranzInBayern);

    FranzCopy = FranzInTaxi;

    BASE_CHECK(FranzCopy != FranzInBayern);

    FranzCopy = pFranzInBayern;

    BASE_CHECK(FranzCopy == FranzInBayern);

    FranzCopy = "Test Franz";

    BASE_CHECK(FranzCopy != FranzInBayern);
}