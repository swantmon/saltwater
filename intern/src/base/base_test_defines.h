
#pragma once

#include "base/base_defines.h"
#include "base/base_test_suite.h"

#include <exception>

#define BASE_TEST(TestFunction)                                                                         \
    void TestFunction();                                                                                \
    void BASE_CONCAT(SetTestSource, TestFunction)();                                                    \
    struct BASE_CONCAT(SReg, TestFunction)                                                              \
    {                                                                                                   \
        BASE_CONCAT(SReg, TestFunction())                                                               \
        {                                                                                               \
            SER::Suite::RegisterTest(TestFunction);                                                     \
        }                                                                                               \
    } const BASE_CONCAT(g_Reg, TestFunction);                                                           \
    void BASE_CONCAT(SetTestSource, TestFunction)()                                                     \
    {                                                                                                   \
        SER::Suite::SetTestSource(__FILE__, __FUNCTION__, __LINE__);                                    \
    }                                                                                                   \
    void TestFunction()                                                                                 \

// -----------------------------------------------------------------------------

#define BASE_CHECK(Condition)                                                                       \
    {                                                                                               \
        SER::Suite::SetTestSource(__FILE__, __FUNCTION__, __LINE__);                                \
        try                                                                                         \
        {                                                                                           \
            if (!(Condition))                                                                       \
            {                                                                                       \
                SER::Suite::LogFailure();                                                           \
                SER::Suite::LogCondition(#Condition);                                               \
            }                                                                                       \
        }                                                                                           \
        catch (std::exception& _rException)                                                         \
        {                                                                                           \
            SER::Suite::LogFailure();                                                               \
            SER::Suite::LogException(_rException.what());                                           \
            SER::Suite::LogCondition(#Condition);                                                   \
        }                                                                                           \
        catch (...)                                                                                 \
        {                                                                                           \
            SER::Suite::LogFailure();                                                               \
            SER::Suite::LogException("unknown exception");                                          \
            SER::Suite::LogCondition(#Condition);                                                   \
        }                                                                                           \
    }                                                                                               \
    ((void) 0)                                                                                      \

#define BASE_TIME_RESET()                                                                           \
    {                                                                                               \
        SER::Suite::ResetTime();                                                                    \
    }                                                                                               \
    ((void) 0)                                                                                      \

#define BASE_TIME_LOG(Message)                                                                      \
    {                                                                                               \
        SER::Suite::SetTestSource(__FILE__, __FUNCTION__, __LINE__);                                \
        SER::Suite::LogTime(#Message);                                                              \
    }                                                                                               \
    ((void) 0)                                                                                      \

    