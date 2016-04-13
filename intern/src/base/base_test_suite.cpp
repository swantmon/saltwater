
#include "base/base_defines.h"
#include "base/base_clock.h"
#include "base/base_timer.h"

#include <assert.h>
#include <sstream>

namespace 
{
	class CTestSuite
    {
        
    public:
        
        CTestSuite();
        ~CTestSuite();

    public:
        
        static CTestSuite& GetInstance();

        void Reset(std::ostream* _pStream = 0);

        void RegisterTest(void (*_TestFtr)());
        void RunTests();

        void SetTestSource(const char* _pFileName, const char* _pFunctionName, int _LineNumber);

        void LogFailure();
        void LogException(const char* _pException);
        void LogCondition(const char* _pCondition);
        void LogTime(const char* _pMessage);
        
        void ResetTime();

        std::ostringstream& GetLogStream();

        unsigned int GetTestCount();
        unsigned int GetFailedTestCount();

    private:

        struct SPage;

    private:

        struct STest
        {
            void (*m_TestFtr)();
        };

        struct SPageHeader
        {
            SPage* m_pNext;
        };

    private:

        static const int s_NumberOfBytesPerPage = 4096;
        static const int s_NumberOfTestsPerPage = (s_NumberOfBytesPerPage - sizeof(SPageHeader)) / sizeof(STest);

    private:

        struct SPage : public SPageHeader
        {
            STest m_Tests[s_NumberOfTestsPerPage];
        };

    private:

        SPage* m_pFirstPage;
        SPage* m_pLastPage;

        int m_NumberOfTests;
        int m_NumberOfFailedTests;
        int m_NumberOfFailedChecks;

        int         m_CurrentLineNumber;
        std::string m_CurrentFunctionName;
        std::string m_CurrentFileName;

        STest* m_pCurrentTest;

        std::ostringstream m_LogStream;
        
        Base::CPerformanceClock m_Clock;
        
        double m_PreviousTimestamp;

    private:

        void LogStatistics();

        void ResetFailedCheckCount();
        void ResetCounters();
    };
} // namespace

namespace 
{
    CTestSuite& CTestSuite::GetInstance()
    {
        static CTestSuite s_Instance;
        
        return s_Instance;
    }
    
    // -----------------------------------------------------------------------------
    
    CTestSuite::CTestSuite()
        : m_pFirstPage          (0)
        , m_pLastPage           (0)
        , m_NumberOfTests       (0)
        , m_NumberOfFailedTests (0)
        , m_NumberOfFailedChecks(0)
        , m_CurrentLineNumber   (0)
        , m_CurrentFunctionName ()
        , m_CurrentFileName     ()
        , m_pCurrentTest        (0)
        , m_LogStream           ()
        , m_Clock               ()
        , m_PreviousTimestamp   (0.0)
    {
    }

    // -----------------------------------------------------------------------------

    CTestSuite::~CTestSuite()
    {
        SPage* pPage;

        while (m_pFirstPage != 0)
        {
            pPage = m_pFirstPage->m_pNext;

            free(m_pFirstPage);

            m_pFirstPage = pPage;
        }

        m_LogStream.clear();
    }

    // -----------------------------------------------------------------------------

    void CTestSuite::Reset(std::ostream* _pStream)
    {
        BASE_UNUSED(_pStream);

        ResetCounters();
    }

    // -----------------------------------------------------------------------------

    void CTestSuite::RegisterTest(void (*_TestFtr)())
    {
        int    IndexOfTestInPage;
        SPage* pPage;

        IndexOfTestInPage = m_NumberOfTests % s_NumberOfTestsPerPage;

        if (IndexOfTestInPage == 0)
        {
            if (m_pFirstPage == 0)
            {
                m_pFirstPage = reinterpret_cast<SPage*>(malloc(s_NumberOfBytesPerPage));
                
                assert(m_pFirstPage != 0);

                m_pFirstPage->m_pNext = 0;

                m_pLastPage = m_pFirstPage;
                m_pLastPage->m_pNext = 0;
            }
            else
            {
                pPage = reinterpret_cast<SPage*>(malloc(s_NumberOfBytesPerPage));
                pPage->m_pNext = 0;

                m_pLastPage->m_pNext = pPage;
                m_pLastPage          = pPage;
            }
        }

        m_pLastPage->m_Tests[IndexOfTestInPage].m_TestFtr = _TestFtr;

        ++ m_NumberOfTests;
    }

    // -----------------------------------------------------------------------------

    void CTestSuite::RunTests()
    {
        int IndexOfTestInPage;
        int NumberOfTests;

        SPage* pPage;

        IndexOfTestInPage = 0;
        NumberOfTests     = 0;

        pPage        = m_pFirstPage;

        while (NumberOfTests < m_NumberOfTests)
        {
            ResetFailedCheckCount();

            IndexOfTestInPage = NumberOfTests % s_NumberOfTestsPerPage;

            m_pCurrentTest = &pPage->m_Tests[IndexOfTestInPage];

            try
            {
                m_pCurrentTest->m_TestFtr();
            }
            catch (std::exception& _rException)
            {
                LogFailure();
                LogException(_rException.what());
            }
            catch (...)
            {
                LogFailure();
                LogException("uncaught exception");
            }

            if (m_NumberOfFailedChecks > 0)
            {
                ++ m_NumberOfFailedTests;

                ResetFailedCheckCount();
                
                m_LogStream << std::endl;
            }

            if ((IndexOfTestInPage + 1) == s_NumberOfTestsPerPage)
            {
                pPage = pPage->m_pNext;
            }

            ++ NumberOfTests;
        }

        LogStatistics();
    }

    // -----------------------------------------------------------------------------

    void CTestSuite::SetTestSource(const char* _pFileName, const char* _pFunctionName, int _LineNumber)
    {
        m_CurrentFileName     = _pFileName;
        m_CurrentFunctionName = _pFunctionName;
        m_CurrentLineNumber   = _LineNumber;
    }

    // -----------------------------------------------------------------------------

    void CTestSuite::LogFailure()
    {
        m_LogStream << "Error:     " << m_CurrentFunctionName << std::endl;
        m_LogStream << "File :     " << m_CurrentFileName << " (" << m_CurrentLineNumber << ")" << std::endl;

        ++ m_NumberOfFailedChecks;
    }

    // -----------------------------------------------------------------------------

    void CTestSuite::LogException(const char* _pException)
    {
        assert(_pException != 0);

        m_LogStream << "Exception: " << _pException << std::endl << std::endl;
    }

    // -----------------------------------------------------------------------------

    void CTestSuite::LogCondition(const char* _pCondition)
    {
        assert(_pCondition != 0);

        m_LogStream << "Condition: " << _pCondition << std::endl << std::endl;
    }
    
    // -----------------------------------------------------------------------------
    
    void CTestSuite::LogTime(const char* _pMessage)
    {
        m_Clock.OnFrame();
        
        double CurrentTimestamp = m_Clock.GetTime();
        
        double DifferenceOfTime = CurrentTimestamp - m_PreviousTimestamp;
        
        m_LogStream << "Info:      " << m_CurrentFunctionName << std::endl;
        m_LogStream << "File:      " << m_CurrentFileName << " (" << m_CurrentLineNumber << ")" << std::endl;
        m_LogStream << "Time:      " << _pMessage << "> " << DifferenceOfTime << "s (" << DifferenceOfTime * 1000.0 << "ms; " << DifferenceOfTime * 1000000.0 << "µs)" << std::endl << std::endl;
        
        m_PreviousTimestamp = CurrentTimestamp;
    }
    
    // -----------------------------------------------------------------------------
    
    void CTestSuite::ResetTime()
    {
        m_PreviousTimestamp = m_Clock.GetTime();
    }

    // -----------------------------------------------------------------------------

    std::ostringstream& CTestSuite::GetLogStream()
    {
        return m_LogStream;
    }

    // -----------------------------------------------------------------------------

    unsigned int CTestSuite::GetTestCount()
    {
        return m_NumberOfTests;
    }

    // -----------------------------------------------------------------------------

    unsigned int CTestSuite::GetFailedTestCount()
    {
        return m_NumberOfFailedTests;
    }

    // -----------------------------------------------------------------------------

    void CTestSuite::LogStatistics()
    {
        m_LogStream << "Result: " << GetFailedTestCount() << " tests failed out of " << GetTestCount() << std::endl;
    }

    // -----------------------------------------------------------------------------

    void CTestSuite::ResetFailedCheckCount()
    {
        m_NumberOfFailedChecks = 0;
    }

    // -----------------------------------------------------------------------------

    void CTestSuite::ResetCounters()
    {
        ResetFailedCheckCount();

        m_NumberOfFailedTests = 0;
    }
} // namespace 

namespace UT
{
namespace Suite
{

    void Reset(std::ostream* _pStream = 0)
    {
        CTestSuite::GetInstance().Reset(_pStream);
    }

    // -----------------------------------------------------------------------------

    void RegisterTest(void (*_TestFtr)())
    {
        CTestSuite::GetInstance().RegisterTest(_TestFtr);
    }

    // -----------------------------------------------------------------------------

    void RunTests()
    {
        CTestSuite::GetInstance().RunTests();
    }

    // -----------------------------------------------------------------------------

    void SetTestSource(const char* _pFileName, const char* _pFunctionName, int _LineNumber)
    {
        CTestSuite::GetInstance().SetTestSource(_pFileName, _pFunctionName, _LineNumber);
    }

    // -----------------------------------------------------------------------------

    void LogFailure()
    {
        CTestSuite::GetInstance().LogFailure();
    }

    // -----------------------------------------------------------------------------

    void LogException(const char* _pException)
    {
        CTestSuite::GetInstance().LogException(_pException);
    }
    
    // -----------------------------------------------------------------------------

    void LogCondition(const char* _pCondition)
    {
        CTestSuite::GetInstance().LogCondition(_pCondition);
    }
    
    // -----------------------------------------------------------------------------
    
    void LogTime(const char* _pMessage)
    {
        CTestSuite::GetInstance().LogTime(_pMessage);
    }
    
    // -----------------------------------------------------------------------------
    
    void ResetTime()
    {
        CTestSuite::GetInstance().ResetTime();
    }
    
    // -----------------------------------------------------------------------------

    std::ostringstream& GetLogStream()
    {
        return CTestSuite::GetInstance().GetLogStream();
    }

    // -----------------------------------------------------------------------------

    unsigned int GetTestCount()
    {
        return CTestSuite::GetInstance().GetTestCount();
    }

    // -----------------------------------------------------------------------------

    unsigned int GetFailedTestCount()
    {
        return CTestSuite::GetInstance().GetFailedTestCount();
    }

} // namespace Suite
} // namespace UT