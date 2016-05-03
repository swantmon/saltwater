
#include "test_precompiled.h"

#include "base/base_test_defines.h"

#include "base/base_pool.h"

#include <vector>

using namespace std;

BASE_TEST(Test_Base_Pool_Managment)
{
    class CComplexClass
    {
    public:
        
        CComplexClass()
            : m_A(-1)
        {
            
        }
        
        ~CComplexClass()
        {
            
        }
        
        int HelloWorld()
        {
            return m_A;
        }
        
    private:
        
        int m_A;
    };
    
    // -----------------------------------------------------------------------------
    
    struct SComplexStruct
    {
        int m_A;
    };
    
    // -----------------------------------------------------------------------------

    Base::CPool<CComplexClass>  ClassPool;
    Base::CPool<SComplexStruct> StructPool;
    Base::CPool<float>          FloatPool;
    
    // -----------------------------------------------------------------------------
    
    CComplexClass& rNewClassFromPoolOne   = ClassPool.Allocate();
    CComplexClass& rNewClassFromPoolTwo   = ClassPool.Allocate();
    CComplexClass& rNewClassFromPoolThree = ClassPool.Allocate();
    
    BASE_CHECK(rNewClassFromPoolOne.HelloWorld() == -1);
    
    BASE_CHECK(rNewClassFromPoolOne.HelloWorld() == rNewClassFromPoolTwo.HelloWorld());
    
    Base::Size NewClassFromPoolID = ClassPool.GetID(rNewClassFromPoolOne);
    
    CComplexClass& rNewClassFromPoolGet = ClassPool.GetItem(NewClassFromPoolID);
    
    BASE_CHECK(&rNewClassFromPoolOne == & rNewClassFromPoolGet);
    
    BASE_CHECK(ClassPool.GetNumberOfItems() == 3);
    
    ClassPool.Free(&rNewClassFromPoolThree);
    
    BASE_CHECK(ClassPool.GetNumberOfItems() == 2);
    
    ClassPool.Clear();
    
    BASE_CHECK(ClassPool.GetNumberOfItems() == 0);
    
    // -----------------------------------------------------------------------------
    
    SComplexStruct& rNewStructFromPoolOne   = StructPool.Allocate();
    SComplexStruct& rNewStructFromPoolTwo   = StructPool.Allocate();
    SComplexStruct& rNewStructFromPoolThree = StructPool.Allocate();
    
    rNewStructFromPoolOne.m_A = -1;
    rNewStructFromPoolTwo.m_A = -2;
    
    Base::Size NewStructFromPoolID = StructPool.GetID(rNewStructFromPoolOne);
    
    SComplexStruct& rNewStructFromPoolGet = StructPool.GetItem(NewStructFromPoolID);
    
    BASE_CHECK(&rNewStructFromPoolOne == & rNewStructFromPoolGet);
    
    BASE_CHECK(rNewStructFromPoolOne.m_A == rNewStructFromPoolGet.m_A);
    
    BASE_CHECK(StructPool.GetNumberOfItems() == 3);
    
    StructPool.Free(&rNewStructFromPoolThree);
    
    BASE_CHECK(StructPool.GetNumberOfItems() == 2);
    
    StructPool.Clear();
    
    BASE_CHECK(StructPool.GetNumberOfItems() == 0);
    
    // -----------------------------------------------------------------------------
    
    for (unsigned int IndexOfElement = 0; IndexOfElement < 128; ++IndexOfElement)
    {
        float& rCheck = FloatPool.Allocate();
        
        rCheck = static_cast<float>(IndexOfElement);
    }
    
    BASE_CHECK(FloatPool.GetNumberOfItems() == 128);
    
    unsigned int IndexCheck = 0;
    
    Base::CPool<float>::CIterator Current = FloatPool.Begin();
    Base::CPool<float>::CIterator End     = FloatPool.End();
    
    for (; Current != End; ++ Current)
    {
        BASE_CHECK(*Current == static_cast<float>(IndexCheck));
        
        ++IndexCheck;
    }
    
    FloatPool.Clear();
    
    BASE_CHECK(FloatPool.GetNumberOfItems() == 0);
}

// -----------------------------------------------------------------------------

BASE_TEST(Test_Base_Pool_Timing)
{
    static const unsigned int s_NumberOfItems = 2048;
    
    vector<float>                       FloatVector;
    Base::CPool<float, s_NumberOfItems> FloatPool;
    unsigned int IndexCheck;
    
    FloatVector.reserve(s_NumberOfItems);
    
    // -----------------------------------------------------------------------------
    
    BASE_TIME_RESET();
    
    for (unsigned int IndexOfElement = 0; IndexOfElement < s_NumberOfItems; ++IndexOfElement)
    {
        float& rCheck = FloatPool.Allocate();
        
        rCheck = static_cast<float>(IndexOfElement);
    }
    
    BASE_TIME_LOG(Fill_Pool_With_Data);
    
    IndexCheck = 0;
    
    Base::CPool<float, s_NumberOfItems>::CIterator PoolCurrent = FloatPool.Begin();
    Base::CPool<float, s_NumberOfItems>::CIterator PoolEnd     = FloatPool.End();
    
    BASE_TIME_RESET();
    
    for (; PoolCurrent != PoolEnd; ++ PoolCurrent)
    {
        if(*PoolCurrent == static_cast<float>(IndexCheck))
        {
            ++IndexCheck;
        }
    }
    
    BASE_TIME_LOG(Iterate_Pool_And_Check_Data);
    
    BASE_CHECK(IndexCheck == s_NumberOfItems);

    FloatPool.Clear();
    
    // -----------------------------------------------------------------------------
    
    BASE_TIME_RESET();
    
    for (unsigned int IndexOfElement = 0; IndexOfElement < s_NumberOfItems; ++IndexOfElement)
    {
        float Check = static_cast<float>(IndexOfElement);
        
        FloatVector.push_back(Check);
    }
    
    BASE_TIME_LOG(Fill_Vector_With_Data);
    
    IndexCheck = 0;
    
    vector<float>::iterator VectorCurrent = FloatVector.begin();
    vector<float>::iterator VectorEnd     = FloatVector.end();
    
    BASE_TIME_RESET();
    
    for (; VectorCurrent != VectorEnd; ++ VectorCurrent)
    {
        if(*VectorCurrent == static_cast<float>(IndexCheck))
        {
            ++IndexCheck;
        }
    }
    
    BASE_TIME_LOG(Iterate_Vector_And_Check_Data);
    
    BASE_CHECK(IndexCheck == s_NumberOfItems);
    
    FloatVector.clear();
}