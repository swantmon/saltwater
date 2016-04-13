
#include "base/base_test_defines.h"

#include "base/base_memory.h"

BASE_TEST(Test_Memory_Timing)
{
    static const unsigned int NumberOfBytes = 1<<16;
    
    unsigned char* pBytes        = static_cast<unsigned char*>(Base::CMemory::Allocate(NumberOfBytes));
    unsigned char* pMemCopyBytes = static_cast<unsigned char*>(Base::CMemory::Allocate(NumberOfBytes));
    unsigned char* pMemMoveBytes = static_cast<unsigned char*>(Base::CMemory::Allocate(NumberOfBytes));
    
    BASE_TIME_RESET();
    
    for (unsigned int i = 0; i < NumberOfBytes; ++i )
    {
        pBytes[i] = 0;
    }
    
    BASE_TIME_LOG(Index);
    
    for (unsigned int i = 0; i < NumberOfBytes; ++i )
    {
        pBytes[i] = 2;
    }
    
    BASE_TIME_RESET();
    
    memmove(pMemMoveBytes, pBytes, NumberOfBytes);
    
    BASE_TIME_LOG(Memmove);
    
    for (unsigned int i = 0; i < NumberOfBytes; ++i )
    {
        pBytes[i] = 1;
    }
    
    BASE_TIME_RESET();
    
    memcpy(pMemCopyBytes, pBytes, NumberOfBytes);
    
    BASE_TIME_LOG(Memcpy);
}