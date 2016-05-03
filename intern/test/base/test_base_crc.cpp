
#include "test_precompiled.h"

#include "base/base_test_defines.h"

#include "base/base_crc.h"

BASE_TEST(Test_Base_CRC32_Franz)
{
    Base::BHash HashFranz;
    Base::BHash HashFranzAgain;
    Base::BHash HashFranzTwisted;

    HashFranz        = ::Base::CRC32("Franz jagt im komplett verwahrlosten Taxi quer durch Bayern", 59);
    HashFranzAgain   = ::Base::CRC32("Franz jagt im komplett verwahrlosten Taxi quer durch Bayern", 59);
    HashFranzTwisted = ::Base::CRC32("Quer durch Bayern jagt Franz im komplett verwahrlosten Taxi", 59);

    BASE_CHECK( HashFranz == HashFranzAgain   );

    BASE_CHECK( HashFranz != HashFranzTwisted );
}


BASE_TEST(Test_Base_CRC32_Franz_Advanced)
{
    Base::BHash HashFranzSplit;
    Base::BHash HashFranzNormal;

    HashFranzSplit  = ::Base::CRC32("Franz jagt im komplett verwahrlosten Taxi quer ", 47);
    HashFranzSplit  = ::Base::CRC32(HashFranzSplit, "durch Bayern", 12);
    HashFranzNormal = ::Base::CRC32("Franz jagt im komplett verwahrlosten Taxi quer durch Bayern", 59);

    BASE_CHECK( HashFranzSplit == HashFranzNormal );
}