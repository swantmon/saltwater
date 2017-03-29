
#include "test_precompiled.h"

#include "base/base_test_defines.h"

#include "base/base_managed_pool.h"

#include <vector>

using namespace std;

BASE_TEST(Test_Base_ManagedPool_Compare)
{
    class CComplexClass : public Base::CManagedPoolItemBase
    {
    public:
        
        CComplexClass()
            : m_A(-1)
        {
            m_pData = new char[256];
        }
        
        ~CComplexClass()
        {
            delete[] m_pData;
        }

        void SetA(int _Value)
        {
            m_A = _Value;
        }
        
        int GetA()
        {
            return m_A;
        }
        
    private:
        
        int m_A;
        void* m_pData;
    };

    // -----------------------------------------------------------------------------

    typedef Base::CManagedPoolItemPtr<CComplexClass> CComplexPtr;

    typedef Base::CManagedPool<CComplexClass, 16, 1> CComplexClasses;
    
    // -----------------------------------------------------------------------------

    CComplexClasses ClassPool;
    
    // -----------------------------------------------------------------------------
    
    CComplexClass* TestPtr = ClassPool.Allocate();

    TestPtr->SetA(1337);

    BASE_CHECK(TestPtr->GetA() == 1337);
}